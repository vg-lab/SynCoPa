/*
 * @file  PSManager.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#include "PSManager.h"

#include <QDebug>

namespace syncopa
{
  PSManager::PSManager( void )
    : _particleSystem( nullptr )
    , _pathFinder( nullptr )
    , _maxParticles( 0 )
    , _modelSynPre( nullptr )
    , _modelSynPost( nullptr )
    , _modelSynMap( nullptr )
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
    , _updaterSynapses( nullptr )
    , _updaterMapperSynapses( nullptr )
    , _normalUpdater( nullptr )
  { }

  PSManager::~PSManager( void )
  {
    if ( _particleSystem )
      delete _particleSystem;
  }

  prefr::ParticleSystem* PSManager::particleSystem( void )
  {
    return _particleSystem;
  }

  void PSManager::init( PathFinder* pathFinder ,
                        unsigned int maxParticles ,
                        prefr::ICamera* camera ,
                        prefr::IGLRenderProgram* program
                      )
  {
    _pathFinder = pathFinder;

    _particleSystem = new prefr::ParticleSystem( maxParticles , camera );
    _particleSystem->parallel( true );

    prefr::Model model = prefr::Model( 1.0f , 1.0f );
    model.velocity.Insert( 0.0f , 0.0f );
    model.size.Insert( 0.0f , 5.0f );
    model.color.Insert( 0.0f , glm::vec4( 1 , 1 , 1 , 1 ));

    _modelSynPre = new prefr::Model( model );
    _modelSynPost = new prefr::Model( model );
    _modelPathPre = new prefr::Model( model );
    _modelPathPost = new prefr::Model( model );

    _modelSynMap = new prefr::Model( );
    _modelSynMap->velocity.Insert( 0.0f , 0.0f );

    _modelSynMap->size.Insert( 0.0f , 8.0f );
    _modelSynMap->color.Insert( 0.0f , glm::vec4( 1 , 0 , 0 , 0.8 ));
    _modelSynMap->color.Insert( 1.0f , glm::vec4( 1 , 1 , 0 , 0.8 ));

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

    _particleSystem->addCluster( _clusterSynPre );
    _particleSystem->addCluster( _clusterSynPost );
    _particleSystem->addCluster( _clusterPathPre );
    _particleSystem->addCluster( _clusterPathPost );

    _clusterSynPre->setSource( _sourceSynPre );
    _clusterSynPost->setSource( _sourceSynPost );
    _clusterPathPre->setSource( _sourcePathPre );
    _clusterPathPost->setSource( _sourcePathPost );

    _updaterSynapses = new UpdaterStaticPosition( );
    _updaterMapperSynapses = new UpdaterMappedValue( );

    _particleSystem->addUpdater( _updaterSynapses );
    _particleSystem->addUpdater( _updaterMapperSynapses );

    _normalUpdater = new prefr::Updater( );
    _particleSystem->addUpdater( _normalUpdater );
//TODO
    _sampler = new prefr::SphereSampler( 1 , 360 );

    _clusterDyn = new prefr::Cluster( );

    _modelDynPre = new prefr::Model( model );
    _modelDynPre->setLife( 1 , 1 );
    _modelDynPre->color.Insert( 0.0f , glm::vec4( 1 , 1 , 1 , 0.7 ));
    _modelDynPre->color.Insert( 0.25f , glm::vec4( 1 , 1 , 1 , 0.2 ));
    _modelDynPre->color.Insert( 1.0f , glm::vec4( 1 , 1 , 1 , 0 ));

    _modelDynPre->size.Insert( 0.0f , 10 );
    _modelDynPre->size.Insert( 0.2f , 5 );
    _modelDynPre->size.Insert( 1.0f , 1 );

    _modelDynPre->velocity.Insert( 0.0f , 0.0f );

    _modelDynPost = new prefr::Model( );
    _modelDynPost->setLife( 1 , 1 );

    _modelDynPost->color.Insert( 0.0f , glm::vec4( 1 , 1 , 1 , 0.7 ));
    _modelDynPost->color.Insert( 0.25f , glm::vec4( 1 , 1 , 1 , 0.2 ));
    _modelDynPost->color.Insert( 1.0f , glm::vec4( 1 , 1 , 1 , 0 ));

    _modelDynPost->size.Insert( 0.0f , 10 );
    _modelDynPost->size.Insert( 0.2f , 5 );
    _modelDynPost->size.Insert( 1.0f , 1 );

    _modelDynPost->velocity.Insert( 0.0f , 0.0f );

    _particleSystem->addCluster( _clusterDyn );

    prefr::Sorter* sorter = new prefr::Sorter( );
    prefr::OIGLRenderer* renderer = new prefr::OIGLRenderer( program );

    _particleSystem->sorter( sorter );
    _particleSystem->renderer( renderer );
    _particleSystem->start( );
  }

  void PSManager::run( void )
  {
    _particleSystem->run( true );
  }

  void PSManager::stop( void )
  {
    _particleSystem->run( false );
  }

  void PSManager::clear( void )
  {
    clearSynapses( );
    clearPaths( ALL_CONNECTIONS );
  }

  void PSManager::clearSynapses( )
  {
    _sourceSynPre->clear( );
    _particleSystem->detachSource( _sourceSynPre );

    _sourceSynPost->clear( );
    _particleSystem->detachSource( _sourceSynPost );
  }

  void PSManager::clearPaths( TNeuronConnection type )
  {
    switch ( type )
    {
      default:
        /* fall through */
      case TNeuronConnection::PRESYNAPTIC:
        _sourcePathPre->clear( );
        _particleSystem->detachSource( _sourcePathPre );
        if ( type == TNeuronConnection::PRESYNAPTIC ) break;
        /* fall through */
      case TNeuronConnection::POSTSYNAPTIC:
        _sourcePathPost->clear( );
        _particleSystem->detachSource( _sourcePathPost );
    }
  }

  void PSManager::_updateBoundingBox( const std::vector< vec3 > positions ,
                                      bool clear_ )
  {
    Eigen::Array3f minimum , maximum;

    if ( clear_ )
    {
      minimum = Eigen::Array3f::Constant( std::numeric_limits< float >::max( ));
      maximum = Eigen::Array3f::Constant( std::numeric_limits< float >::min( ));
    }
    else
    {
      minimum = _boundingBox.minimum( );
      maximum = _boundingBox.maximum( );
    }

    auto updateMinMax = [ &minimum , &maximum ]( const Eigen::Vector3f& v )
    {
      const Eigen::Array3f aux( v );
      minimum = minimum.min( aux );
      maximum = maximum.max( aux );
    };
    std::for_each( positions.cbegin( ) , positions.cend( ) , updateMinMax );

    _boundingBox.minimum( ) = minimum;
    _boundingBox.maximum( ) = maximum;
  }

  void PSManager::configureSynapses( const tsynapseVec& synapses ,
                                     const TNeuronConnection type )
  {
    std::vector< vec3 > positionsPre;
    std::vector< vec3 > positionsPost;
    positionsPre.reserve( synapses.size( ));
    positionsPost.reserve( synapses.size( ));

    auto addSynapse = [ &positionsPre , &positionsPost ](
      const nsolMSynapse_ptr syn )
    {
      positionsPre.push_back( syn->preSynapticSurfacePosition( ));
      positionsPost.push_back( syn->postSynapticSurfacePosition( ));
    };
    std::for_each( synapses.cbegin( ) , synapses.cend( ) , addSynapse );

    clearSynapses( );
    switch ( type )
    {
      case TNeuronConnection::PRESYNAPTIC:
        setupSynapses( positionsPre , PRESYNAPTIC );
        break;
      case TNeuronConnection::POSTSYNAPTIC:
        setupSynapses( positionsPost , POSTSYNAPTIC );
        break;
      default:
        setupSynapses( positionsPre , PRESYNAPTIC );
        setupSynapses( positionsPost , POSTSYNAPTIC );
        break;
    }
  }

  void PSManager::configureMappedSynapses( const tsynapseVec& synapses ,
                                           const tFloatVec& lifeValues ,
                                           TNeuronConnection type )
  {
    assert( synapses.size( ) == lifeValues.size( ));

    std::vector< vec3 > positionsPre;
    std::vector< vec3 > positionsPost;
    positionsPre.reserve( synapses.size( ));
    positionsPost.reserve( synapses.size( ));

    auto addSynapse = [ &positionsPre , &positionsPost ](
      const nsolMSynapse_ptr syn )
    {
      positionsPre.push_back( syn->preSynapticSurfacePosition( ));
      positionsPost.push_back( syn->postSynapticSurfacePosition( ));
    };
    std::for_each( synapses.cbegin( ) , synapses.cend( ) , addSynapse );

    clearSynapses( );
    switch ( type )
    {
      default:
        /* fall through */
      case TNeuronConnection::PRESYNAPTIC:
        _mapSynapses( positionsPre , lifeValues , PRESYNAPTIC );
        if ( type == TNeuronConnection::PRESYNAPTIC ) break;
        /* fall through */
      case TNeuronConnection::POSTSYNAPTIC:
        _mapSynapses( positionsPost , lifeValues , POSTSYNAPTIC );
    }
  }

  void PSManager::_mapSynapses( const tPosVec& positions ,
                                const tFloatVec& lifeValues ,
                                TNeuronConnection type )
  {
    if ( positions.empty( ))
      return;

    assert( positions.size( ) == lifeValues.size( ));

    auto availableParticles = _particleSystem->retrieveUnused(
      positions.size( ));

    auto source = ( type == syncopa::PRESYNAPTIC ? _sourceSynPre
                                                 : _sourceSynPost );
    auto cluster = ( type == syncopa::PRESYNAPTIC ? _clusterSynPre
                                                  : _clusterSynPost );
    auto model = ( type == syncopa::PRESYNAPTIC ? _modelSynMap
                                                : _modelSynMap ); //TODO

    for ( unsigned int i = 0; i < lifeValues.size( ); ++i )
    {
      auto particle = availableParticles.at( i );

      particle.set_life( lifeValues[ i ] );
    }

    source->addPositions( availableParticles.indices( ) , positions );

    cluster->particles( availableParticles );

    cluster->setModel( model );
    cluster->setUpdater( _updaterMapperSynapses );
    _particleSystem->addSource( source , cluster->particles( ).indices( ));
  }

  void PSManager::setupSynapses( const std::vector< vec3 > positions ,
                                 TNeuronConnection type )
  {
    if ( positions.empty( )) return;

    auto availableParticles =
      _particleSystem->retrieveUnused( positions.size( ));

    auto source = ( type == syncopa::PRESYNAPTIC ? _sourceSynPre
                                                 : _sourceSynPost );
    auto cluster = ( type == syncopa::PRESYNAPTIC ? _clusterSynPre
                                                  : _clusterSynPost );
    auto model = ( type == syncopa::PRESYNAPTIC ? _modelSynPre
                                                : _modelSynPost );

    source->addPositions( availableParticles.indices( ) , positions );
    cluster->particles( availableParticles );

    cluster->setModel( model );
    cluster->setUpdater( _updaterSynapses );
    _particleSystem->addSource( source , cluster->particles( ).indices( ));

    _updateBoundingBox(positions);
  }

  void PSManager::setupPath( const std::vector< vec3 > positions ,
                             TNeuronConnection type, bool clear )
  {
    clearPaths( type );

    if ( positions.empty( )) return;

    _particleSystem->run( false );

    auto availableParticles = _particleSystem->retrieveUnused(
      positions.size( ));

    auto source = ( type == syncopa::PRESYNAPTIC ? _sourcePathPre
                                                 : _sourcePathPost );
    auto cluster = ( type == syncopa::PRESYNAPTIC ? _clusterPathPre
                                                  : _clusterPathPost );
    auto model = ( type == syncopa::PRESYNAPTIC ? _modelPathPre
                                                : _modelPathPost );

    source->addPositions( availableParticles.indices( ) , positions );

    cluster->particles( availableParticles );

    cluster->setModel( model );
    cluster->setUpdater( _updaterSynapses );
    _particleSystem->addSource( source , cluster->particles( ).indices( ));

    _updateBoundingBox(positions, clear);

    _particleSystem->run( true );
  }

  vec4 PSManager::colorSynapses( TNeuronConnection type ) const
  {
    if ( type == PRESYNAPTIC )
      return glmToEigen( _modelSynPre->color.GetFirstValue( ));
    else
      return glmToEigen( _modelSynPost->color.GetFirstValue( ));
  }

  void PSManager::colorSynapses( const vec4& color , TNeuronConnection type )
  {
    switch ( type )
    {
      default:
        /* fall through */
      case TNeuronConnection::PRESYNAPTIC:
        _colorSynPre = color;
        _modelSynPre->color.Clear( );
        _modelSynPre->color.Insert( 0 , eigenToGLM( color ));
        if ( type == TNeuronConnection::PRESYNAPTIC ) break;
        /* fall through */
      case TNeuronConnection::POSTSYNAPTIC:
        _colorSynPost = color;
        _modelSynPost->color.Clear( );
        _modelSynPost->color.Insert( 0 , eigenToGLM( color ));
    }
  }

  float PSManager::sizeSynapses( TNeuronConnection type ) const
  {
    return ( type == PRESYNAPTIC ? _modelSynPre
                                 : _modelSynPost )->size.GetFirstValue( );
  }

  void PSManager::sizeSynapses( float size , TNeuronConnection type )
  {
    switch ( type )
    {
      default:
        /* fall through */
      case TNeuronConnection::PRESYNAPTIC:
        _modelSynPre->size.Insert( 0 , size );
        if ( type == TNeuronConnection::PRESYNAPTIC ) break;
        /* fall through */
      case TNeuronConnection::POSTSYNAPTIC:
        _modelSynPost->size.Insert( 0 , size );
    }
  }

  vec4 PSManager::colorPaths( TNeuronConnection type ) const
  {
    if ( type == PRESYNAPTIC )
      return glmToEigen( _modelPathPre->color.GetFirstValue( ));
    else
      return glmToEigen( _modelPathPost->color.GetFirstValue( ));
  }

  void PSManager::colorPaths( const vec4& color , TNeuronConnection type )
  {
    switch ( type )
    {
      default:
        /* fall through */
      case TNeuronConnection::PRESYNAPTIC:
        _colorPathPre = color;
        _modelPathPre->color.Insert( 0 , eigenToGLM( color ));
        if ( type == TNeuronConnection::PRESYNAPTIC ) break;
        /* fall through */
      case TNeuronConnection::POSTSYNAPTIC:
        _colorPathPost = color;
        _modelPathPost->color.Insert( 0 , eigenToGLM( color ));
        break;
    }
  }

  void PSManager::colorSynapseMap( const tColorVec& colors )
  {
    _modelSynMap->color.Clear( );

    auto insertColor = [ this ]( const std::pair< float , glm::vec4 >& color )
    { _modelSynMap->color.Insert( color.first , color.second ); };
    std::for_each( colors.cbegin( ) , colors.cend( ) , insertColor );
  }

  float PSManager::sizePaths( TNeuronConnection type ) const
  {
    return ( type == PRESYNAPTIC ? _modelPathPre
                                 : _modelPathPost )->size.GetFirstValue( );
  }

  void PSManager::sizePaths( float size , TNeuronConnection type )
  {
    switch ( type )
    {
      default:
        /* fall through */
      case TNeuronConnection::PRESYNAPTIC:
        _modelPathPre->size.Insert( 0 , size );
        if ( type == TNeuronConnection::PRESYNAPTIC ) break;
        /* fall through */
      case TNeuronConnection::POSTSYNAPTIC:
        _modelPathPost->size.Insert( 0 , size );
        break;
    }
  }

  float PSManager::sizeSynapseMap( TNeuronConnection ) const
  {
    return _modelSynMap->size.GetFirstValue( );
  }

  void PSManager::sizeSynapsesMap( float newSize , TNeuronConnection )
  {
    _modelSynMap->size.Insert( 0 , newSize );
  }

  float PSManager::sizeDynamic( void ) const
  {
    return _modelDynPre->size.GetFirstValue( );
  }

  void PSManager::sizeDynamic( float newSize )
  {
    _modelDynPre->size.Insert( 0.0f , newSize );
    _modelDynPost->size.Insert( 0.0f , newSize );
  }

  QColor PSManager::getPreDynamicColor( )
  {
    if ( _modelDynPre->color.size == 0 ) return Qt::white;
    auto fetch = _modelDynPre->color.values[ 0 ];
    return {
      static_cast<int>(fetch.r * 255) ,
      static_cast<int>(fetch.g * 255) ,
      static_cast<int>(fetch.b * 255) ,
    };
  }

  QColor PSManager::getPostDynamicColor( )
  {
    if ( _modelDynPre->color.size == 0 ) return Qt::white;
    auto fetch = _modelDynPre->color.values[ 0 ];
    return {
      static_cast<int>(fetch.r * 255) ,
      static_cast<int>(fetch.g * 255) ,
      static_cast<int>(fetch.b * 255) ,
    };
  }

  void PSManager::setPreDynamicColor( const QColor& color )
  {
    glm::vec4 vecColor( color.redF( ) , color.greenF( ) , color.blueF( ) ,
                        0.7 );

    _modelDynPre->color.Clear( );
    _modelDynPre->color.Insert( 0.0f , vecColor );
    vecColor.a = 0.2;
    _modelDynPre->color.Insert( 0.25f , vecColor );
    vecColor.a = 0.0;
    _modelDynPre->color.Insert( 1.0f , vecColor );
  }

  void PSManager::setPostDynamicColor( const QColor& color )
  {
    glm::vec4 vecColor( color.redF( ) , color.greenF( ) , color.blueF( ) ,
                        0.7 );

    _modelDynPost->color.Clear( );
    _modelDynPost->color.Insert( 0.0f , vecColor );
    vecColor.a = 0.2;
    _modelDynPost->color.Insert( 0.25f , vecColor );
    vecColor.a = 0.0;
    _modelDynPost->color.Insert( 1.0f , vecColor );
  }

  nlgeometry::AxisAlignedBoundingBox PSManager::boundingBox( void ) const
  {
    return _boundingBox;
  }

  void PSManager::showSynapses( bool state , TNeuronConnection type )
  {
    switch ( type )
    {
      default:
        /* fall through */
      case TNeuronConnection::PRESYNAPTIC:
        _sourceSynPre->active( state );
        if ( type == TNeuronConnection::PRESYNAPTIC ) break;
        /* fall through */
      case TNeuronConnection::POSTSYNAPTIC:
        _sourceSynPost->active( state );
    }
  }

  void PSManager::showPaths( bool state , TNeuronConnection type )
  {
    switch ( type )
    {
      default:
        /* fall through */
      case TNeuronConnection::PRESYNAPTIC:
        _sourcePathPre->active( state );
        if ( type == TNeuronConnection::PRESYNAPTIC ) break;
        /* fall through */
      case TNeuronConnection::POSTSYNAPTIC:
        _sourcePathPost->active( state );
    }
  }

  void PSManager::dynamicVelocity( const float velocityModule )
  {
    _dynamicVelocityModule = velocityModule;

    auto setDynamicVelocity = [ velocityModule ]( MobilePolylineSource* e )
    { e->velocityModule( velocityModule ); };
    std::for_each( _dynamicSources.begin( ) , _dynamicSources.end( ) ,
                   setDynamicVelocity );
  }

  MobilePolylineSource* PSManager::getSpareMobileSouce( TNeuronConnection type )
  {
    MobilePolylineSource* result;

    if ( _availableDynamicSources.empty( ))
    {
      result = new MobilePolylineSource( _mobileSourcesEmissionRate ,
                                         vec3( 0 , 0 , 0 ));
    }
    else
    {
      result = *_availableDynamicSources.begin( );
      _availableDynamicSources.erase( result );
    }

    _dynamicSources.insert( result );

    const auto indices = _particleSystem->retrieveUnused(
      _particlesPerDynamicSource );
    _clusterDyn->particles( indices );

    _clusterDyn->setModel(
      ( type == PRESYNAPTIC ) ? _modelDynPre : _modelDynPost );
    _clusterDyn->setUpdater( _normalUpdater );

    result->delay( 0 );
    result->maxEmissionCycles( 0 );
    result->autoDeactivateWhenFinished( true );
    result->velocityModule( _dynamicVelocityModule );

    result->sampler( _sampler );

    _particleSystem->addSource( result , indices.indices( ));

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

    _particleSystem->detachSource( source_ );
  }
}
