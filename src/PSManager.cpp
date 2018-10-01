/*
 * @file  PSManager.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#include "PSManager.h"

namespace syncopa
{

  PSManager::PSManager( void )
  : _particleSystem( nullptr )
  , _maxParticles( 0 )
  , _modelSynPre( nullptr )
  , _modelSynPost( nullptr )
  , _modelPathPre( nullptr )
  , _modelPathPost( nullptr )
  , _modelDynPre( nullptr )
  , _modelDynPost( nullptr )
  , _dynamicVelocityModule( 200 )
  , _sourceSynPre( nullptr )
  , _sourceSynPost( nullptr )
  , _sourcePathPre( nullptr )
  , _sourcePathPost( nullptr )
  , _sampler( nullptr )
  , _totalDynamicSources( 20 )
  , _particlesPerDynamicSource( 2000 )
  , _mobileSourcesEmissionRate( 0.2f )
  , _clusterDyn( nullptr )
  , _clusterSynPre( nullptr )
  , _clusterSynPost( nullptr )
  , _clusterPathPre( nullptr )
  , _clusterPathPost( nullptr )
  , _updaterSynapses( nullptr)
  , _normalUpdater( nullptr )
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

    _particleSystem->parallel( false );

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
    _clusterPathPre->setSource( _sourcePathPre );
    _clusterPathPost->setSource( _sourcePathPost );


    _particleSystem->addCluster( _clusterSynPre );
    _particleSystem->addCluster( _clusterSynPost );
    _particleSystem->addCluster( _clusterPathPre );
    _particleSystem->addCluster( _clusterPathPost );

    _updaterSynapses = new UpdaterStaticPosition( );
    _particleSystem->addUpdater( _updaterSynapses );

    _normalUpdater = new prefr::Updater( );
    _particleSystem->addUpdater( _normalUpdater );
//TODO
    _sampler = new prefr::SphereSampler( 1, 360 );

    _clusterDyn = new prefr::Cluster( );

    _modelDynPre = new prefr::Model( model );
    _modelDynPre->setLife( 1, 1 );
    _modelDynPre->color.Insert( 0.0f, glm::vec4( 0.5, 0.5, 1, 0.7 ));
    _modelDynPre->color.Insert( 0.25f, glm::vec4( 0, 0.5, 1, 0.2 ));
    _modelDynPre->color.Insert( 1.0f, glm::vec4( 0, 1, 1, 0 ));

    _modelDynPre->size.Insert( 0.0f, 10 );
    _modelDynPre->size.Insert( 0.2f, 5 );
    _modelDynPre->size.Insert( 1.0f, 1 );

    _modelDynPre->velocity.Insert( 0.0f, 0.0f );

    _modelDynPost = new prefr::Model( *_modelDynPre );
    _modelDynPost->color.Insert( 0.0f, glm::vec4( 1, 0.5, 0.5, 0.7 ));
    _modelDynPost->color.Insert( 0.25f, glm::vec4( 1, 0.5, 0, 0.2 ));
    _modelDynPost->color.Insert( 1.0f, glm::vec4( 1, 1, 0, 0 ));


    _particleSystem->addCluster( _clusterDyn );

    prefr::Sorter* sorter = new prefr::Sorter( );
    prefr::GLRenderer* renderer = new prefr::GLRenderer( );

    _particleSystem->sorter( sorter );
    _particleSystem->renderer( renderer );

    _particleSystem->start();
  }

  void PSManager::run ( void )
  {
    _particleSystem->run( true );
  }

  void PSManager::stop( void )
  {
    _particleSystem->run( false );
  }

  void PSManager::clear( void )
  {
    clearSynapses( ALL_CONNECTIONS );
    clearPaths( ALL_CONNECTIONS );
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
      _sourceSynPost->clear( );
      _particleSystem->detachSource( _sourceSynPost );
    }
    else
    {
      _sourceSynPre->clear( );
      _particleSystem->detachSource( _sourceSynPre );

      _sourceSynPost->clear( );
      _particleSystem->detachSource( _sourceSynPost );
    }
  }

  void PSManager::clearPaths( TNeuronConnection type )
  {
    if( type == PRESYNAPTIC )
    {
      _sourcePathPre->clear( );
      _particleSystem->detachSource( _sourcePathPre );

    }
    else if ( type == POSTSYNAPTIC )
    {
      _sourcePathPost->clear( );
      _particleSystem->detachSource( _sourcePathPost );
    }
    else
    {
      _sourcePathPre->clear( );
      _particleSystem->detachSource( _sourcePathPre );

      _sourcePathPost->clear( );
      _particleSystem->detachSource( _sourcePathPost );
    }
  }

  void PSManager::setupSynapses( const std::vector< vec3 > positions,
                                 TNeuronConnection type )
  {
    if( positions.empty( ))
      return;

    _particleSystem->run( true );

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

    auto source = ( type == syncopa::PRESYNAPTIC ? _sourceSynPre : _sourceSynPost );
    auto cluster = ( type == syncopa::PRESYNAPTIC ? _clusterSynPre : _clusterSynPost );
    auto model = ( type == syncopa::PRESYNAPTIC ? _modelSynPre : _modelSynPost );

    source->addPositions( availableParticles.indices( ), positions );

    cluster->particles( availableParticles );
//    std::cout << "Cluster " << cluster->particles( ).size( ) << " " << availableParticles.size( ) << std::endl;

    cluster->setModel( model );
    cluster->setUpdater( _updaterSynapses );
//    cluster->setSource( source );
    _particleSystem->addSource( source, cluster->particles( ).indices( ) );

    std::cout << "Source " << source->particles( ).size( ) << " " << availableParticles.size( ) << std::endl;

    _particleSystem->run( true );
  }

  void PSManager::setupPath( const std::vector< vec3 > positions,
                             TNeuronConnection type )
  {
    if( positions.empty( ))
      return;

    _particleSystem->run( false );

    clearPaths( type );

    std::cerr << "Requesting particles " << positions.size( ) << std::endl;

    auto availableParticles =
        _particleSystem->retrieveUnused( positions.size( ));

//    Eigen::Array3f minimum =
//      Eigen::Array3f::Constant( std::numeric_limits< float >::max( ));
//    Eigen::Array3f maximum =
//      Eigen::Array3f::Constant( std::numeric_limits< float >::min( ));

//    for( auto pos : positions )
//    {
//      Eigen::Array3f aux( pos );
//      minimum = minimum.min( aux );
//      maximum = maximum.max( aux );
//    }
//
//    _boundingBox.minimum( ) = minimum;
//    _boundingBox.maximum( ) = maximum;
//
//    std::cout << "Bounding box min: " << minimum.x( )
//              << " " << minimum.y( )
//              << " " << minimum.z( ) << std::endl;
//
//    std::cout << "Bounding box max: " << maximum.x( )
//                  << " " << maximum.y( )
//                  << " " << maximum.z( ) << std::endl;


//    if( availableParticles.size( ) != positions.size( ))
//    {
//      std::cerr << "There are no available particles " << positions.size( ) << std::endl;
//      return;
//    }

    auto source = ( type == syncopa::PRESYNAPTIC ? _sourcePathPre : _sourcePathPost );
    auto cluster = ( type == syncopa::PRESYNAPTIC ? _clusterPathPre : _clusterPathPost );
    auto model = ( type == syncopa::PRESYNAPTIC ? _modelPathPre : _modelPathPost );

    source->addPositions( availableParticles.indices( ), positions );

    cluster->particles( availableParticles );
//    std::cout << "Cluster " << cluster->particles( ).size( ) << " " << availableParticles.size( ) << std::endl;

    cluster->setModel( model );
    cluster->setUpdater( _updaterSynapses );
//    cluster->setSource( source );
    _particleSystem->addSource( source, cluster->particles( ).indices( ) );

    std::cout << "Source " << source->particles( ).size( ) << " " << availableParticles.size( ) << std::endl;

    _particleSystem->run( true );
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

  void PSManager::colorPaths( const vec4& color, TNeuronConnection type )
  {
    if( type == PRESYNAPTIC )
      _modelPathPre->color.Insert( 0, eigenToGLM( color ));
    else if( type == POSTSYNAPTIC )
      _modelPathPost->color.Insert( 0, eigenToGLM( color ));
    else
    {
      _modelPathPre->color.Insert( 0, eigenToGLM( color ));
      _modelPathPost->color.Insert( 0, eigenToGLM( color ));
    }
  }


  float PSManager::sizePaths( TNeuronConnection type ) const
  {
    return ( type == PRESYNAPTIC ? _modelPathPre : _modelPathPost )->size.GetFirstValue( );
  }

  void PSManager::sizePaths( float size, TNeuronConnection type )
  {
    if( type == PRESYNAPTIC || type == ALL_CONNECTIONS )
      _modelPathPre->size.Insert( 0, size );

    if( type == POSTSYNAPTIC || type == ALL_CONNECTIONS)
      _modelPathPost->size.Insert( 0, size );
  }

  nlgeometry::AxisAlignedBoundingBox PSManager::boundingBox( void ) const
  {
    return _boundingBox;
  }

  void PSManager::showSynapses( bool state, TNeuronConnection type )
  {
    if( type == PRESYNAPTIC || type == ALL_CONNECTIONS )
    {
      _sourceSynPre->active( state );
    }

    if ( type == POSTSYNAPTIC || type == ALL_CONNECTIONS )
    {
      _sourceSynPost->active( state );
    }

  }

  void PSManager::showPaths( bool state, TNeuronConnection type )
  {
    if( type == PRESYNAPTIC || type == ALL_CONNECTIONS )
    {
      _sourcePathPre->active( state );
    }

    if ( type == POSTSYNAPTIC || type == ALL_CONNECTIONS )
    {
      _sourcePathPost->active( state );
    }
  }

  MobilePolylineSource* PSManager::getSpareMobileSouce( TNeuronConnection type )
  {

    MobilePolylineSource* result;

    if( _availableDynamicSources.empty( ))
    {
      result = new MobilePolylineSource( _mobileSourcesEmissionRate, vec3( 0, 0, 0 ));
      std::cout << "Creating new source " << result->gid( ) << std::endl;
    }
    else
    {
      result = *_availableDynamicSources.begin( );
      _availableDynamicSources.erase( result );
//      std::cout << "Using old source " << result->gid( ) << std::endl;
    }

    _dynamicSources.insert( result );

    auto indices = _particleSystem->retrieveUnused( _particlesPerDynamicSource );
//    result->particles( ) = indices;

    _clusterDyn->particles( indices );

    _clusterDyn->setModel(( type == PRESYNAPTIC ) ? _modelDynPre : _modelDynPost );
    _clusterDyn->setUpdater( _normalUpdater );

    result->delay( 0 );
    result->maxEmissionCycles( 0 );
    result->autoDeactivateWhenFinished( true );
    result->velocityModule( _dynamicVelocityModule );

    result->sampler( _sampler );

    _particleSystem->addSource( result, indices.indices( ) );

    result->restart( );

    return result;
  }

  void PSManager::releaseMobileSource( MobilePolylineSource* source_ )
  {
    _availableDynamicSources.insert( source_ );
    _dynamicSources.erase( source_ );

    source_->finishedPath.disconnect_all_slots( );
    source_->finishedSection.disconnect_all_slots( );
    source_->reachedSynapse.disconnect_all_slots( );

    std::cout << "-- Releasing source " << source_->gid( )
              << " type " << (unsigned int) source_->functionType( ) << std::endl;

    _particleSystem->detachSource( source_ );
  }


  void PSManager::setupDynamicPath( const tPosVec&  )
  {
//    _particleSystem->detachSource( _sourceDynPre );
//
//    tPosVec posit;
////    posit.push_back( vec3( 0, 0, 0 ));
////    posit.push_back( vec3( 50, 50, 50 ));
////    posit.push_back( vec3( 100, 100, 100 ));
//    posit = positions;
//
//    _boundingBox.clear( );
//    for( auto pos : posit )
//    {
//      _boundingBox.expand( pos );
//    }
//
//    _sourceDynPre->path( posit );
//    _sourceDynPre->velocityModule( 100 );
//
//    auto indices = _particleSystem->retrieveUnused( 1000 );
//    // TODO SETUP PARTICLES AND SOURCE VALUES
//    std::cout << "Using indices";
//    for( auto idx : indices )
//      std::cout << " " << idx.id( );
//    std::cout << std::endl;
//
//    _clusterDyn->particles( indices );
//    _clusterDyn->setSource( _sourceDynPre );
//    _clusterDyn->setUpdater( _normalUpdater );
//    _clusterDyn->setModel( _modelDyn );
//    _particleSystem->addSource( _sourceDynPre, indices.indices( ));

//    _sourceDynPre->restart( );
  }

}
