//
// Created by gaeqs on 16/03/25.
//

#include "SnuddaLoader.h"

#include <unordered_map>
#include <vector>
#include <cmath>

#include <glm/glm.hpp>

constexpr int TO_MICROMETERS = 1000000;

namespace syncopa
{
    std::unordered_map<uint32_t, nsol::Neuron*> SnuddaLoader::loadNeurons(nsol::DataSet& dataset) const {
        auto idsDS = _file->openDataSet("network/neurons/neuron_id");
        auto positionDS = _file->openDataSet("network/neurons/position");
        auto rotationDS = _file->openDataSet("network/neurons/rotation");
        auto voxelSizeDS = _file->openDataSet("meta/voxel_size");

        hsize_t dims;
        idsDS.getSpace().getSimpleExtentDims(&dims);

        std::vector<uint32_t> ids(dims);
        std::vector<std::array<double, 3>> position(dims);
        std::vector<std::array<double, 9>> rotation(dims);
        double vSize;

        idsDS.read(ids.data(), H5::PredType::INTEL_I32);
        positionDS.read(position.data(), H5::PredType::IEEE_F64LE);
        rotationDS.read(rotation.data(), H5::PredType::IEEE_F64LE);
        voxelSizeDS.read(&vSize, H5::PredType::IEEE_F64LE);

        std::unordered_map<uint32_t, nsol::Neuron*> neurons;

        for (size_t i = 0; i < std::min(ids.size(), _maxNeurons); ++i) {
            glm::vec3 pos(position[i][0], position[i][1], position[i][2]);

            Eigen::Matrix4f model;
            for (size_t c = 0; c < 3; ++c) {
                for (size_t r = 0; r < 3; ++r) {
                    model(r, c) = static_cast<float>(rotation[i][c + r * 3]);
                }
            }

            // neuron coordinates are in meters, convet to micrometers by multiplying by 10^6
            // because swc units are in micrometers.
            // Comment: https://github.com/Hjorthmedh/Snudda/blob/master/snudda/utils/export_sonata.py :142
            model(0, 3) = pos[0] * TO_MICROMETERS;
            model(1, 3) = pos[1] * TO_MICROMETERS;
            model(2, 3) = pos[2] * TO_MICROMETERS;
            model(3, 3) = 1.0f;

            nsol::Neuron* neuron = new nsol::Neuron(nullptr, 0, ids[i], model,
                                                    nullptr, nsol::Neuron::PYRAMIDAL);
            dataset.addNeuron(neuron);
            neurons.insert({ids[i], neuron});
        }

        idsDS.close();
        positionDS.close();
        rotationDS.close();
        voxelSizeDS.close();

        return neurons;
    }

    std::string SnuddaLoader::loadMorphologies(const std::unordered_map<uint32_t, nsol::Neuron*>& neurons) const {
        static const std::string SNUDDA_PREFIX = "$SNUDDA_DATA";

        auto morphologiesDS = _file->openDataSet("network/neurons/morphology");
        auto stringType = morphologiesDS.getStrType();

        hsize_t dims;
        morphologiesDS.getSpace().getSimpleExtentDims(&dims);

        std::vector<char*> cStringArray(dims, nullptr);
        H5::DataSpace dataspace = morphologiesDS.getSpace();
        morphologiesDS.read(cStringArray.data(), stringType, dataspace);

        std::unordered_map<std::string, nsol::NeuronMorphologyPtr> loaded;

        nsol::SwcReaderTemplated<nsol::Node, nsol::NeuronMorphologySection,
            nsol::Dendrite, nsol::Axon, nsol::Soma,
            nsol::NeuronMorphology, nsol::Neuron> reader;

        for (auto& pair: neurons) {
            auto id = pair.first;
            auto neuron = pair.second;
            auto name = std::string(cStringArray[id]);
            if (loaded.count(name) > 0) {
                auto morphology = loaded[name];
                neuron->morphology(morphology);
                morphology->parentNeurons().push_back(neuron);
                continue;
            }
            std::string modified = name;
            modified.replace(0, SNUDDA_PREFIX.length(), _dataPath.string());
            std::cout << modified << std::endl;
            auto morphology = reader.readMorphology(modified, false);
            loaded[name] = morphology;
            neuron->morphology(morphology);
            morphology->parentNeurons().push_back(neuron);
        }

        morphologiesDS.close();

        return {};
    }

    std::string SnuddaLoader::loadSynapses(nsol::DataSet& dataset) const 
    {
        // From https://github.com/Hjorthmedh/Snudda/blob/master/snudda/utils/export_sonata.py : 743
        //   Convert synapse coordinates(in SWC reference frame) to SectionID
        //   and section_X(fractional distance along dendritic section)
        auto voxelSizeDs = _file->openDataSet("meta/voxel_size");
        auto synapsesDs = _file->openDataSet("network/synapses");
        auto origoDs = _file->openDataSet("meta/simulation_origo");

        hsize_t dims;
        synapsesDs.getSpace().getSimpleExtentDims(&dims);

        double voxelSize;
        std::vector<std::array<int32_t, 13>> synapses(dims);

        std::vector<double> simulation_origo(3, 0);

        voxelSizeDs.read(&voxelSize, H5::PredType::INTEL_I32);
        synapsesDs.read(synapses.data(), H5::PredType::INTEL_I32);
        origoDs.read(simulation_origo.data(), H5::PredType::IEEE_F64LE);

        const Eigen::Vector3f origoVector(simulation_origo[0], simulation_origo[1], simulation_origo[2]);

        auto& neurons = dataset.neurons();

        uint32_t idGenerator = 0;
        std::cout << "Information of " << synapses.size() << " synapses" << std::endl;
        for (auto& synapse: synapses) 
        {
            // https: // github.com/Hjorthmedh/Snudda/blob/master/snudda/utils/load.py#L621 : 726
            // synapse_coords = synapses[:, 2:5] * self.data["voxel_size"] + self.data["simulation_origo"]
            Eigen::Vector3f position(synapse[2], synapse[3], synapse[4]);
            position *= static_cast<float>(voxelSize);
            position += origoVector;

            auto neuron = neurons.find(synapse[0]);
            if (neuron == neurons.end()) return "Couldn't find neuron " + std::to_string(synapse[1]);
            auto morph = neuron->second->morphology();
            if (morph == nullptr) return "Morphology is null";
            auto id = synapse[9];

            auto nsolSynapse = new nsol::MorphologySynapse();

            nsol::Vec3f prePoint;
            for (auto neurite : morph->neurites())
            {
                for (auto section : neurite->sections())
                {
                    auto morphSection = dynamic_cast<nsol::NeuronMorphologySection *>(section);
                    for (auto node : section->nodes())
                    {
                        if (node->id() == id)
                        {
                            prePoint = node->point();
                            nsolSynapse->preSynapticSection(morphSection);
                        }
                    }
                }
            }

            // No post coords or info, try to compute it.
            neuron = neurons.find(synapse[1]);
            if (neuron == neurons.end()) return "Couldn't find neuron " + std::to_string(synapse[1]);
            float minDist = std::numeric_limits<float>::max();
            nsol::Node *closestNode = nullptr;
            nsol::NeuronMorphologySection *closestSection = nullptr;
            morph = neuron->second->morphology();
            if (morph == nullptr) return "Morphology is null";
            for (auto neurite : morph->neurites())
            {
                for (auto section : neurite->sections())
                {
                    auto morphSection = dynamic_cast<nsol::NeuronMorphologySection *>(section);
                    for (auto node : section->nodes())
                    {
                        auto dist = std::abs((node->point() - prePoint).norm());
                        if (dist < minDist)
                        {
                            minDist = dist;
                            closestNode = node;
                            closestSection = morphSection;
                        }
                    }
                }
            }

            if (closestNode != nullptr)
                nsolSynapse->postSynapticSection(closestSection);

            // THIS IS SWAPPED. Snudda doesn't include presynaptic information.
            nsolSynapse->preSynapticNeuron(synapse[0]);
            nsolSynapse->postSynapticNeuron(synapse[1]);
            nsolSynapse->preSynapticSurfacePosition(position);
            nsolSynapse->postSynapticSurfacePosition(closestNode->point());
            nsolSynapse->gid(idGenerator++);
            dataset.circuit().addSynapse(nsolSynapse);
            std::cout << "\r" << (idGenerator * 100)/dims << std::flush;
        }

        voxelSizeDs.close();
        synapsesDs.close();
        origoDs.close();
        return {};
    }

    void SnuddaLoader::setMaxNeuronsToLoad(size_t maxNeurons) {
        _maxNeurons = maxNeurons;
    }

    SnuddaLoader::SnuddaLoader(const boost::filesystem::path& path, const boost::filesystem::path& repoPath)
        : _file(std::make_unique<H5::H5File>(path.string(), 0)),
          _dataPath(repoPath / "data"),
          _maxNeurons(10000000) {}

    SnuddaLoader::~SnuddaLoader() {
        _file->close();
    }

    void SnuddaLoader::load(nsol::DataSet& dataset) const {
        auto neurons = loadNeurons(dataset);

        auto error = loadMorphologies(neurons);
        if (!error.empty()) {
            std::cerr << error << std::endl;
            return;
        }

        error = loadSynapses(dataset);
        if (!error.empty()) {
            std::cerr << error << std::endl;
        }
    }
}
