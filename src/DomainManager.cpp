/*
 * @file  DomainManager.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "DomainManager.h"

namespace syncopa
{
  DomainManager::DomainManager( )
  : _dataset( nullptr )
  , _neuronScene( nullptr )
  , _psManager( nullptr )
  , _pathFinder( nullptr )
  , _dynPathManager( nullptr )
  , _nlrenderer( nullptr )
  , _particleSizeThreshold( 0.45 )
  , _colorSelectedPre( 0.5, 0.5, 1 )
  , _colorSelectedPost( 1, 0.5, 0 )
  , _colorRelated( 0.7, 0.5, 0 )
  , _colorContext( 0.3, 0.3, 0.3 )
  , _colorSynapsesPre( 1, 0, 0 )
  , _colorSynapsesPost( 1, 0, 1 )
  , _colorPathsPre( 0, 1, 0 )
  , _colorPathsPost( 0, 0, 1 )
  , _colorSynMapPre( 0.5, 0, 0.5   )
  , _colorSynMapPost( 1, 0, 0 )
  , _alphaSynapsesPre( 0.85f )
  , _alphaSynapsesPost( 0.85f )
  , _alphaPathsPre( 0.1f )
  , _alphaPathsPost( 0.1f )
  , _alphaSynapseMapPre( 0.20f )
  , _showSelectedPre( true )
  , _showSelectedPost( true )
  , _showRelated( true )
  , _showContext( true )
  , _showSynapsesPre( true )
  , _showSynapsesPost( true )
  , _showPathsPre( true )
  , _showPathsPost( true )
  , _mapSynapseValues( false )
  , _currentSynapseAttrib( TBSA_SYNAPSE_OTHER )
  , _maxValue( 0.0f )
  , _minValue( 0.0f )
  , _binsNumber( 20 )
  {
    _pathFinder = new syncopa::PathFinder( );
    _dynPathManager = new syncopa::DynamicPathManager( );

  }

  DomainManager::~DomainManager( )
  {
    if( _psManager )
      delete _psManager;

    if( _pathFinder )
      delete _pathFinder;
  }

  void DomainManager::loadBlueConfig( const std::string& blueConfigFilePath,
                                     const std::string& target )
  {

    if( _dataset )
      delete _dataset;

    _dataset = new nsol::DataSet( );

    std::cout << "Loading data hierarchy..." << std::endl;
    _dataset->loadBlueConfigHierarchy( blueConfigFilePath, target );

    std::cout << "Loading morphologies..." << std::endl;
    _dataset->loadAllMorphologies( );

    std::cout << "Loading connectivity..." << std::endl;
    _dataset->loadBlueConfigConnectivityWithMorphologies( );

    _pathFinder->dataset( _dataset, &_synapseFixInfo );
    _loadSynapseFixInfo( );

    _neuronScene = new syncopa::NeuronScene( _dataset );
    std::cout << "Generating meshes..." << std::endl;
    _neuronScene->generateMeshes( );

    _neuronScene->color( vec3( 1, 1, 1 ), PRESYNAPTIC );
    _neuronScene->color( vec3( 0, 1, 1 ), POSTSYNAPTIC );


    gidUSet gids;
    for( auto neuron : _dataset->neurons( ))
    {
      gids.insert( neuron.first );
    }

    _gidsAll = gids;

    createParticleSystem( );

    loadSynapses( _gidsAll );
    auto& synapses = getSynapses( );
    configureSynapses( synapses, _mapSynapseValues, _currentSynapseAttrib);

  }

  void DomainManager::clearSelection( void )
  {

  }

  void DomainManager::clear( void )
  {
    _neuronsSelectedPre = syncopa::TRenderMorpho( );
    _neuronsSelectedPost = syncopa::TRenderMorpho( );
    _neuronsRelated = _neuronScene->getRender( _gidsAll );
    _neuronsContext = syncopa::TRenderMorpho( );

    _synapses.clear( );

    _dynPathManager->clear( );
    _psManager->clear( );

  }

  void DomainManager::_loadSynapseFixInfo( void )
  {
    if( _dataset )
    {
      auto blueConfig = _dataset->blueConfig( );

      brain::Circuit brainCircuit( *blueConfig );
      brion::GIDSet gidSetBrain = brainCircuit.getGIDs( _dataset->blueConfigTarget( ));

      const brain::Synapses& brainSynapses =
          brainCircuit.getAfferentSynapses( gidSetBrain,
                                            brain::SynapsePrefetch::attributes );

      auto synapses = _dataset->circuit( ).synapses( );
      for( unsigned int i = 0; i < synapses.size( ); ++i )
      {
        auto synapse = dynamic_cast< nsolMSynapse_ptr >( synapses[ i ]);
        auto brainSynapse = brainSynapses[ synapse->gid( ) - 1 ];

        tBrainSynapse infoPre =
            std::make_tuple( brainSynapse.getPresynapticSectionID( ),
                             brainSynapse.getPresynapticSegmentID( ),
                             brainSynapse.getPresynapticDistance( ));

        tBrainSynapse infoPost =
            std::make_tuple( brainSynapse.getPostsynapticSectionID( ),
                             brainSynapse.getPostsynapticSegmentID( ),
                             brainSynapse.getPostsynapticDistance( ));

        tBrainSynapseAttribs attribs =
            std::make_tuple( brainSynapse.getDelay( ),
                             brainSynapse.getConductance( ),
                             brainSynapse.getUtilization( ),
                             brainSynapse.getDepression( ),
                             brainSynapse.getFacilitation( ),
                             brainSynapse.getDecay( ),
                             brainSynapse.getEfficacy( ));

        _synapseFixInfo.insert(
            std::make_pair( synapse, std::make_tuple( attribs, infoPre, infoPost )));

      }

      std::cout << "Loaded BRAIN synapse info of " << _synapseFixInfo.size( ) << std::endl;
      //      calculateFixedSynapseSections( );
    }
  }

  const TSynapseInfo& DomainManager::synapseFixInfo( void ) const
  {
    return _synapseFixInfo;
  }


  const tsynapseVec& DomainManager::getSynapses( void ) const
  {
    return _synapses;
  }

  void DomainManager::loadSynapses( const gidUSet& gids )
  {
    if( gids.empty( ))
      return;

    std::vector< nsolMSynapse_ptr > result;

    std::set< unsigned int > gidset( gids.begin( ), gids.end( ));

    auto& circuit = _dataset->circuit( );
    auto synapseSet = circuit.synapses( gidset,
                                        nsol::Circuit::PRESYNAPTICCONNECTIONS );

    result.reserve( synapseSet.size( ));

    for( auto syn : synapseSet )
    {

      auto msyn = dynamic_cast< nsolMSynapse_ptr >( syn );
      if( !msyn)
      {
        std::cout << "EMPTY SYNAPSE " << std::endl;
        continue;
      }

      auto sectionPre = msyn->preSynapticSection( );
      auto sectionPost = msyn->postSynapticSection( );

      // axo-somatic
      if( !sectionPre || ( !sectionPost &&
          msyn->synapseType( ) != nsol::MorphologySynapse::AXOSOMATIC ))
        continue;


      auto synInfo = _synapseFixInfo.find( msyn );
      if( synInfo == _synapseFixInfo.end( ))
        continue;

      auto infoPre = std::get< TBSI_PRESYNAPTIC >( synInfo->second );
      auto infoPost = std::get< TBSI_POSTSYNAPTIC >( synInfo->second );

      if( std::get< TBS_SEGMENT_INDEX >( infoPre ) >=
          sectionPre->nodes( ).size( ) - 1 )
      {
        std::cout << "Discarding synapse with pre segment "
                  << std::get< TBS_SEGMENT_INDEX >( infoPre )
                  << " segments " << ( sectionPre->nodes( ).size( ) - 1 )
                  << std::endl;
        continue;
      }

      if( sectionPost && std::get< TBS_SEGMENT_INDEX >( infoPost ) >=
          sectionPost->nodes( ).size( ) - 1 )
      {
        std::cout << "Discarding synapse with post segment "
                  << std::get< TBS_SEGMENT_INDEX >( infoPost )
                  << " segments " << ( sectionPost->nodes( ).size( ) - 1 )
                  << std::endl;
        continue;
      }

      result.push_back( msyn );
    }

    result.shrink_to_fit( );

    _synapses = result;

    std::cout << "Loaded " << result.size( ) << " synapses." << std::endl;

  }

  void DomainManager::_loadSynapses( unsigned int presynapticGID,
                                  const gidUSet& postsynapticGIDs )
  {
    if( presynapticGID == *_gidsSelectedPre.begin( ) && postsynapticGIDs.empty( ))
      return;

    std::vector< nsolMSynapse_ptr > result;

    std::set< unsigned int > gidsPre = { presynapticGID };

    auto& circuit = _dataset->circuit( );
    auto synapseSet = circuit.synapses( gidsPre,
                                        nsol::Circuit::PRESYNAPTICCONNECTIONS );


    result.reserve( synapseSet.size( ));

    for( auto syn : synapseSet )
    {
      if( !postsynapticGIDs.empty( ) &&
          postsynapticGIDs.find( syn->postSynapticNeuron( )) == postsynapticGIDs.end( ))
        continue;


      auto msyn = dynamic_cast< nsolMSynapse_ptr >( syn );
      if( !msyn)
      {
        std::cout << "EMPTY SYNAPSE " << std::endl;
        continue;
      }

      auto sectionPre = msyn->preSynapticSection( );
      auto sectionPost = msyn->postSynapticSection( );

      // axo-somatic
      if( !sectionPre || ( !sectionPost &&
          msyn->synapseType( ) != nsol::MorphologySynapse::AXOSOMATIC ))
        continue;


      auto synInfo = _synapseFixInfo.find( msyn );
      if( synInfo == _synapseFixInfo.end( ))
        continue;

      auto infoPre = std::get< TBSI_PRESYNAPTIC >( synInfo->second );
      auto infoPost = std::get< TBSI_POSTSYNAPTIC >( synInfo->second );

      if( std::get< TBS_SEGMENT_INDEX >( infoPre ) >=
          sectionPre->nodes( ).size( ) - 1 )
      {
        std::cout << "Discarding synapse with pre segment "
                  << std::get< TBS_SEGMENT_INDEX >( infoPre )
                  << " segments " << ( sectionPre->nodes( ).size( ) - 1 )
                  << std::endl;
        continue;
      }

      if( sectionPost && std::get< TBS_SEGMENT_INDEX >( infoPost ) >=
          sectionPost->nodes( ).size( ) - 1 )
      {
        std::cout << "Discarding synapse with post segment "
                  << std::get< TBS_SEGMENT_INDEX >( infoPost )
                  << " segments " << ( sectionPost->nodes( ).size( ) - 1 )
                  << std::endl;
        continue;
      }

      result.push_back( msyn );
    }

    result.shrink_to_fit( );

    _synapses = result;

    std::cout << "Loaded " << _synapses.size( ) << " synapses." << std::endl;

  }

  void DomainManager::renderMorphologies( void )
  {
    if( !_nlrenderer )
      return;

    // Render selected neurons with full morphology
    if( _showSelectedPre )
      _nlrenderer->render( std::get< syncopa::MESH >( _neuronsSelectedPre ),
                           std::get< syncopa::MATRIX >( _neuronsSelectedPre ),
                           std::get< syncopa::COLOR >( _neuronsSelectedPre ));

    if( _showSelectedPost )
      _nlrenderer->render( std::get< syncopa::MESH >( _neuronsSelectedPost ),
                           std::get< syncopa::MATRIX >( _neuronsSelectedPost ),
                           std::get< syncopa::COLOR >( _neuronsSelectedPost ));

    if( _showRelated )
      _nlrenderer->render( std::get< syncopa::MESH >( _neuronsRelated ),
                           std::get< syncopa::MATRIX >( _neuronsRelated ),
                           std::get< syncopa::COLOR >( _neuronsRelated ), true, false );

    if( _showContext )
      _nlrenderer->render( std::get< syncopa::MESH >( _neuronsContext ),
                           std::get< syncopa::MATRIX >( _neuronsContext ),
                           std::get< syncopa::COLOR >( _neuronsContext ), true, false );

  }

  void DomainManager::processDynamicEvents( void )
  {

    //  std::cout << "***** Processing finished paths" << std::endl;
      _dynPathManager->processFinishedPaths( );
    //  std::cout << "***** Processing bifurcations" << std::endl;
      _dynPathManager->processPendingSections( );
    //  std::cout << "***** Processing synapses" << std::endl;
      _dynPathManager->processPendingSynapses( );

  }

  void DomainManager::configureSynapses( const tsynapseVec& synapses,
                                     bool mapValues,
                                     TBrainSynapseAttribs attrib,
                                     const TNeuronConnection type )
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


    if( !mapValues )
    {
      if( type == PRESYNAPTIC || type == ALL_CONNECTIONS )
        _psManager->setupSynapses( positionsPre, PRESYNAPTIC );

      if( type == POSTSYNAPTIC || type == ALL_CONNECTIONS )
        _psManager->setupSynapses( positionsPost, POSTSYNAPTIC );
    }
    else
    {
      _currentSynapseAttrib = attrib;

      _calculateSynapsesAttribValues( synapses );

      if( type == PRESYNAPTIC || type == ALL_CONNECTIONS )
        _psManager->mapSynapses( positionsPre, _currentAttribNormValues,
                                 PRESYNAPTIC, attrib );

      if( type == POSTSYNAPTIC || type == ALL_CONNECTIONS )
        _psManager->mapSynapses( positionsPost, _currentAttribNormValues,
                                 POSTSYNAPTIC, attrib );
    }

    if( type == PRESYNAPTIC || type == ALL_CONNECTIONS )
      _psManager->updateBoundingBox( positionsPre, true );
    if( type == POSTSYNAPTIC || type == ALL_CONNECTIONS )
      _psManager->updateBoundingBox( positionsPost, false );
  }

  void DomainManager::selectPresynapticNeuron( unsigned int gid )
  {
    _gidsSelectedPre = { gid };

    _gidsRelated = _pathFinder->connectedTo( gid );

    _gidsSelectedPost.clear( );

    _gidsOther = _gidsAll;

    for( auto gidToDelete : _gidsSelectedPre )
      _gidsOther.erase( gidToDelete );

    for( auto gidToDelete : _gidsSelectedPost )
      _gidsOther.erase( gidToDelete );

    for( auto gidToDelete : _gidsRelated )
      _gidsOther.erase( gidToDelete );


    _dynPathManager->clear( );
    _loadSynapses( gid, _gidsSelectedPost );

    _pathFinder->configure( gid, _gidsSelectedPost, _synapses );

  //  _gidsOther.erase( _gidsSelectedPre.begin( ), _gidsSelectedPre.end( ));
  //  _gidsOther.erase( _gidsSelectedPost.begin( ), _gidsSelectedPost.end( ));
  //  _gidsOther.erase( _gidsRelated.begin( ), _gidsRelated.end( ));

    setupNeuronMorphologies( );
  //  std::vector< unsigned int > gidsvPre = { gid };
  //  std::vector< unsigned int > gidsvPost( gidsPost.begin( ), gidsPost.end( ));

    setupSynapses( );
    setupPaths( _gidsSelectedPre, _gidsRelated );

    updateSynapsesVisibility( );
    updatePathsVisibility( );
  //  setupDynamicPath( gid );

    updateBoundingBox( );
  }

  void DomainManager::selectPostsynapticNeuron( const std::vector< unsigned int >& gidsv )
  {
    _gidsSelectedPost = gidUSet( gidsv.begin( ), gidsv.end( ));

    unsigned int gidPre = *_gidsSelectedPre.begin( );
    _gidsRelated = _pathFinder->connectedTo( gidPre );

    for( auto gid : _gidsSelectedPost )
      _gidsRelated.erase( gid );
  //  _gidsRelated.erase( _gidsSelectedPost.begin( ), _gidsSelectedPost.end( ));

    _gidsOther.clear( );

    _dynPathManager->clear( );

    _loadSynapses( gidPre, _gidsSelectedPost );
    _pathFinder->configure( *_gidsSelectedPre.begin( ), _gidsSelectedPost, _synapses );

    setupNeuronMorphologies( );

  //  std::set< unsigned int > gidsPre = { _gidsSelectedPre };
    setupSynapses( );
    setupPaths( _gidsSelectedPre, _gidsSelectedPost );

    updateSynapsesVisibility( );
    updatePathsVisibility( );
  //  setupDynamicPath( *_gidsSelectedPre.begin( ) );

    updateBoundingBox( );
  }


  void DomainManager::createParticleSystem( void )
  {

    _psManager = new syncopa::PSManager( );
    _psManager->init( _pathFinder, 500000 );

    _psManager->colorSynapses( vec4( _colorSynapsesPre.x( ), _colorSynapsesPre.y( ),
                                     _colorSynapsesPre.z( ), _alphaSynapsesPre ),
                               PRESYNAPTIC );

    _psManager->colorSynapses( vec4( _colorSynapsesPost.x( ), _colorSynapsesPost.y( ),
                                     _colorSynapsesPost.z( ), _alphaSynapsesPost ),
                               POSTSYNAPTIC );

    _psManager->colorPaths( vec4( _colorPathsPre.x( ), _colorPathsPre.y( ),
                                  _colorPathsPre.z( ), _alphaPathsPre ),
                            PRESYNAPTIC );

    _psManager->colorPaths( vec4( _colorPathsPost.x( ), _colorPathsPost.y( ),
                                  _colorPathsPost.z( ), _alphaPathsPost ),
                            POSTSYNAPTIC );

    _psManager->colorSynapseMap( vec4( _colorSynMapPre.x( ), _colorSynMapPre.y( ),
                                  _colorSynMapPre.z( ), _alphaSynapseMapPre ),
                            PRESYNAPTIC );

    _psManager->colorSynapseMap( vec4( _colorSynMapPost.x( ), _colorSynMapPost.y( ),
                                  _colorSynMapPost.z( ), _alphaSynapseMapPre ),
                            POSTSYNAPTIC );



    _psManager->sizeSynapses( 8.0, PRESYNAPTIC );
    _psManager->sizeSynapses( 8.0, POSTSYNAPTIC );

    _psManager->sizePaths( 3.0, PRESYNAPTIC );
    _psManager->sizePaths( 3.0, POSTSYNAPTIC );

    _dynPathManager->init( _pathFinder, _psManager );
  }

  void DomainManager::setupSynapses( void )
  {
    auto& synapses = getSynapses( );
    configureSynapses( synapses, _mapSynapseValues, _currentSynapseAttrib);
  }

  void DomainManager::setupPaths( const gidUSet& gidsPre,
                                 const gidUSet& gidsPost )
  {

    float pointSize = _psManager->sizePaths( ) * _particleSizeThreshold * 0.5;

    // TODO FIX MULTIPLE PRESYNAPTIC SELECTION
    unsigned int gidPre = *gidsPre.begin( );
    auto points =
        _pathFinder->getAllPathsPoints( gidPre, gidsPost, getSynapses( ),
                                        pointSize, PRESYNAPTIC );

    _psManager->setupPath( points, PRESYNAPTIC );

    std::cout << "GENERATING POSTSYNAPTIC PATHS" << std::endl;
    points = _pathFinder->getAllPathsPoints( gidPre, gidsPost, getSynapses( ),
                                             pointSize, POSTSYNAPTIC );

    _psManager->setupPath( points, POSTSYNAPTIC );
  }

  const nsol::DataSet* DomainManager::dataSet( void ) const
  {
    return _dataset;
  }

  void DomainManager::nlRenderer( nlrender::Renderer* renderer )
  {
    _nlrenderer = renderer;
  }

  void setColor( syncopa::TRenderMorpho& renderConfig, const vec3& color )
  {
    for( auto& c : std::get< syncopa::COLOR >( renderConfig ))
      c = color;
  }

  void DomainManager::setupNeuronMorphologies( void )
  {

    // Selected
    _neuronsSelectedPre = _neuronScene->getRender( _gidsSelectedPre );
    setColor( _neuronsSelectedPre, _colorSelectedPre );

    _neuronsSelectedPost = _neuronScene->getRender( _gidsSelectedPost );
    setColor( _neuronsSelectedPost, _colorSelectedPost );

    // Related
    _neuronsRelated = _neuronScene->getRender( _gidsRelated );
    setColor( _neuronsRelated, _colorRelated );

    // Context
    _neuronsContext = _neuronScene->getRender( _gidsOther );
    setColor(   _neuronsContext, _colorContext );

  }

  void DomainManager::colorSelectedPre( const syncopa::vec3& color )
  {
    _colorSelectedPre = color;

    setColor( _neuronsSelectedPre, _colorSelectedPre );
  }

  void DomainManager::colorSelectedPost( const syncopa::vec3& color )
  {
    _colorSelectedPost = color;

    setColor( _neuronsSelectedPost, _colorSelectedPost );
  }

  void DomainManager::colorRelated( const syncopa::vec3& color )
  {
    _colorRelated = color;

    setColor( _neuronsRelated, _colorRelated );
  }

  void DomainManager::colorContext( const syncopa::vec3& color )
  {
    _colorContext = color;

    setColor( _neuronsContext, _colorContext );
  }

  void DomainManager::colorSynapsesPre( const syncopa::vec3& color )
  {
    _colorSynapsesPre = color;

    syncopa::vec4 composedColor( color.x( ), color.y( ), color.z( ), _alphaSynapsesPre );
    _psManager->colorSynapses( composedColor, syncopa::PRESYNAPTIC );
  }

  void DomainManager::colorSynapsesPost( const syncopa::vec3& color )
  {
    _colorSynapsesPost = color;

    syncopa::vec4 composedColor( color.x( ), color.y( ), color.z( ), _alphaSynapsesPre );
    _psManager->colorSynapses( composedColor, syncopa::POSTSYNAPTIC );
  }

  void DomainManager::colorPathsPre( const syncopa::vec3& color )
  {
    _colorPathsPre = color;

    syncopa::vec4 composedColor( color.x( ), color.y( ), color.z( ), _alphaPathsPre );
    _psManager->colorPaths( composedColor, syncopa::PRESYNAPTIC );
  }

  void DomainManager::colorPathsPost( const syncopa::vec3& color )
  {
    _colorPathsPost = color;

    syncopa::vec4 composedColor( color.x( ), color.y( ), color.z( ), _alphaPathsPost );
    _psManager->colorPaths( composedColor, syncopa::POSTSYNAPTIC );
  }

  void DomainManager::colorSynapseMapPre( const syncopa::vec3& color )
  {
    _colorSynMapPre = color;

    syncopa::vec4 composedColor( color.x( ), color.y( ), color.z( ), _alphaSynapseMapPre );
    _psManager->colorSynapseMap( composedColor, syncopa::PRESYNAPTIC );
  }

  void DomainManager::colorSynapseMapPost( const syncopa::vec3& color )
  {
    _colorSynMapPost = color;

    syncopa::vec4 composedColor( color.x( ), color.y( ), color.z( ), _alphaSynapseMapPre );
    _psManager->colorSynapseMap( composedColor, syncopa::POSTSYNAPTIC );
  }


  const syncopa::vec3& DomainManager::colorSelectedPre( void ) const
  {
    return _colorSelectedPre;
  }

  const syncopa::vec3& DomainManager::colorSelectedPost( void ) const
  {
    return _colorSelectedPost;
  }

  const syncopa::vec3& DomainManager::colorRelated( void ) const
  {
    return _colorRelated;
  }

  const syncopa::vec3& DomainManager::colorContext( void ) const
  {
    return _colorContext;
  }

  const syncopa::vec3& DomainManager::colorSynapsesPre( void ) const
  {
  //  syncopa::vec4 result = _psManager->colorSynapses( syncopa::PRESYNAPTIC );
  //
  //  return syncopa::vec3( result.block< 3, 1 >( 0, 0 ));
    return _colorSynapsesPre;
  }

  const syncopa::vec3& DomainManager::colorSynapsesPost( void ) const
  {
  //  syncopa::vec4 result = _psManager->colorSynapses( syncopa::POSTSYNAPTIC );
  //
  //  return syncopa::vec3( result.block< 3, 1 >( 0, 0 ));
    return _colorSynapsesPost;
  }

  const syncopa::vec3& DomainManager::colorPathsPre( void ) const
  {
    return _colorPathsPre;
  }

  const syncopa::vec3& DomainManager::colorPathsPost( void ) const
  {
    return _colorPathsPost;
  }

  const syncopa::vec3& DomainManager::colorSynapseMapPre( void ) const
  {
    return _colorSynMapPre;
  }


  void DomainManager::colorSynapseMap( const tQColorVec& qcolors )
  {
    _colorSynMap = qcolors;

    tColorVec colors;
    colors.reserve( qcolors.size( ));

    for( auto color : qcolors )
    {
      glm::vec4 composedColor = qtToGLM( color.second );
      composedColor.w = _alphaSynapseMapPre;

      colors.emplace_back( std::make_pair( color.first, composedColor ));
    }

    _psManager->colorSynapseMap( colors );
  }

  tQColorVec DomainManager::colorSynapseMap( void ) const
  {
    return _colorSynMap;
  }

  float DomainManager::particleSizeThreshold( void )
  {
    return _particleSizeThreshold;
  }

  //bool DomainManager::showDialog( QColor& current, const std::string& message )
  //{
  //  QColor result = QColorDialog::getColor( current, this,
  //                                          QString( message ),
  //                                          QColorDialog::DontUseNativeDialog);
  //
  //  if( result.isValid( ))
  //  {
  //    current = result;
  //    return true;
  //  }
  //  else
  //    return false;
  //
  //}

  void DomainManager::updatePathsVisibility( void )
  {
    _psManager->showPaths( _showPathsPre, PRESYNAPTIC );
    _psManager->showPaths( _showPathsPost, POSTSYNAPTIC );
  }

  void DomainManager::updateSynapsesVisibility( void )
  {
    _psManager->showSynapses( _showSynapsesPre, PRESYNAPTIC );
    _psManager->showSynapses( _showSynapsesPost, POSTSYNAPTIC );
  }

  void DomainManager::showSelectedPre( int state )
  {
    _showSelectedPre = state;
  }

  void DomainManager::showSelectedPost( int state )
  {
    _showSelectedPost = state;
  }

  void DomainManager::showRelated( int state )
  {
    _showRelated = state;
  }

  void DomainManager::showContext( int state )
  {
    _showContext = state;
  }

  void DomainManager::showSynapsesPre( int state )
  {
    _showSynapsesPre = state;
    _psManager->showSynapses( state, syncopa::PRESYNAPTIC );
  }
  void DomainManager::showSynapsesPost( int state )
  {
    _showSynapsesPost = state;
    _psManager->showSynapses( state, syncopa::POSTSYNAPTIC );
  }

  void DomainManager::showPathsPre( int state )
  {
    _showPathsPre = state;
    _psManager->showPaths( state, syncopa::PRESYNAPTIC );
  }

  void DomainManager::showPathsPost( int state )
  {
    _showPathsPost = state;
    _psManager->showPaths( state, syncopa::POSTSYNAPTIC );
  }

  void DomainManager::startDynamic( void )
  {
    stopDynamic( );
    _dynPathManager->createRootSources( );
  }


  void DomainManager::stopDynamic( void )
  {
    _dynPathManager->clear( );
  }

  void DomainManager::setSynapseMappingState( bool state )
  {
    _mapSynapseValues = state;
    configureSynapses( getSynapses( ), state, _currentSynapseAttrib );
  }

  void DomainManager::setSynapseMapping( int attrib )
  {
    if( attrib < 0 || ( attrib > (int) TBSA_SYNAPSE_OTHER ))
      return;

    std::cout << "Synapse mapping changed to attribute " << attrib << std::endl;

    _currentSynapseAttrib = ( TBrainSynapseAttribs ) attrib;
    configureSynapses( getSynapses( ), true, _currentSynapseAttrib );
  }


  void DomainManager::_calculateSynapsesAttribValues( const tsynapseVec& synapses )
  {
    auto& synapseInfoMap = _synapseFixInfo;

    std::vector< float > values;
    values.reserve( synapses.size( ));

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


      float value = 0.0f;
      auto synapseAttribs = std::get< TBSI_ATTRIBUTES >( synapseInfo->second );

      switch( _currentSynapseAttrib )
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

    _currentAttribValues = values;

    _currentAttribNormValues.clear( );
    _currentAttribNormValues.resize( values.size( ), 0.0f );

    _minValue = minValue;
    _maxValue = maxValue;

    double invRange = 1.0 / ( maxValue - minValue );

    _generateHistogram( values, minValue, maxValue );

    for( unsigned int i = 0; i < values.size( ); ++i )
    {
      float normalizedValue = ( values[ i ] - minValue ) * invRange;
      normalizedValue = std::min( std::max( 0.0f, normalizedValue ), 1.0f );
//      _gidToParticleId.insert( std::make_pair( synapses[ i ]->gid( ),
//                                               particle.id( )));
      _currentAttribNormValues[ i ] = normalizedValue;
    }
  }


  void DomainManager::_generateHistogram( const std::vector< float >& values,
                                          float minValue, float maxValue )
  {
    assert( _binsNumber > 0 );

    std::cout << "Total values: " << values.size( ) << std::endl;

    _synapseAttribHistogram.clear( );
    if( _synapseAttribHistogram.size( ) != _binsNumber )
      _synapseAttribHistogram.resize( _binsNumber, 0 );

    _histoFunction.clear( );

    float invNormValues = 1.0f / ( maxValue - minValue );

    unsigned int maxBin = 0;
    unsigned int minBin = std::numeric_limits< unsigned int >::max( );

    for( auto value : values )
    {
      unsigned int pos = (( value - minValue ) * invNormValues ) * ( _binsNumber - 1 );

      unsigned int& bin = _synapseAttribHistogram[ pos ];
      bin += 1;
    }

    std::cout << "Bins: ";
    for( auto bin : _synapseAttribHistogram )
    {
      std::cout << " " << bin;

      if( bin > maxBin )
        maxBin = bin;

      if( bin < minBin )
        minBin = bin;

    }
    std::cout << std::endl;

    std::cout << "Max bin " << maxBin << " min bin " << minBin << std::endl;

    float step = 1.0f / ( _binsNumber - 1 );
    float acc = 0.0f;

    double invNormHistogram = 1.0 / ( maxBin - minBin );

    std::cout << "Histogram: ";
    for( auto bin : _synapseAttribHistogram )
    {
      double normalizedY = ( bin - minBin ) * invNormHistogram;
      double normalizedX = acc;

      _histoFunction.append( QPointF( normalizedX, normalizedY ));

      std::cout << " (" << normalizedX << ", " << normalizedY << ")";

      acc += step;
    }
    std::cout << std::endl;

  }

  void DomainManager::updateBoundingBox( void )
  {
    _boundingBox = _psManager->boundingBox( );
  }


  nlgeometry::AxisAlignedBoundingBox DomainManager::boundingBox( void ) const
  {
    std::cout << "Bounding box Max " << _boundingBox.maximum( ) << std::endl;
    std::cout << "Bounding box Min " << _boundingBox.minimum( ) << std::endl;
    return _boundingBox;
  }

  const QPolygonF& DomainManager::getSynapseMappingPlot( void ) const
  {
    return _histoFunction;
  }

  prefr::ParticleSystem* DomainManager::particleSystem( void ) const
  {
    return _psManager->particleSystem( );
  }

}

