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

int main( int argc, char** argv )
{
  if( argc < 3 )
    return -1;

  std::string blueConfigPath = argv[ 1 ];
  std::string target = argv[ 2 ];

  auto dataset = new nsol::DataSet( );

  std::cout << "Loading data hierarchy..." << std::endl;
   dataset->loadBlueConfigHierarchy( blueConfigPath, target );

  std::cout << "Loading morphologies..." << std::endl;
  dataset->loadAllMorphologies( );

  std::cout << "Loading connectivity..." << std::endl;
  dataset->loadBlueConfigConnectivityWithMorphologies( );

//  syncopa::PathFinder* pathFinder = new syncopa::PathFinder( dataset );



}
