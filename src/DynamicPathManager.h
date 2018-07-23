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

    void configure( unsigned int presynapticGid,
                    const std::vector< nsol::MorphologySynapsePtr >& synapses,
                    const tSectionsInfoMap& infoSections );

    void addPostsynapticPath( nsol::MorphologySynapsePtr synapse,
                              const tPosVec& nodes );

    void processPendingSynapses( void );
    void processPendingSections( void );
    void processFinishedPaths( void );

    void clear( void );

    void synapse( nsol::MorphologySynapsePtr syn );
    void finished( unsigned int sourceID );

    void finishedSection( unsigned int sectionID );

  protected:

    void _createSourceOnDeepestPath( cnode_ptr origin, MobilePolylineSource* source  );

    unsigned int _presynapticGID;

    PathFinder* _pathFinder;

    PSManager* _psManager;

    ConnectivityTree _treePath;

    tSectionsInfoMap _infoSections;

    std::unordered_map< nsol::MorphologySynapsePtr,
                        utils::PolylineInterpolation > _pathsPost;

    std::unordered_map< unsigned int, utils::EventPolylineInterpolation > _computedPaths;

    std::unordered_map< unsigned int, MobilePolylineSource* > _sources;

    std::unordered_set< syncopa::MobilePolylineSource* > _rootSources;

    std::list< unsigned int > _pendingSections;
    std::list< unsigned int > _pendingSources;

    unsigned int _maxDepth;
  };


}

#endif /* SRC_DYNAMICPATHMANAGER_H_ */
