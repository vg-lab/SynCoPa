/*
 * @file  branching.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include <brion/brion.h>
#include <brain/brain.h>

#include "../PathFinder.h"
#include "../ConnectivityTree.h"

int main( int /*argc*/, char** /*argv*/ )
{
//  if( argc < 2 )
//    return -1;
//
//  std::string blueCofigPath = argv[ 1 ];
//
//  brion::BlueConfig* blueConfig = new brion::BlueConfig( blueCofigPath );
//  brain::Circuit* circuit = new brain::Circuit( *blueConfig );
//
//  auto gidSet = circuit->getGIDs( );
//
//  std::cout << "Loaded target with " << gidSet.size( ) << " nuerons." << std::endl;
//
//  const brain::Synapses& synapses =
//      circuit->getAfferentSynapses( gidSet, brain::SynapsePrefetch::positions );
//
//  std::vector< brion::Vector3f > positions;
//  positions.reserve( synapses.size( ));
//
//  try
//  {
//    for( auto syn : synapses )
//      positions.push_back( syn.getPostsynapticSurfacePosition( ));
//  }
//  catch( ... )
//  {
//
//    for( auto syn : synapses )
//    {
//      unsigned int segmentID = syn.getPresynapticSegmentID( );
//
//    }
//
//  }

//  syncopa::PathFinder* pathFinder = new syncopa::PathFinder( );
//
//  syncopa::ConnectivityTree* tree = new syncopa::ConnectivityTree( );


}
