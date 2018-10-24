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
  , _updaterSynapses( nullptr)
  , _updaterMapperSynapses( nullptr )
  , _normalUpdater( nullptr )
  , _binsNumber( 20 )
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

  void PSManager::init( PathFinder* pathFinder, unsigned int maxParticles )
  {
    _pathFinder = pathFinder;

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

    _modelSynMap = new prefr::Model( );
    _modelSynMap->velocity.Insert( 0.0f, 0.0f );

    _modelSynMap->size.Insert( 0.0f, 8.0f );
    _modelSynMap->color.Insert( 0.0f, glm::vec4( 1, 0, 0, 0.8 ));
    _modelSynMap->color.Insert( 1.0f, glm::vec4( 1, 1, 0, 0.8 ));


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
    _updaterMapperSynapses = new UpdaterMappedValue( );

    _particleSystem->addUpdater( _updaterSynapses );
    _particleSystem->addUpdater( _updaterMapperSynapses );

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

  void PSManager::_updateBoundingBox( const std::vector< vec3 > positions, bool clear_ )
  {

    Eigen::Array3f minimum;
    Eigen::Array3f maximum;

    if( clear_ )
    {
       minimum = Eigen::Array3f::Constant( std::numeric_limits< float >::max( ));
       maximum = Eigen::Array3f::Constant( std::numeric_limits< float >::min( ));
    }
    else
    {
      minimum = _boundingBox.minimum( );
      maximum = _boundingBox.maximum( );
    }

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
  }

  void PSManager::configureSynapses( const tsynapseVec& synapses,
                                     bool mapValues,
                                     TBrainSynapseAttribs attrib,
                                     const TNeuronConnection type )
  {
    if( !mapValues )
    {

      std::vector< vec3 > positionsPre;
      std::vector< vec3 > positionsPost;
      positionsPre.reserve( synapses.size( ));
      positionsPost.reserve( synapses.size( ));

      unsigned int counter = 0;
      for( auto syn : synapses )
      {
//        unsigned int postGid = syn->postSynapticNeuron( );

        auto morphoSyn = dynamic_cast< nsol::MorphologySynapsePtr >( syn );

        positionsPre.push_back( morphoSyn->preSynapticSurfacePosition( ));
        positionsPost.push_back( morphoSyn->postSynapticSurfacePosition( ));

        ++counter;
      }

      std::cout << " Loaded " << counter << " synapses." << std::endl;

      if( type == PRESYNAPTIC || type == ALL_CONNECTIONS )
        setupSynapses( positionsPre, PRESYNAPTIC );

      if( type == POSTSYNAPTIC || type == ALL_CONNECTIONS )
        setupSynapses( positionsPost, POSTSYNAPTIC );
    }
    else
    {
      if( type == PRESYNAPTIC || type == ALL_CONNECTIONS )
        mapSynapses( synapses, PRESYNAPTIC, attrib );

      if( type == POSTSYNAPTIC || type == ALL_CONNECTIONS )
        mapSynapses( synapses, POSTSYNAPTIC, attrib );
    }
  }

  void PSManager::mapSynapses( const tsynapseVec& synapses,
                               TNeuronConnection type,
                               TBrainSynapseAttribs attrib )
  {
    if( synapses.empty( ))
      return;

    clearSynapses( type );

//    _gidToParticleId.clear( );

    auto availableParticles =
           _particleSystem->retrieveUnused( synapses.size( ));

    tPosVec positions;
    std::vector< float > values;
    positions.reserve( synapses.size( ));
    values.reserve( synapses.size( ));

    auto source = ( type == syncopa::PRESYNAPTIC ? _sourceSynPre : _sourceSynPost );
    auto cluster = ( type == syncopa::PRESYNAPTIC ? _clusterSynPre : _clusterSynPost );
    auto model = ( type == syncopa::PRESYNAPTIC ? _modelSynMap : _modelSynMap ); //TODO

//    using tsyn = nsol::MorphologySynapse;


    auto& synapseInfoMap = _pathFinder->synapsesInfo( );

    float maxValue = 0.0f;
    float minValue = std::numeric_limits< float >::max( );

    for( auto synapse : synapses )
//    for( unsigned int i = 0; i < synapses.size( ); ++i )
    {
//      auto synapse = synapses[ i ];

      auto synapseInfo = synapseInfoMap.find( synapse );
      assert( synapseInfo != synapseInfoMap.end( ));
//      if( synapseInfo == synapseInfoMap.end( ))
//      {
//        std::cout << "ERROR: " << i << " synapse info " << synapse->gid( ) << " not found." << std::endl;
//        continue;
//      }

      positions.emplace_back( ( type == PRESYNAPTIC ) ?
          synapse->preSynapticSurfacePosition( ) :
          synapse->postSynapticSurfacePosition( ));

      float value = 0.0f;
      auto synapseAttribs = std::get< TBSI_ATTRIBUTES >( synapseInfo->second );

      switch( attrib )
      {
        case TBSA_SYNAPSE_DELAY:
          value = std::get< TBSA_SYNAPSE_DELAY >( synapseAttribs );
          break;
        case TBSA_SYNAPSE_CONDUCTANCE:
          value = std::get< TBSA_SYNAPSE_CONDUCTANCE >( synapseAttribs );
                    break;
        case TBSA_SYNAPSE_UTILIZATION:
          value = std::get< TBSA_SYNAPSE_UTILIZATION >( synapseAttribs );
          break;
        case TBSA_SYNAPSE_DEPRESSION:
          value = std::get< TBSA_SYNAPSE_DEPRESSION >( synapseAttribs );
          break;
        case TBSA_SYNAPSE_FACILITATION:
          value = std::get< TBSA_SYNAPSE_FACILITATION >( synapseAttribs );
          break;
        case TBSA_SYNAPSE_DECAY:
          value = std::get< TBSA_SYNAPSE_DECAY >( synapseAttribs );
          break;
        case TBSA_SYNAPSE_EFFICACY:
          value = std::get< TBSA_SYNAPSE_EFFICACY >( synapseAttribs );
          break;
        case TBSA_SYNAPSE_OTHER:
          value = ( unsigned int ) synapse->synapseType( );
          break;

      }

      maxValue = std::max( maxValue, value );
      minValue = std::min( minValue, value );

      values.emplace_back( value );

    }

    double invRange = 1.0 / ( maxValue - minValue );

//    _generateHistogram( values, minValue, maxValue );

    std::cout << "Norm values ";
    for( unsigned int i = 0; i < values.size( ); ++i )
    {
      float normalizedValue = ( values[ i ] - minValue ) * invRange;
      normalizedValue = std::min( std::max( 0.0f, normalizedValue ), 1.0f );
      auto particle = availableParticles.at( i );

      std::cout << " " << normalizedValue;

      particle.set_life( normalizedValue );

//      _gidToParticleId.insert( std::make_pair( synapses[ i ]->gid( ),
//                                               particle.id( )));
    }

    std::cout << std::endl;

    source->addPositions( availableParticles.indices( ), positions );

    cluster->particles( availableParticles );
    //    std::cout << "Cluster " << cluster->particles( ).size( ) << " " << availableParticles.size( ) << std::endl;

    cluster->setModel( model );
    cluster->setUpdater( _updaterMapperSynapses );
    //    cluster->setSource( source );
    _particleSystem->addSource( source, cluster->particles( ).indices( ) );

    //    std::cout << "Source " << source->particles( ).size( ) << " " << availableParticles.size( ) << std::endl;

    _updateBoundingBox( positions );
  }

  void PSManager::_generateHistogram( const std::vector< float >& values,
                                      float minValue, float maxValue )
  {
    assert( _binsNumber > 0 );

    _synapseAttribHistogram.clear( );
    _synapseAttribHistogram.resize( _binsNumber, 0 );

    _histoFunction.clear( );

    _histoFunction.insert( 0, QPoint( 0, 0 ));

    float invNorm = 1.0f / ( maxValue - minValue );

//    float step = 1.0f / ( _binsNumber - 1 );
//    float acc = 0.0f;

    for( auto value : values )
    {
      unsigned int pos = ( value * invNorm ) * ( _binsNumber - 1 );

      _synapseAttribHistogram[ pos ] += 1;
    }

    std::cout << "Bins: ";
    for( auto bin : _synapseAttribHistogram )
      std::cout << " " << bin;
    std::cout << std::endl;


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

//    std::cout << "Source " << source->particles( ).size( ) << " " << availableParticles.size( ) << std::endl;

    _updateBoundingBox( positions );

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
    {
      _colorSynPre = color;
      _modelSynPre->color.Insert( 0, eigenToGLM( color ));
    }
    else if( type == POSTSYNAPTIC )
    {
      _colorSynPost = color;
      _modelSynPost->color.Insert( 0, eigenToGLM( color ));
    }
    else
    {
      _colorSynPre = color;
      _colorSynPost = color;

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
    {
      _modelSynPre->size.Insert( 0, size );
    }
    else if( type == POSTSYNAPTIC )
    {
      _modelSynPost->size.Insert( 0, size );
    }
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
    {
      _colorPathPre = color;
      _modelPathPre->color.Insert( 0, eigenToGLM( _colorPathPre ));
    }
    else if( type == POSTSYNAPTIC )
    {
      _colorPathPost = color;
      _modelPathPost->color.Insert( 0, eigenToGLM( _colorPathPost ));
    }
    else
    {
      _colorPathPre = color;
      _colorPathPost = color;

      _modelPathPre->color.Insert( 0, eigenToGLM( color ));
      _modelPathPost->color.Insert( 0, eigenToGLM( color ));
    }
  }

  void PSManager::colorSynapseMap( const vec4& color, TNeuronConnection type )
  {
    if( type == PRESYNAPTIC )
    {
      _colorSynMapStart = color;
      _modelSynMap->color.Insert( 0, eigenToGLM( color ));
    }
    else
    {
      _colorSynMapEnd = color;
      _modelSynMap->color.Insert( 1.0, eigenToGLM( color ));
    }
  }

  void PSManager::colorSynapseMap( const tColorVec& colors )
  {
    _modelSynMap->color.Clear( );

    std::cout << "Color mapping: ";
    for( auto color : colors )
    {
      std::cout << " " << color.first;
      _modelSynMap->color.Insert( color.first, color.second );
    }

    std::cout << std::endl;
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
