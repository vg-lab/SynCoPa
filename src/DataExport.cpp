//
// Created by gaeqs on 2/06/22.
//

#include "DataExport.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>
#include <unordered_set>
#include <unordered_map>
#include <nsol/nsol.h>

const std::vector<std::string> stat_types = {
    "total", "mean",
    "variance", "std_dev",
    "min", "max"
};
const std::vector<std::string> stat_names = {
    "dentritic_volume",
    "axon_volume",
    "neuritic_volume",
    "soma_volume",
    "volume",
    "dentritic_surface",
    "axon_surface",
    "neuritic_surface",
    "soma_surface",
    "surface",
    "dentritic_length",
    "axon_length",
    "neuritic_length",
    "dentritic_bifurcations",
    "axon_bifurcations",
    "neuritic_bifurcations"
};

std::string morphologicalTypeToString(
    const nsol::Neuron::TMorphologicalType& type) {
    switch (type) {
        case nsol::Neuron::PYRAMIDAL:
            return "PYRAMIDAL";
        case nsol::Neuron::INTERNEURON:
            return "INTERNEURON";
        default:
            break;
    }

    return "UNDEFINED";
}

std::string functionalTypeToString(const nsol::Neuron::TFunctionalType& type) {
    switch (type) {
        case nsol::Neuron::INHIBITORY:
            return "INHIBITORY";
        case nsol::Neuron::EXCITATORY:
            return "EXCITATORY";
        default:
            break;
    }

    return "UNDEFINED_FUNCTIONAL_TYPE";
}

void syncopa::toXML(const nsol::DataSet* dataset, std::ostream& out,
                    const XMLExportParameters& parameters) {
    const auto& columns = dataset->columns();

    // Write headers
    out << R"(<?xml version="1.0" encoding="UTF-8"?>)"
            << std::endl << std::endl;
    out << "<scene version=\"0.1\">" << std::endl;
    out << "  <morphology>" << std::endl;


    out << "    <columns>" << std::endl;
    for (const auto& col: columns) {
        out << "      <column id=\"" << col->id() << "\">" << std::endl;

        for (const auto& minicol: col->miniColumns()) {
            out << "        <minicolumn id=\"" << minicol->id()
                    << "\">" << std::endl;

            for (const auto& neuron: minicol->neurons()) {
                const auto morphological = morphologicalTypeToString(
                    neuron->morphologicalType());
                const auto functional = functionalTypeToString(
                    neuron->functionalType());

                out << "          <neuron gid=\"" << neuron->gid() << "\" "
                        << "layer=\"" << neuron->layer() << "\" "
                        << "morphologicalType=\""
                        << morphological
                        << "\" "
                        << "functionalType=\""
                        << functional
                        << "\">" << std::endl;

                const auto& xform = neuron->transform();
                out << "            <transform>" << std::endl;
                out << "              "
                        << xform(0, 0) << ", " << xform(0, 1) << ", "
                        << xform(0, 2) << ", " << xform(0, 3) << ", " <<
                        std::endl;
                out << "              "
                        << xform(1, 0) << ", " << xform(1, 1) << ", "
                        << xform(1, 2) << ", " << xform(1, 3) << ", " <<
                        std::endl;
                out << "              "
                        << xform(2, 0) << ", " << xform(2, 1) << ", "
                        << xform(2, 2) << ", " << xform(2, 3) << ", " <<
                        std::endl;
                out << "              "
                        << xform(3, 0) << ", " << xform(3, 1) << ", "
                        << xform(3, 2) << ", " << xform(3, 3) << std::endl;
                out << "            </transform>" << std::endl;

                auto morphology = neuron->morphology();
                if (morphology != nullptr) {
                    out << "            <morphology>" << std::endl;

                    auto morphStats = morphology->stats();

                    if (morphStats != nullptr && parameters.
                        exportAggregatedStatistics) {
                        out << "              <stats>" << std::endl;

                        for (auto type: parameters.aggregatedStatistics) {
                            auto& typeN = stat_types.at(type);
                            out << "                <" << typeN << ">" <<
                                    std::endl;
                            for (int stat = 0;
                                 stat <
                                 nsol::NeuronMorphologyStats::NEURON_MORPHOLOGY_NUM_STATS
                                 ;
                                 stat++) {
                                const float value = morphStats->getStat(
                                    static_cast<nsol::NeuronMorphologyStats
                                        ::TNeuronMorphologyStat>(stat), type);

                                auto& statN = stat_names.at(stat);

                                out << "                  <" << statN << ">";
                                out << value;
                                out << "</" << statN << ">" << std::endl;
                            }
                            out << "                </" << typeN << ">" <<
                                    std::endl;
                        }

                        out << "              </stats>" << std::endl;
                    }

                    if (parameters.exportMorphologyStatistics) {
                        const auto soma = morphology->soma();
                        if (soma != nullptr) {
                            const auto stats = soma->stats();
                            if (stats != nullptr) {
                                out << "              <soma>" << std::endl;
                                out << "                <surface>";
                                out << stats->getStat(nsol::SomaStats::SURFACE);
                                out << "</surface>" << std::endl;
                                out << "                <volume>";
                                out << stats->getStat(nsol::SomaStats::VOLUME);
                                out << "</volume>" << std::endl;
                                out << "              </soma>" << std::endl;
                            }
                        }

                        const auto axon = morphology->axon();
                        if (axon != nullptr) {
                            const auto stats = axon->stats();
                            if (stats != nullptr) {
                                out << "              <axon>" << std::endl;
                                out << "                <surface>";
                                out << stats->getStat(
                                    nsol::NeuriteStats::SURFACE);
                                out << "</surface>" << std::endl;
                                out << "                <volume>";
                                out << stats->getStat(
                                    nsol::NeuriteStats::VOLUME);
                                out << "</volume>" << std::endl;
                                out << "                <length>";
                                out << stats->getStat(
                                    nsol::NeuriteStats::LENGTH);
                                out << "</length>" << std::endl;
                                out << "                <bifurcations>";
                                out << stats->getStat(
                                    nsol::NeuriteStats::BIFURCATIONS);
                                out << "</bifurcations>" << std::endl;
                                out << "              </axon>" << std::endl;
                            }
                        }

                        out << "              <dendrites>" << std::endl;
                        for (const auto& dendrite: *morphology->dendrites()) {
                            const std::string dentriteType =
                                    dendrite->dendriteType() ==
                                    nsol::Dendrite::BASAL
                                        ? "BASAL"
                                        : "APICAL";
                            const std::string neuriteType =
                                    dendrite->neuriteType() ==
                                    nsol::Dendrite::DENDRITE
                                        ? "DENDRITE"
                                        : "AXON";

                            out << "                <dendrite dentrite_type=\""
                                    << dentriteType << "\" neurite_type=\""
                                    << neuriteType << "\">" << std::endl;

                            const auto stats = dendrite->stats();
                            if (stats != nullptr) {
                                out << "                  <surface>";
                                out << stats->getStat(
                                    nsol::NeuriteStats::SURFACE);
                                out << "</surface>" << std::endl;
                                out << "                  <volume>";
                                out << stats->getStat(
                                    nsol::NeuriteStats::VOLUME);
                                out << "</volume>" << std::endl;
                                out << "                  <length>";
                                out << stats->getStat(
                                    nsol::NeuriteStats::LENGTH);
                                out << "</length>" << std::endl;
                                out << "                  <bifurcations>";
                                out << stats->getStat(
                                    nsol::NeuriteStats::BIFURCATIONS);
                                out << "</bifurcations>" << std::endl;
                            }

                            out << "                </dendrite>" << std::endl;
                        }
                        out << "              </dendrites>" << std::endl;
                    }
                    out << "            </morphology>" << std::endl;
                }
                out << "          </neuron>" << std::endl;
            }

            out << "        </minicolumn>" << std::endl;
        }

        out << "      </column>" << std::endl;
    }

    out << "    </columns>" << std::endl;
    out << "  </morphology>" << std::endl;
    out << "</scene>" << std::endl;
}

void syncopa::toAggregateCSV(const nsol::DataSet* dataset, std::ostream& out,
                             char columnSeparator, char rowSeparator) {
    const auto& synapses = dataset->circuit().synapses();

    out << "PreNeuronId" << columnSeparator
            << "PostNeuronId" << columnSeparator
            << "Weight";

    std::map<std::pair<unsigned int, unsigned int>, unsigned int>
            synapsesCount;

    for (const auto& syn: synapses) {
        synapsesCount[std::make_pair(syn->preSynapticNeuron(),
                                     syn->postSynapticNeuron())]++;
    }

    for (const auto& item: synapsesCount) {
        out << rowSeparator;
        out << item.first.first;
        out << columnSeparator;
        out << item.first.second;
        out << columnSeparator;
        out << item.second;
    }
}

void syncopa::toCompactCSV(const nsol::DataSet* dataset, std::ostream& out,
                           const char columnSeparator,
                           const char rowSeparator) {
    const auto& synapses = dataset->circuit().synapses();

    const brain::Circuit brainCircuit(*dataset->blueConfig());
    const brion::GIDSet gidSetBrain = brainCircuit.getGIDs(
        dataset->blueConfigTarget());

    const brain::Synapses& brainSynapses =
            brainCircuit.getAfferentSynapses(gidSetBrain,
                                             brain::SynapsePrefetch::attributes);

    out << "SynId" << columnSeparator
            << "PreNeuronId" << columnSeparator
            << "PostNeuronId" << columnSeparator
            << "Delay" << columnSeparator
            << "Conductance" << columnSeparator
            << "Utilization" << columnSeparator
            << "Depression" << columnSeparator
            << "Facilitation" << columnSeparator
            << "Efficacy";

    for (const auto& syn: synapses) {
        auto morphSyn = dynamic_cast<nsol::MorphologySynapse*>(syn);
        if (morphSyn) {
            auto brainSyn = brainSynapses[morphSyn->gid() - 1];
            out << rowSeparator;
            out << morphSyn->gid();
            out << columnSeparator;
            out << morphSyn->preSynapticNeuron();
            out << columnSeparator;
            out << morphSyn->postSynapticNeuron();
            out << columnSeparator;
            out << brainSyn.getDelay();
            out << columnSeparator;
            out << brainSyn.getConductance();
            out << columnSeparator;
            out << brainSyn.getUtilization();
            out << columnSeparator;
            out << brainSyn.getDepression();
            out << columnSeparator;
            out << brainSyn.getFacilitation();
            out << columnSeparator;
            out << brainSyn.getEfficacy();
        }
    }
}

void syncopa::toMatrixCSV(const nsol::DataSet* dataset, std::ostream& out,
                          const char columnSeparator, const char rowSeparator) {
    const auto& neurons = dataset->neurons();

    const auto minmax = std::minmax_element(
        neurons.begin(), neurons.end(),
        [ ](std::pair<const unsigned int, nsol::NeuronPtr> a,
            std::pair<const unsigned int, nsol::NeuronPtr> b) {
            return a.first < b.first;
        });

    const auto min = minmax.first;
    const auto max = minmax.second;

    out << "Pre gid \\ Post gid";

    for (unsigned int i = min->first; i <= max->first; i++) {
        out << columnSeparator << i;
    }

    std::map<unsigned int, unsigned int> synapsesMap;
    for (const auto& pre: dataset->neurons()) {
        const auto synapses = dataset->circuit().synapses(
            pre.first, nsol::Circuit::PRESYNAPTICCONNECTIONS
        );

        for (const auto& syn: synapses) {
            const unsigned int post = syn->postSynapticNeuron();
            synapsesMap[post] = synapsesMap[post] + 1;
        }

        out << rowSeparator << pre.first;

        for (unsigned int i = min->first; i <= max->first; i++) {
            out << columnSeparator << synapsesMap[i];
        }
        synapsesMap.clear();
    }
}

QJsonObject syncopa::toHanoiJSON(const nsol::DataSet* dataset) {
    const auto& columns = dataset->columns();
    QJsonObject root;
    QJsonObject hierarchy;

    // Hierarchy
    hierarchy.insert("name", "root");
    hierarchy.insert("id", -1);
    QJsonArray hierarchyChildren;

    uint32_t minColumnId = 0;
    for (auto& n: dataset->neurons()) {
        minColumnId = std::max(minColumnId, n.first + 1);
    }

    uint32_t extraIdsCount = 0;
    for (auto& c: dataset->columns()) {
        extraIdsCount = std::max(extraIdsCount,
                                 static_cast<uint32_t>(c->id()) + 1);
    }
    extraIdsCount += minColumnId;


    for (auto& column: columns) {
        QJsonObject jsonColumn;
        QJsonArray columnChildren;

        for (const auto& miniColumn: column->miniColumns()) {
            QJsonObject jsonMiniColumn;
            QJsonArray miniColumnChildren;

            std::unordered_map<uint32_t, std::vector<uint32_t>> gidsPerLayer;


            for (const auto& neuron: miniColumn->neurons()) {
                gidsPerLayer[static_cast<uint32_t>(neuron->layer())].push_back(neuron->gid());
            }

            for (auto& layer: gidsPerLayer) {
                QJsonObject jsonLayer;
                QJsonArray layerChildren;

                for (const auto& neuron: layer.second) {
                    QJsonObject jsonNeuron;
                    jsonNeuron.insert("id", static_cast<int>(neuron));
                    jsonNeuron.insert("name", QVariant(neuron).toString());
                    layerChildren.append(jsonNeuron);
                }

                int32_t lid = static_cast<int32_t>(extraIdsCount++);
                jsonLayer.insert("id", lid);
                jsonLayer.insert("name", QVariant(layer.first).toString());
                jsonLayer.insert("children", layerChildren);

                miniColumnChildren.append(jsonLayer);
            }

            int32_t mid = static_cast<int32_t>(extraIdsCount++);
            jsonMiniColumn.insert("id", mid);
            jsonMiniColumn.
                    insert("name", QVariant(static_cast<unsigned int>(miniColumn->id())).toString());
            jsonMiniColumn.insert("children", miniColumnChildren);
            columnChildren.append(jsonMiniColumn);
        }

        int32_t cid = column->id() + minColumnId;
        jsonColumn.insert("id", cid);
        jsonColumn.insert("name", QVariant(static_cast<unsigned int>(column->id())).toString());
        jsonColumn.insert("children", columnChildren);
        hierarchyChildren.append(jsonColumn);
    }
    hierarchy.insert("children", hierarchyChildren);

    root.insert("hierarchy", hierarchy);

    // Connectivity

    const auto& synapses = dataset->circuit().synapses();

    std::unordered_map<uint32_t, std::unordered_set<uint32_t>> parsedSynapses;

    for (auto& synapse: synapses) {
        parsedSynapses[synapse->preSynapticNeuron()]
                .insert(synapse->postSynapticNeuron());
    }

    QJsonArray connections;
    for (auto& con: parsedSynapses) {
        QJsonObject connection;
        QJsonArray imports;
        connection.insert("id", static_cast<int>(con.first));

        for (auto& post: con.second) {
            QJsonObject import;
            import.insert("size", 1);
            import.insert("id", static_cast<int>(post));
            imports.append(import);
        }
        connection.insert("imports", imports);
        connections.append(connection);
    }
    root.insert("connections", connections);

    // Domain

    QString string = R"(
    {
        "name": "SynCoPa",
        "network": {
            "mode": "directed"
        },
        "hierarchy": {
            "insert": false,
        "edit": false
        }
    }
    )";

    root.insert("domain", QJsonDocument::fromJson(string.toUtf8()).object());

    return root;
}

std::vector<std::unordered_set<uint32_t>> egonetwork(const nsol::DataSet* dataset,
                                                     std::unordered_set<uint32_t> focalNodes,
                                                     uint32_t deep) {
    std::unordered_map<uint32_t, std::unordered_set<uint32_t>> synapses;
    for (auto& synapse: dataset->circuit().synapses()) {
        synapses[synapse->preSynapticNeuron()].insert(
            synapse->postSynapticNeuron());
    }

    std::unordered_set<uint32_t> present = focalNodes;
    std::vector<std::unordered_set<uint32_t>> levels;
    levels.reserve(deep);
    levels.push_back(std::move(focalNodes)); // Focal nodes

    for (uint32_t i = 0; i < deep; ++i) {
        auto& previous = levels.back();
        std::unordered_set<uint32_t> current;
        for (auto& preNeuron: previous) {
            auto connections = synapses.find(preNeuron);
            if (connections == synapses.end()) continue;
            for (auto& postNeuron: connections->second) {
                if (present.find(postNeuron) == present.end()) continue;
                present.insert(postNeuron);
                current.insert(postNeuron);
            }
        }

        levels.push_back(std::move(current));
    }

    return levels;
}
