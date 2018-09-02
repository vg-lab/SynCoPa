/*
 * @file  DynamicPathManager.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef SRC_DYNAMICPATHMANAGER_H_
#define SRC_DYNAMICPATHMANAGER_H_

#include "types.h"

#include <nsol/nsol.h>
#include <prefr/prefr.h>

#include "PathFinder.h"
#include "PSManager.h"
#include "ConnectivityTree.h"
#include "PolylineInterpolation.hpp"
#include "prefr/MobilePolylineSource.h"

namespace syncopa
{
  class DynamicPathManager
  {
  public:

    DynamicPathManager( void );

    void init( PathFinder* pathFinder_, PSManager* psManager_ );

    void createRootSources( void );

    void processPendingSynapses( void );
    void processPendingSections( void );
    void processFinishedPaths( void );

    void clear( void );

    void synapse( unsigned long int synapsePtr );
    void finished( unsigned int sourceID );

    void finishedSection( unsigned int sectionID );

    void restart( void );
    void pause( void );
    void play( void );

  protected:

    void _createSourceOnDeepestPath( nsolMSection_ptr origin, MobilePolylineSource* source  );

    unsigned int _presynapticGID;

    PathFinder* _pathFinder;

    PSManager* _psManager;

    std::unordered_map< unsigned int, MobilePolylineSource* > _sources;

    std::unordered_set< syncopa::MobilePolylineSource* > _rootSources;

    std::list< unsigned int > _pendingSections;
    std::list< unsigned int > _pendingSources;
    std::list< nsolMSynapse_ptr > _pendingSynapses;
  };


}

#endif /* SRC_DYNAMICPATHMANAGER_H_ */
