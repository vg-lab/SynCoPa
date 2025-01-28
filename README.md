# SynCoPa: Visualizing Connectivity Paths and Synapses Over Detailed Morphologies
(c) 2015-2025. Visualization & Graphics Lab / URJC
[www.vg-lab.es](www.vg-lab.es)

## Introduction
SynCoPa is a tool designed for bridging gaps among neuronal graphical representations by providing techniques that allow combining detailed morphological neuron representations with the visualization of neuron interconnections at the synapse level. SynCoPa has been conceived for the interactive exploration and analysis of the connectivity elements and paths of simple to medium complexity neuronal circuits at the connectome level. This has been done by providing visual metaphors for synapses and interconnection paths, in combination with the representation of detailed neuron morphologies. SynCoPa could be helpful, for example, for establishing or confirming a hypothesis about the spatial distributions of synapses, or for answering questions about the way neurons establish connections or the relationships between connectivity and morphological features. Last, SynCoPa is easily extendable to include functional data provided, for example, by any of the morphologically-detailed simulators available nowadays, such as Neuron and Arbor, for providing a deep insight into the circuits features prior to simulating it, in particular any analysis where it is important to combine morphology, network topology, and physiology.

[Link to scientific paper](https://www.frontiersin.org/journals/neuroinformatics/articles/10.3389/fninf.2021.753997/full) 

## Dependencies
When building Syncopa from the sources it depends on the following libraries:  

* [EyeScale CMake Modules](https://github.com/Eyescale/CMake)
* [ZeroEQ](https://github.com/HBPVIS/ZeroEQ)
* [Lexis](https://github.com/HBPVIS/Lexis)
* [gmrvlex](https://github.com/vg-lab/gmrvlex)
* [ReTo](https://github.com/vg-lab/ReTo)
* [plab](https://github.com/vg-lab/particlelab)
* [Brion](https://github.com/BlueBrain/Brion))
* [nsol](https://github.com/vg-lab/nsol)
* [neurolots](https://github.com/vg-lab/neurolots)
* [scoop](https://github.com/vg-lab/scoop)

Depending on the the CMake configuration some libraries won't be needed. In order to connect applications one another, it is necessary to compile the project with ZeroEQ and its vocabulary libraries.

## Building
Syncopa has been succesfully built and used on Ubuntu 14.04/16.04/22.04, Mac OSX Yosemite and Windows 7/8 (Visual Studio 2013 Win64). The following steps should be enough to build it:

```bash
git clone https://github.com/vg-lab/SynCoPa.git syncopa
mkdir syncopa/build && cd syncopa/build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCLONE_SUBPROJECTS=ON
make
```
