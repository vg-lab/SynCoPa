//
// Created by gaeqs on 9/06/22.
//

#include "ParticleManager.h"

#include "particlelab/ParticleLabShaders.h"

#include <plab/gl/SimpleRenderer.h>

#include <QDebug>

namespace syncopa
{

  ParticleManager::ParticleManager( )
    : _particlesBoundingBox( )
    , _staticProgram( )
    , _staticGradientProgram( )
    , _staticRenderer( nullptr )
    , _staticGradientRenderer( nullptr )
    , _synapseCluster( nullptr )
    , _synapseModel( nullptr )
    , _synapseGradientModel( nullptr )
    , _synapseBB( )
    , _pathCluster( nullptr )
    , _pathModel( nullptr )
    , _pathBB( )
    , _gradientMode( false )
  {
  }

  void ParticleManager::init( std::shared_ptr< plab::ICamera > camera )
  {
    _staticProgram.loadFromText( STATIC_VERTEX_SHADER ,
                                 PARTICLE_FRAGMENT_SHADER );
    _staticProgram.compileAndLink( );

    _staticGradientProgram.loadFromText( STATIC_GRADIENT_VERTEX_SHADER ,
                                         PARTICLE_FRAGMENT_SHADER );
    _staticGradientProgram.compileAndLink( );

    _dynamicProgram.loadFromText( DYNAMIC_VERTEX_SHADER ,
                                  PARTICLE_FRAGMENT_SHADER );
    _dynamicProgram.compileAndLink( );

    _staticAccProgram.loadFromText( STATIC_VERTEX_SHADER ,
                                    PARTICLE_ACC_FRAGMENT_SHADER );
    _staticAccProgram.compileAndLink( );

    _staticAccGradientProgram.loadFromText( STATIC_GRADIENT_VERTEX_SHADER ,
                                            PARTICLE_ACC_FRAGMENT_SHADER );
    _staticAccGradientProgram.compileAndLink( );

    _dynamicAccProgram.loadFromText( DYNAMIC_VERTEX_SHADER ,
                                     PARTICLE_ACC_FRAGMENT_SHADER );
    _dynamicAccProgram.compileAndLink( );

    _staticRenderer = std::make_shared< plab::SimpleRenderer >(
      _staticProgram.program( ));

    _staticGradientRenderer = std::make_shared< plab::SimpleRenderer >(
      _staticGradientProgram.program( ));

    _dynamicRenderer = std::make_shared< plab::SimpleRenderer >(
      _dynamicProgram.program( ));

    _staticAccRenderer = std::make_shared< plab::SimpleRenderer >(
      _staticAccProgram.program( ));

    _staticAccGradientRenderer = std::make_shared< plab::SimpleRenderer >(
      _staticAccGradientProgram.program( ));

    _dynamicAccRenderer = std::make_shared< plab::SimpleRenderer >(
      _dynamicAccProgram.program( ));

    // SYNAPSES
    _synapseCluster = std::make_shared< plab::Cluster< SynapseParticle >>( );
    _synapseModel = std::make_shared< StaticModel >(
      camera , 8.0f , 8.0f , glm::vec4( 0.0f , 1.0f , 0.0f , 0.55f ) ,
      glm::vec4( 0.94f , 0.0f , 0.5f , 0.55f ) , true , true );

    _synapseGradientModel = std::make_shared< StaticGradientModel >(
      camera , 8.0f , 8.0f , tColorVec( ) , true , true
    );

    // PATHS
    _pathCluster = std::make_shared< plab::Cluster< SynapseParticle >>( );
    _pathModel = std::make_shared< StaticModel >(
      camera , 3.0f , 3.0f , glm::vec4( 0.75f , 0.35f , 0.09f , 0.8f ) ,
      glm::vec4( 0.0f , 0.0f , 1.0f , 0.8f ) , true , true );

    _pathCluster->setModel( _pathModel );
    _pathCluster->setRenderer( _staticRenderer );

    // DYNAMIC
    _dynamicCluster = std::make_shared< plab::Cluster< DynamicPathParticle>>( );
    _dynamicModel = std::make_shared< DynamicModel >(
      camera , 8.0f , 8.0f , glm::vec4( 1.0f ) ,
      glm::vec4( 1.0f ) , true , true , 0.0f , 0.0f , 0.5f
    );

    _dynamicCluster->setModel( _dynamicModel );
    _dynamicCluster->setRenderer( _dynamicRenderer );
  }

  const std::shared_ptr< StaticModel >&
  ParticleManager::getSynapseModel( ) const
  {
    return _synapseModel;
  }

  const std::shared_ptr< StaticGradientModel >&
  ParticleManager::getSynapseGradientModel( ) const
  {
    return _synapseGradientModel;
  }

  const std::shared_ptr< StaticModel >& ParticleManager::getPathModel( ) const
  {
    return _pathModel;
  }


  const std::shared_ptr< DynamicModel >&
  ParticleManager::getDynamicModel( ) const
  {
    return _dynamicModel;
  }

  const nlgeometry::AxisAlignedBoundingBox&
  ParticleManager::getSynapseBoundingBox( ) const
  {
    return _synapseBB;
  }

  const nlgeometry::AxisAlignedBoundingBox&
  ParticleManager::getPathBoundingBox( ) const
  {
    return _pathBB;
  }

  const nlgeometry::AxisAlignedBoundingBox&
  ParticleManager::getParticlesBoundingBox( ) const
  {
    return _particlesBoundingBox;
  }

  bool ParticleManager::isAccumulativeMode( ) const
  {
    return _synapseModel->isAccumulativeMode( );
  }

  void ParticleManager::setAccumulativeMode( bool accumulativeMode )
  {
    _synapseModel->setAccumulativeMode( accumulativeMode );
    _synapseGradientModel->setAccumulativeMode( accumulativeMode );
    _pathModel->setAccumulativeMode( accumulativeMode );
    _dynamicModel->setAccumulativeMode( accumulativeMode );

    if ( accumulativeMode )
    {
      _pathCluster->setRenderer( _staticAccRenderer );
      _dynamicCluster->setRenderer( _dynamicAccRenderer );
      _synapseCluster->setRenderer(
        _gradientMode ? _staticAccGradientRenderer : _staticAccRenderer );
    }
    else
    {
      _pathCluster->setRenderer( _staticRenderer );
      _dynamicCluster->setRenderer( _dynamicRenderer );
      _synapseCluster->setRenderer(
        _gradientMode ? _staticGradientRenderer : _staticRenderer );
    }
  }

  void ParticleManager::setSynapses( const tsynapseVec& synapses )
  {
    std::vector< SynapseParticle > particles;

    float minLimit = std::numeric_limits< float >::min( );
    float maxLimit = std::numeric_limits< float >::max( );
    glm::vec3 min( maxLimit , maxLimit , maxLimit );
    glm::vec3 max( minLimit , minLimit , minLimit );

    for ( const auto& syn: synapses )
    {
      SynapseParticle pre = SynapseParticle( );
      SynapseParticle post = SynapseParticle( );
      pre.position = eigenToGLM( syn->preSynapticSurfacePosition( ));
      post.position = eigenToGLM( syn->postSynapticSurfacePosition( ));
      pre.isPostsynaptic = 0;
      post.isPostsynaptic = 1;
      pre.value = 0;
      post.value = 0;
      particles.push_back( pre );
      particles.push_back( post );

      min = glm::min( min , pre.position );
      min = glm::min( min , post.position );
      max = glm::max( max , pre.position );
      max = glm::max( max , post.position );
    }

    _synapseBB.minimum( ) = syncopa::vec3( min.x , min.y , min.z );
    _synapseBB.maximum( ) = syncopa::vec3( max.x , max.y , max.z );
    recalculateParticlesBoundingBox( );

    _synapseCluster->setParticles( particles );
    _synapseCluster->setModel( _synapseModel );
    _synapseCluster->setRenderer(
      isAccumulativeMode( ) ? _staticAccRenderer : _staticRenderer );
    _gradientMode = false;
  }

  void ParticleManager::setMappedSynapses( const tsynapseVec& synapses ,
                                           const tFloatVec& lifeValues )
  {
    std::vector< SynapseParticle > particles;

    float minLimit = std::numeric_limits< float >::min( );
    float maxLimit = std::numeric_limits< float >::max( );
    glm::vec3 min( maxLimit , maxLimit , maxLimit );
    glm::vec3 max( minLimit , minLimit , minLimit );

    int i = 0;
    for ( const auto& syn: synapses )
    {
      float value = lifeValues.at( i++ );
      SynapseParticle pre = SynapseParticle( );
      SynapseParticle post = SynapseParticle( );
      pre.position = eigenToGLM( syn->preSynapticSurfacePosition( ));
      post.position = eigenToGLM( syn->postSynapticSurfacePosition( ));
      pre.isPostsynaptic = 0;
      post.isPostsynaptic = 1;
      pre.value = value;
      post.value = value;
      particles.push_back( pre );
      particles.push_back( post );

      min = glm::min( min , pre.position );
      min = glm::min( min , post.position );
      max = glm::max( max , pre.position );
      max = glm::max( max , post.position );
    }

    _synapseBB.minimum( ) = glmToEigen( min );
    _synapseBB.maximum( ) = glmToEigen( max );
    recalculateParticlesBoundingBox( );

    _synapseCluster->setParticles( particles );
    _synapseCluster->setModel( _synapseGradientModel );
    _synapseCluster->setRenderer(
      isAccumulativeMode( ) ? _staticAccGradientRenderer
                            : _staticGradientRenderer );
    _gradientMode = false;
  }

  void ParticleManager::setPaths(
    const std::vector< vec3 >& pre , const std::vector< vec3 >& post )
  {
    std::vector< SynapseParticle > particles;

    float minLimit = std::numeric_limits< float >::min( );
    float maxLimit = std::numeric_limits< float >::max( );
    glm::vec3 min( maxLimit , maxLimit , maxLimit );
    glm::vec3 max( minLimit , minLimit , minLimit );

    for ( const auto& pos: pre )
    {
      SynapseParticle particle = SynapseParticle( );
      particle.position = eigenToGLM( pos );
      particle.isPostsynaptic = 0;
      particles.push_back( particle );

      min = glm::min( min , particle.position );
      max = glm::max( max , particle.position );
    }

    for ( const auto& pos: post )
    {
      SynapseParticle particle = SynapseParticle( );
      particle.position = eigenToGLM( pos );
      particle.isPostsynaptic = 1;
      particles.push_back( particle );

      min = glm::min( min , particle.position );
      max = glm::max( max , particle.position );
    }

    _pathBB.minimum( ) = glmToEigen( min );
    _pathBB.maximum( ) = glmToEigen( max );
    recalculateParticlesBoundingBox( );
    _pathCluster->setParticles( particles );
  }

  void ParticleManager::setDynamic(
    const std::vector< DynamicPathParticle >& particles )
  {
    _dynamicCluster->setParticles( particles );
  }

  void ParticleManager::clearSynapses( )
  {
    _synapseCluster->allocateBuffer( 0 );
  }

  void ParticleManager::clearPaths( )
  {
    _pathCluster->allocateBuffer( 0 );
  }

  void ParticleManager::clearDynamic( )
  {
    _dynamicCluster->allocateBuffer( 0 );
  }

  void ParticleManager::draw( bool drawPaths , bool drawDynamic ) const
  {
    if ( _synapseCluster != nullptr )
    {
      _synapseCluster->render( );
    }
    if ( drawPaths && _pathCluster != nullptr )
    {
      _pathCluster->render( );
    }
    if ( drawDynamic && _dynamicCluster != nullptr )
    {
      _dynamicCluster->render( );
    }
  }

  void ParticleManager::recalculateParticlesBoundingBox( )
  {
    auto min = glm::min( eigenToGLM( _synapseBB.minimum( )) ,
                         eigenToGLM( _pathBB.minimum( )));
    auto max = glm::max( eigenToGLM( _synapseBB.maximum( )) ,
                         eigenToGLM( _pathBB.maximum( )));
    _particlesBoundingBox.minimum( ) = glmToEigen( min );
    _particlesBoundingBox.maximum( ) = glmToEigen( max );
  }
}
