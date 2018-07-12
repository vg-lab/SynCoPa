/*
 * @file  projectionTest.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include <brion/brion.h>
#include <brain/brain.h>

int main( int , char** argv )
{


  std::string file = argv[ 1 ];
  std::string target = argv[ 2 ];

  brion::BlueConfig* blueConfig = new brion::BlueConfig( file );

  brion::GIDSet gidSet = blueConfig->parseTarget( target );

  brain::Circuit brainCircuit( *blueConfig );
  brion::GIDSet gidSetBrain = brainCircuit.getGIDs( target );

  const brain::Synapses& brainSynapses = brainCircuit.getEfferentSynapses(
      gidSetBrain, brain::SynapsePrefetch::all );

  std::cout << "Loaded " << brainSynapses.size( ) << " synapses " << std::endl;

  auto morphologies =
      brainCircuit.loadMorphologies( gidSet, brain::Circuit::Coordinates::local );

//  for( auto morph : morphologies )
//  {
//    morph->getSections( brion::SectionType::SECTION_DENDRITE );
//  }

}
