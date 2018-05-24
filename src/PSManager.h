/*
 * @file  PSManager.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef SRC_PSMANAGER_H_
#define SRC_PSMANAGER_H_

#include "types.h"

#include <prefr/prefr.h>
#include <reto/reto.h>
#include <nlgeometry/nlgeometry.h>

#include "prefr/SourceMultiPosition.h"
#include "prefr/UpdaterStaticPosition.h"

namespace synvis
{

  class PSManager
  {
public:

    PSManager( void );
    ~PSManager( void );

    void init( unsigned int maxParticles = 500000 );

    prefr::ParticleSystem* particleSystem( void );

    void clear( void );

    void clearSynapses( TNeuronConnection type = ALL_CONNECTIONS );

    void clearPaths( TNeuronConnection type = ALL_CONNECTIONS );

    void setupSynapses( const std::vector< vec3 > positions,
                        TNeuronConnection type = PRESYNAPTIC );

    void setupPath( const std::vector< vec3 > nodePositions,
                    TNeuronConnection type = PRESYNAPTIC );

    vec4 colorSynapses( TNeuronConnection type = PRESYNAPTIC ) const;
    void colorSynapses( const vec4& color, TNeuronConnection type = PRESYNAPTIC );

    float sizeSynapses( TNeuronConnection type = PRESYNAPTIC ) const;
    void sizeSynapses( float size, TNeuronConnection type = PRESYNAPTIC );

    vec4 colorPaths( TNeuronConnection type = PRESYNAPTIC ) const;
    void colorPaths( const vec4& color, TNeuronConnection type = PRESYNAPTIC );

    float sizePaths( TNeuronConnection type = PRESYNAPTIC ) const;
    void sizePaths( float size, TNeuronConnection type = PRESYNAPTIC );


    nlgeometry::AxisAlignedBoundingBox boundingBox( void ) const;

protected:

    prefr::ParticleSystem* _particleSystem;

    unsigned int _maxParticles;

//    bool _autoResizeParticleSystem;

    // Models for pre and postsynaptic positions of synapses
    prefr::Model* _modelSynPre;
    prefr::Model* _modelSynPost;

    // Models for pre and postsynaptic paths between neurons
    prefr::Model* _modelPathPre;
    prefr::Model* _modelPathPost;

    SourceMultiPosition* _sourceSynPre;
    SourceMultiPosition* _sourceSynPost;

    SourceMultiPosition* _sourcePathPre;
    SourceMultiPosition* _sourcePathPost;

    // Clusters for pre and postsynaptic positions of synapses
    prefr::Cluster* _clusterSynPre;
    prefr::Cluster* _clusterSynPost;

    // Clusters for pre and postsynaptic paths between neurons
    prefr::Cluster* _clusterPathPre;
    prefr::Cluster* _clusterPathPost;

    UpdaterStaticPosition* _updaterSynapses;

    nlgeometry::AxisAlignedBoundingBox _boundingBox;
  };


}



#endif /* SRC_PSMANAGER_H_ */
