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
#include "prefr/MobilePolylineSource.h"

namespace syncopa
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

    void setupDynamicPath( const tPosVec& positions );

    vec4 colorSynapses( TNeuronConnection type = PRESYNAPTIC ) const;
    void colorSynapses( const vec4& color, TNeuronConnection type = PRESYNAPTIC );

    float sizeSynapses( TNeuronConnection type = PRESYNAPTIC ) const;
    void sizeSynapses( float size, TNeuronConnection type = PRESYNAPTIC );

    vec4 colorPaths( TNeuronConnection type = PRESYNAPTIC ) const;
    void colorPaths( const vec4& color, TNeuronConnection type = PRESYNAPTIC );

    float sizePaths( TNeuronConnection type = PRESYNAPTIC ) const;
    void sizePaths( float size, TNeuronConnection type = PRESYNAPTIC );

    void showSynapses( bool state, TNeuronConnection type = PRESYNAPTIC );
    void showPaths( bool state, TNeuronConnection type = PRESYNAPTIC );

    void run( void );
    void stop( void );

    nlgeometry::AxisAlignedBoundingBox boundingBox( void ) const;

    MobilePolylineSource* getSpareMobileSouce( void );
    void releaseMobileSource( MobilePolylineSource* source_ );


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

    prefr::Model* _modelDyn;
    float _dynamicVelocityModule;

    SourceMultiPosition* _sourceSynPre;
    SourceMultiPosition* _sourceSynPost;

    SourceMultiPosition* _sourcePathPre;
    SourceMultiPosition* _sourcePathPost;

    prefr::SphereSampler* _sampler;

    unsigned int _totalDynamicSources;
    unsigned int _particlesPerDynamicSource;
    float _mobileSourcesEmissionRate;
    std::unordered_set< MobilePolylineSource* > _availableDynamicSources;
    std::unordered_set< MobilePolylineSource* > _dynamicSources;

    prefr::Cluster* _clusterDyn;

    // Clusters for pre and postsynaptic positions of synapses
    prefr::Cluster* _clusterSynPre;
    prefr::Cluster* _clusterSynPost;

    // Clusters for pre and postsynaptic paths between neurons
    prefr::Cluster* _clusterPathPre;
    prefr::Cluster* _clusterPathPost;

    UpdaterStaticPosition* _updaterSynapses;
    prefr::Updater* _normalUpdater;

    nlgeometry::AxisAlignedBoundingBox _boundingBox;
  };


}



#endif /* SRC_PSMANAGER_H_ */
