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

#include "PathFinder.h"
#include "prefr/SourceMultiPosition.h"
#include "prefr/UpdaterStaticPosition.h"
#include "prefr/MobilePolylineSource.h"
#include "prefr/UpdaterMappedValue.h"

namespace syncopa
{

  class PSManager
  {
public:

    PSManager( void );
    ~PSManager( void );

    void init( PathFinder* pathFinder,
               unsigned int maxParticles = 500000 );

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
    void sizeSynapses( float size, TNeuronConnection type );

    vec4 colorPaths( TNeuronConnection type = PRESYNAPTIC ) const;
    void colorPaths( const vec4& color, TNeuronConnection type );

    float sizePaths( TNeuronConnection type = PRESYNAPTIC ) const;
    void sizePaths( float size, TNeuronConnection type );

    void colorSynapseMap( const tColorVec& colors );

    float sizeSynapseMap( TNeuronConnection type = PRESYNAPTIC ) const;
    void sizeSynapsesMap( float size, TNeuronConnection type );

    float sizeDynamic( void ) const;
    void sizeDynamic( float newSize );

    void showSynapses( bool state, TNeuronConnection type = PRESYNAPTIC );
    void showPaths( bool state, TNeuronConnection type = PRESYNAPTIC );

    void run( void );
    void stop( void );

    void dynamicVelocity( float velocityModule );
    float dynamicVelocity( void ) const;

    nlgeometry::AxisAlignedBoundingBox boundingBox( void ) const;

    MobilePolylineSource* getSpareMobileSouce( TNeuronConnection type = PRESYNAPTIC );
    void releaseMobileSource( MobilePolylineSource* source_ );

    void configureSynapses( const tsynapseVec& synapses,
                            TNeuronConnection type = ALL_CONNECTIONS );

    void configureMappedSynapses( const tsynapseVec& synapses,
                                  const tFloatVec& lifeValues,
                                  TNeuronConnection type = ALL_CONNECTIONS );

protected:

    void _mapSynapses( const tPosVec& positions,
                      const tFloatVec& lifeValues,
                      TNeuronConnection type );


    void _updateBoundingBox( const std::vector< vec3 > positions,
                             bool clear = true );

    prefr::ParticleSystem* _particleSystem;

    PathFinder* _pathFinder;

    unsigned int _maxParticles;

//    bool _autoResizeParticleSystem;

    // Models for pre and postsynaptic positions of synapses
    prefr::Model* _modelSynPre;
    prefr::Model* _modelSynPost;

    vec4 _colorSynPre;
    vec4 _colorSynPost;

    prefr::Model* _modelSynMap;
//    prefr::Model* _modelSynMapPost;

    vec4 _colorSynMapStart;
    vec4 _colorSynMapEnd;

    // Models for pre and postsynaptic paths between neurons
    prefr::Model* _modelPathPre;
    prefr::Model* _modelPathPost;

    vec4 _colorPathPre;
    vec4 _colorPathPost;

    prefr::Model* _modelDynPre;
    prefr::Model* _modelDynPost;

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
    UpdaterMappedValue* _updaterMapperSynapses;
    prefr::Updater* _normalUpdater;

    nlgeometry::AxisAlignedBoundingBox _boundingBox;

    std::unordered_map< unsigned int, unsigned int > _gidToParticleId;

  };


}



#endif /* SRC_PSMANAGER_H_ */
