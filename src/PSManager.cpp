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
  , _updaterSynapses( nullptr)
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
    _particleSystem->addSource( _sourceSynPre );
    _particleSystem->addSource( _sourceSynPost );
    _particleSystem->addSource( _sourcePathPre );
    _particleSystem->addSource( _sourcePathPost );

    _clusterSynPre = new prefr::Cluster( );
    _clusterSynPost = new prefr::Cluster( );
    _clusterPathPre = new prefr::Cluster( );
    _clusterPathPost = new prefr::Cluster( );

    _clusterSynPre->setSource( _sourceSynPre );
    _clusterSynPost->setSource( _sourceSynPost );

    _particleSystem->addCluster( _clusterSynPre );
    _particleSystem->addCluster( _clusterSynPost );
    _particleSystem->addCluster( _clusterPathPre );
    _particleSystem->addCluster( _clusterPathPost );

    _updaterSynapses = new UpdaterStaticPosition( );
    _particleSystem->addUpdater( _updaterSynapses );

    prefr::Sorter* sorter = new prefr::Sorter( );
    prefr::GLRenderer* renderer = new prefr::GLRenderer( );

    _particleSystem->sorter( sorter );
    _particleSystem->renderer( renderer );

    _particleSystem->start();
  }

  void PSManager::clear( void )
  {
    clearSynapses( ALL_CONNECTIONS );
  }

  void PSManager::clearSynapses( TNeuronConnection type )
  {
    if( type == PRESYNAPTIC )
    {
      _sourceSynPre->clear( );
      _particleSystem->detachSource( _sourceSynPre );

    }
    else if ( type == POSTSYNAPTIC )
    {

    }
    else
    {
      _sourceSynPre->clear( );
      _particleSystem->detachSource( _sourceSynPre );
    }
  }

  void PSManager::clearPaths( void )
  {
    //TODO
  }

  void PSManager::setupSynapses( const std::vector< vec3 > positions,
                                 TNeuronConnection type )
  {
    if( positions.empty( ))
      return;

    _particleSystem->run( false );

    clearSynapses( type );

    std::cerr << "Requesting particles " << positions.size( ) << std::endl;

    auto availableParticles =
        _particleSystem->retrieveUnused( positions.size( ));

    Eigen::Array3f minimum =
      Eigen::Array3f::Constant( std::numeric_limits< float >::max( ));
    Eigen::Array3f maximum =
      Eigen::Array3f::Constant( std::numeric_limits< float >::min( ));

    for( auto pos : positions )
    {
      Eigen::Array3f aux( pos );
      minimum = minimum.min( aux );
      maximum = maximum.max( aux );
    }

    _boundingBox.minimum( ) = minimum;
    _boundingBox.maximum( ) = maximum;

    std::cout << "Bounding box min: " << minimum.x( )
              << " " << minimum.y( )
              << " " << minimum.z( ) << std::endl;

    std::cout << "Bounding box max: " << maximum.x( )
                  << " " << maximum.y( )
                  << " " << maximum.z( ) << std::endl;
//    if( availableParticles.size( ) != positions.size( ))
//    {
//      std::cerr << "There are no available particles " << positions.size( ) << std::endl;
//      return;
//    }

    auto source = ( type == synvis::PRESYNAPTIC ? _sourceSynPre : _sourceSynPost );
    auto cluster = ( type == synvis::PRESYNAPTIC ? _clusterSynPre : _clusterSynPost );

    source->addPositions( availableParticles.indices( ), positions );

    cluster->particles( availableParticles );
    std::cout << "Cluster " << cluster->particles( ).size( ) << " " << availableParticles.size( ) << std::endl;

    cluster->setModel( _modelSynPre );
    cluster->setUpdater( _updaterSynapses );
//    cluster->setSource( source );
    _particleSystem->addSource( source, cluster->particles( ).indices( ) );

    std::cout << "Source " << source->particles( ).size( ) << " " << availableParticles.size( ) << std::endl;

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


  nlgeometry::AxisAlignedBoundingBox PSManager::boundingBox( void ) const
  {
    return _boundingBox;
  }



}
