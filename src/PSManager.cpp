/*
 * @file  PSManager.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#include "PSManager.h"

namespace synvis
{

  PSManager::PSManager( void )
  : _particleSystem( nullptr )
  , _maxParticles( 0 )
  , _modelSynPre( nullptr )
  , _modelSynPost( nullptr )
  , _modelPathPre( nullptr )
  , _modelPathPost( nullptr )
  , _sourceSynPre( nullptr )
  , _sourceSynPost( nullptr )
  , _sourcePathPre( nullptr )
  , _sourcePathPost( nullptr )
  , _clusterSynPre( nullptr )
  , _clusterSynPost( nullptr )
  , _clusterPathPre( nullptr )
  , _clusterPathPost( nullptr )
  { }

  PSManager::~PSManager( void )
  {
    if( _particleSystem )
      delete _particleSystem;
  }

  prefr::ParticleSystem* PSManager::particleSystem( void )
  {
    return _particleSystem;
  }

  void PSManager::init( unsigned int maxParticles )
  {
//    unsigned int maxParticles = 500000;

    _particleSystem = new prefr::ParticleSystem( maxParticles );

    _particleSystem->parallel( true );

    prefr::Model model = prefr::Model( 1.0f, 1.0f );
    model.velocity.Insert( 0.0f, 0.0f );
    model.size.Insert( 0.0f, 5.0f );
    model.color.Insert( 0.0f, glm::vec4( 1, 1, 1, 1 ));

    _modelSynPre = new prefr::Model( model );
    _modelSynPost = new prefr::Model( model );
    _modelPathPre = new prefr::Model( model );
    _modelPathPost= new prefr::Model( model );

    _sourceSynPre = new SourceMultiPosition( );
    _sourceSynPost = new SourceMultiPosition( );
    _sourcePathPre = new SourceMultiPosition( );
    _sourcePathPost = new SourceMultiPosition( );

    prefr::ParticleSet emptyParticleSet;
    _particleSystem->addSource( _sourceSynPre, { 0, 1} );
//    _particleSystem->addSource( _sourceSynPost );
//    _particleSystem->addSource( _sourcePathPre );
//    _particleSystem->addSource( _sourcePathPost );

    _clusterSynPre = new prefr::Cluster( );
    _clusterSynPost = new prefr::Cluster( );
    _clusterPathPre = new prefr::Cluster( );
    _clusterPathPost = new prefr::Cluster( );

    _particleSystem->addCluster( _clusterSynPre );
    _particleSystem->addCluster( _clusterSynPost );
    _particleSystem->addCluster( _clusterPathPre );
    _particleSystem->addCluster( _clusterPathPost );

    prefr::Sorter* sorter = new prefr::Sorter( );
    prefr::GLRenderer* renderer = new prefr::GLRenderer( );

    _particleSystem->sorter( sorter );
    _particleSystem->renderer( renderer );

    _particleSystem->start();
  }

  void PSManager::clear( void )
  {
    //TODO
  }

  void PSManager::clearSynapses( TNeuronConnection type )
  {
    if( type == PRESYNAPTIC )
    {
      _particleSystem->detachSource( _sourceSynPre );
    }
    else if ( type == POSTSYNAPTIC )
    {

    }
    else
    {

    }
  }

  void PSManager::clearPaths( void )
  {
    //TODO
  }

  void PSManager::setupSynapses( const std::vector< vec3 > positions,
                                 TNeuronConnection /*type*/ )
  {
    if( positions.empty( ))
      return;

    _particleSystem->run( false );

//    std::cerr << "Requesting particles " << positions.size( ) << std::endl;
//
//    auto availableParticles =
//        _particleSystem->retrieveUnused( positions.size( ));
//
//    if( availableParticles.size( ) != positions.size( ))
//    {
//      std::cerr << "There are no available particles " << positions.size( ) << std::endl;
//      return;
//    }
//
//    auto source = ( type == synvis::PRESYNAPTIC ? _sourceSynPre : _sourceSynPost );
//
//    source->addPositions( availableParticles, positions );
//
////    _clusterSynPre->particles( ).addIndices( availableParticles );
//    _clusterSynPre->setModel( _modelSynPre );

//    _particleSystem->addSource( _)

    _particleSystem->run( true );
  }

  void PSManager::setupPath( const std::vector< vec3 > /*nodePositions*/,
                             TNeuronConnection /*type*/ )
  {
    //TODO
  }

  vec4 PSManager::colorSynapses( TNeuronConnection type ) const
  {
    if( type == PRESYNAPTIC )
      return glmToEigen( _modelSynPre->color.GetFirstValue( ));
    else
      return glmToEigen( _modelSynPost->color.GetFirstValue( ));
  }

  void PSManager::colorSynapses( const vec4& color, TNeuronConnection type )
  {
    if( type == PRESYNAPTIC )
      _modelSynPre->color.Insert( 0, eigenToGLM( color ));
    else if( type == POSTSYNAPTIC )
      _modelSynPost->color.Insert( 0, eigenToGLM( color ));
    else
    {
      _modelSynPre->color.Insert( 0, eigenToGLM( color ));
      _modelSynPost->color.Insert( 0, eigenToGLM( color ));
    }
  }


  float PSManager::sizeSynapses( TNeuronConnection type ) const
  {
    return ( type == PRESYNAPTIC ? _modelSynPre : _modelSynPost )->size.GetFirstValue( );
  }

  void PSManager::sizeSynapses( float size, TNeuronConnection type )
  {
    if( type == PRESYNAPTIC )
      _modelSynPre->size.Insert( 0, size );
    else if( type == POSTSYNAPTIC )
      _modelSynPost->size.Insert( 0, size );
    else
    {
      _modelSynPre->size.Insert( 0, size );
      _modelSynPost->size.Insert( 0, size );
    }
  }

  vec4 PSManager::colorPaths( TNeuronConnection type ) const
  {
    if( type == PRESYNAPTIC )
      return glmToEigen( _modelPathPre->color.GetFirstValue( ));
    else
      return glmToEigen( _modelPathPost->color.GetFirstValue( ));
  }

  void PSManager::colorPaths( const vec4& /*color*/, TNeuronConnection /*type*/ )
  {

  }




}
