/*
 * @file  OpenGLWidget.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "OpenGLWidget.h"
#include <QOpenGLContext>
#include <QMouseEvent>
#include <QColorDialog>
#include <QShortcut>
#include <QDebug>

#include <string>
#include <iostream>
#include <glm/glm.hpp>

#include <map>
#include <utility>

#include <plab/reto/RetoCamera.h>
#include <QOpenGLDebugLogger>

#include "particlelab/ParticleLabShaders.h"

#include "MainWindow.h"

using namespace syncopa;

constexpr float CAMERA_ANIMATION_DURATION = 0.75; /** camera animation duration in seconds. */

OpenGLWidget::OpenGLWidget(
  QWidget* parent ,
  Qt::WindowFlags windowFlags
                          )
  : QOpenGLWidget( parent , windowFlags )
  , _fpsLabel( this )
  , _showFps( false )
  , _camera( std::make_shared< plab::RetoCamera >( ))
  , _frameCount( 0 )
  , _deltaTime( 0.0f )
  , _mouseX( 0 )
  , _mouseY( 0 )
  , _rotation( false )
  , _translation( false )
  , _idleUpdate( false )
  , _paint( true )
  , _currentClearColor( 20 , 20 , 20 , 0 )
  , _nlrenderer( nullptr )
  , _dataset( nullptr )
  , _particleManager( )
  , _pathFinder( )
  , _neuronScene( nullptr )
  , _domainManager( nullptr )
  , _mode( UNDEFINED )
  , _particleSizeThreshold( 0.45 )
  , _elapsedTimeRenderAcc( 0.0f )
  , _alphaSynapsesMap( 0.55 )
  , _dynamicActive( false )
  , _dynamicMovement( true )
  , _oglFunctions( nullptr )
  , _screenPlaneShader( nullptr )
  , _quadVAO( 0 )
  , _weightFrameBuffer( 0 )
  , _accumulationTexture( 0 )
  , _revealTexture( 0 )
  , _currentWidth( 0 )
  , _currentHeight( 0 )
  , _mapSynapseValues( false )
  , _currentSynapseAttrib( TBSA_SYNAPSE_DELAY )
{
  _camera->camera( )->farPlane( 5000 );
  _animation = new reto::CameraAnimation( );
  _domainManager = new syncopa::DomainManager( );

  _lastCameraPosition = glm::vec3( 0 , 0 , 0 );

  _fpsLabel.setStyleSheet(
    "QLabel { background-color : #333;"
    "color : white;"
    "padding: 3px;"
    "margin: 10px;"
    " border-radius: 10px;}" );

  // This is needed to get key events
  this->setFocusPolicy( Qt::WheelFocus );

  _maxFPS = 200.0f;
  _renderPeriod = 1.0f / _maxFPS;
  _renderPeriodMicroseconds = _renderPeriod * 1000000;

  _renderSpeed = 1.f;

  new QShortcut( QKeySequence( Qt::Key_Tab ) , this ,
                 SLOT( toggleDynamicMovement( )) );
}

OpenGLWidget::~OpenGLWidget( void )
{
}

void OpenGLWidget::initializeGL( void )
{
  initializeOpenGLFunctions( );

  const GLubyte* vendor = glGetString( GL_VENDOR );
  const GLubyte* rendr = glGetString( GL_RENDERER );
  const GLubyte* version = glGetString( GL_VERSION );
  const GLubyte* shadingVer = glGetString( GL_SHADING_LANGUAGE_VERSION );

  std::cout << "GL Hardware: " << vendor << " (" << rendr << ")" << std::endl;
  std::cout << "GL Version: " << version << " (shading ver. " << shadingVer
            << ")" << std::endl;

  auto* logger = new QOpenGLDebugLogger( this );
  logger->initialize( );

  connect(
    logger , &QOpenGLDebugLogger::messageLogged ,
    [ ]( const QOpenGLDebugMessage& message )
    {
      if ( message.severity( ) <= QOpenGLDebugMessage::MediumSeverity )
        qDebug( ) << message;
    }
  );

  logger->startLogging( );

  glEnable( GL_DEPTH_TEST );
  glClearColor( static_cast<float>( _currentClearColor.red( )) / 255.0f ,
                static_cast<float>( _currentClearColor.green( )) / 255.0f ,
                static_cast<float>( _currentClearColor.blue( )) / 255.0f ,
                static_cast<float>( _currentClearColor.alpha( )) / 255.0f );
  glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
  glEnable( GL_CULL_FACE );

  _then = _lastFrame = std::chrono::system_clock::now( );

  QOpenGLWidget::initializeGL( );

  nlrender::Config::init( );
  _nlrenderer = new nlrender::Renderer( );
  _nlrenderer->tessCriteria( nlrender::Renderer::LINEAR );
  _nlrenderer->lod( ) = 4;
  _nlrenderer->maximumDistance( ) = 10;

  _currentWidth = width( );
  _currentHeight = height( );

  initRenderToTexture( );

  _particleManager.init( _camera );
}

void ExpandBoundingBox( glm::vec3& minBounds ,
                        glm::vec3& maxBounds ,
                        const glm::vec3& position )
{
  for ( unsigned int i = 0; i < 3; ++i )
  {
    minBounds[ i ] = std::min( minBounds[ i ] , position[ i ] );
    maxBounds[ i ] = std::max( maxBounds[ i ] , position[ i ] );
  }
}

void OpenGLWidget::loadBlueConfig( const std::string& blueConfigFilePath ,
                                   const std::string& target )
{
  delete _dataset;

  _dataset = new nsol::DataSet( );

  emit progress( tr( "Loading data hierarchy" ) , 0 );
  _dataset->loadBlueConfigHierarchy<
    nsol::NodeCached ,
    nsol::NeuronMorphologySectionCachedStats ,
    nsol::DendriteCachedStats ,
    nsol::AxonCachedStats ,
    nsol::SomaStats ,
    nsol::NeuronMorphologyCachedStats ,
    nsol::Neuron
  >( blueConfigFilePath , target );

  emit progress( tr( "Loading morphologies" ) , 33 );
  _dataset->loadAllMorphologies<
    nsol::NodeCached ,
    nsol::NeuronMorphologySectionCachedStats ,
    nsol::DendriteCachedStats ,
    nsol::AxonCachedStats ,
    nsol::SomaStats ,
    nsol::NeuronMorphologyCachedStats ,
    nsol::Neuron
  >( );

  emit progress( tr( "Loading connectivity" ) , 66 );
  _dataset->loadBlueConfigConnectivityWithMorphologies<
    nsol::NodeCached ,
    nsol::NeuronMorphologySectionCachedStats ,
    nsol::DendriteCachedStats ,
    nsol::AxonCachedStats ,
    nsol::SomaStats ,
    nsol::NeuronMorphologyCachedStats ,
    nsol::Neuron
  >( );

  _domainManager->dataset( _dataset );

  emit progress( tr( "Configuring path finder" ) , 100 );
  _pathFinder.dataset( _dataset , &_domainManager->synapsesInfo( ));

  _neuronScene = new syncopa::NeuronScene( _dataset );

  connect(
    _neuronScene , SIGNAL( progress(
                             const QString & , const unsigned int)) ,
    this , SIGNAL( progress(
                     const QString & , const unsigned int))
  );

  _neuronScene->generateMeshes( );

  emit progress( QString( ) , 100 );
}

void OpenGLWidget::createParticleSystem( void )
{
  makeCurrent( );
}

void OpenGLWidget::setupSynapses( void )
{
  if ( !_mapSynapseValues )
    _particleManager.setSynapses( _domainManager->getSynapses( ));
  else
  {
    _particleManager.setMappedSynapses(
      _domainManager->getFilteredSynapses( ) ,
      _domainManager->getFilteredNormValues( ));
  }
}

void OpenGLWidget::setupPaths( void )
{
  if ( _mode != PATHS )
  {
    _particleManager.clearPaths( );
    return;
  }
}

void OpenGLWidget::home( bool animate )
{
  const float FOV = sin( _camera->camera( )->fieldOfView( ));
  const auto rotation = Eigen::Vector3f{ 0.0f , 0.0f , 0.0f };
  const auto bb = _mode == PATHS
                  ? _particleManager.getParticlesBoundingBox( )
                  : _particleManager.getSynapseBoundingBox( );
  const auto position = bb.center( );
  const auto radius = bb.radius( ) / FOV;


  if ( !animate )
  {
    _camera->position( position );
    _camera->rotation( rotation );
    _camera->radius( radius );
  }
  else
  {
    if ( _camera->isAniming( )) _camera->stopAnim( );
    delete _animation;


    _animation = new reto::CameraAnimation( reto::CameraAnimation::LINEAR ,
                                            reto::CameraAnimation::NONE ,
                                            reto::CameraAnimation::LINEAR );

    auto startCam = new reto::KeyCamera( 0.f , _camera->position( ) ,
                                         _camera->rotation( ) ,
                                         _camera->radius( ));
    _animation->addKeyCamera( startCam );

    auto targetCam = new reto::KeyCamera( CAMERA_ANIMATION_DURATION ,
                                          position , rotation , radius );
    _animation->addKeyCamera( targetCam );

    _camera->startAnim( _animation );
  }
}

void setColor( syncopa::TRenderMorpho& renderConfig , const vec3& color )
{
  for ( auto& c: std::get< syncopa::COLOR >( renderConfig ))
    c = color;
}

void OpenGLWidget::updateMorphologyModel(
  std::shared_ptr< syncopa::NeuronClusterManager > manager )
{
  _neuronModel.clear( );
  std::unordered_set< unsigned int > usedSomaNeurons;
  std::unordered_set< unsigned int > usedMorphologyNeurons;

  QString focused = manager->getFocused( );
  for ( const auto& cluster: manager->getClusters( ))
  {
    auto& metadata = manager->getMetadata( cluster.getName( ));
    if ( !metadata.enabled ||
         ( !focused.isEmpty( ) && cluster.getName( ) != focused ))
      continue;

    QString focusedSel = metadata.focusedSelection;
    for ( const auto& selection: cluster.getSelections( ))
    {
      auto& selectionMetadata = metadata.selection[ selection.first ];
      if ( !selectionMetadata.enabled ||
           ( !focusedSel.isEmpty( ) && focusedSel != selection.first ))
        continue;

      if ( selectionMetadata.partsToShow == syncopa::NeuronMetadataShowPart::ALL
           || selectionMetadata.partsToShow ==
              syncopa::NeuronMetadataShowPart::SOMA_ONLY )
      {

        // SOMA MODELS
        auto ids = selection.second;
        for ( const auto& item: usedSomaNeurons )
          ids.erase( item );

        auto model = _neuronScene->getRender( ids );

        setColor( model ,
                  glmToEigen( glm::vec3( qtToGLM( selectionMetadata.color ))));

        std::get< syncopa::SHOW_SOMA >( model ) = true;
        std::get< syncopa::SHOW_MORPHOLOGIES >( model ) = false;

        _neuronModel.push_back( model );

        usedSomaNeurons.insert( ids.cbegin( ) , ids.cend( ));
      }

      if ( selectionMetadata.partsToShow == syncopa::NeuronMetadataShowPart::ALL
           || selectionMetadata.partsToShow ==
              syncopa::NeuronMetadataShowPart::MORPHOLOGY_ONLY )
      {

        // MORPHOLOGY MODELS
        auto ids = selection.second;
        for ( const auto& item: usedMorphologyNeurons )
          ids.erase( item );


        auto model = _neuronScene->getRender( ids );

        setColor( model ,
                  glmToEigen( glm::vec3( qtToGLM( selectionMetadata.color ))));

        std::get< syncopa::SHOW_SOMA >( model ) = false;
        std::get< syncopa::SHOW_MORPHOLOGIES >( model ) = true;

        _neuronModel.push_back( model );

        usedMorphologyNeurons.insert( ids.cbegin( ) , ids.cend( ));
      }
    }
  }
}

void OpenGLWidget::updateSynapsesModel(
  std::shared_ptr< syncopa::NeuronClusterManager > manager )
{
  std::unordered_set< unsigned int > neuronsWithAllSynapses;
  std::unordered_set< unsigned int > neuronsWithConnectedSynapses;

  QString focused = manager->getFocused( );
  for ( const auto& cluster: manager->getClusters( ))
  {
    auto& metadata = manager->getMetadata( cluster.getName( ));
    if ( !metadata.enabled ||
         ( !focused.isEmpty( ) && cluster.getName( ) != focused ))
      continue;
    QString focusedSel = metadata.focusedSelection;
    for ( const auto& selection: cluster.getSelections( ))
    {
      auto& selectionMetadata = metadata.selection[ selection.first ];
      if ( !selectionMetadata.enabled
           || selectionMetadata.synapsesVisibility ==
              syncopa::SynapsesVisibility::HIDDEN
           || ( !focusedSel.isEmpty( ) && focusedSel != selection.first ))
        continue;

      for ( const auto id: selection.second )
      {
        neuronsWithConnectedSynapses.insert( id );
      }
      if ( selectionMetadata.synapsesVisibility ==
           syncopa::SynapsesVisibility::ALL )
      {
        for ( const auto id: selection.second )
        {
          neuronsWithAllSynapses.insert( id );
        }
      }
    }
  }

  _domainManager->loadConnectedSynapses( neuronsWithConnectedSynapses );
  _domainManager->loadSynapses( neuronsWithAllSynapses , true );
  _domainManager->updateSynapseMapping( );

  setupSynapses( );
}

void OpenGLWidget::updatePathsModel(
  std::shared_ptr< syncopa::NeuronClusterManager > manager )
{
  std::unordered_set< unsigned int > preNeuronsWithAllPaths;
  std::unordered_set< unsigned int > preNeuronsWithConnectedPaths;

  std::unordered_set< unsigned int > postNeuronsWithAllPaths;
  std::unordered_set< unsigned int > postNeuronsWithConnectedPaths;

  QString focused = manager->getFocused( );
  for ( const auto& cluster: manager->getClusters( ))
  {
    auto& metadata = manager->getMetadata( cluster.getName( ));
    if ( !metadata.enabled ||
         ( !focused.isEmpty( ) && cluster.getName( ) != focused ))
      continue;
    QString focusedSel = metadata.focusedSelection;
    for ( const auto& selection: cluster.getSelections( ))
    {
      auto& selectionMetadata = metadata.selection[ selection.first ];
      if ( !selectionMetadata.enabled
           || selectionMetadata.pathsVisibility ==
              syncopa::PathsVisibility::HIDDEN
           || ( !focusedSel.isEmpty( ) && focusedSel != selection.first ))
        continue;

      for ( const auto id: selection.second )
      {
        switch ( selectionMetadata.pathTypes )
        {
          case PathTypes::PRE_ONLY:
            preNeuronsWithConnectedPaths.insert( id );
            break;
          case PathTypes::POST_ONLY:
            postNeuronsWithConnectedPaths.insert( id );
            break;
          case PathTypes::ALL:
            preNeuronsWithConnectedPaths.insert( id );
            postNeuronsWithConnectedPaths.insert( id );
            break;
        }
      }
      if ( selectionMetadata.pathsVisibility ==
           syncopa::PathsVisibility::ALL )
      {
        for ( const auto id: selection.second )
        {
          switch ( selectionMetadata.pathTypes )
          {
            case PathTypes::PRE_ONLY:
              preNeuronsWithAllPaths.insert( id );
              break;
            case PathTypes::POST_ONLY:
              postNeuronsWithAllPaths.insert( id );
              break;
            case PathTypes::ALL:
              preNeuronsWithAllPaths.insert( id );
              postNeuronsWithAllPaths.insert( id );
              break;
          }
        }
      }
    }
  }

  bool empty = preNeuronsWithAllPaths.empty( ) &&
               preNeuronsWithConnectedPaths.empty( ) &&
               postNeuronsWithAllPaths.empty( ) &&
               postNeuronsWithConnectedPaths.empty( );

  if ( empty )
  {
    _particleManager.clearPaths( );
  }
  else
  {

    // TODO sizepaths
    const float pointSize =
      _particleManager.getPathModel( )->getParticlePreSize( ) *
      _particleSizeThreshold * 0.5f;
    auto& synapses = _dataset->circuit( ).synapses( );

    tsynapseVec outUsedSynapses;
    tsynapseVec outUsedPreSynapses;
    tsynapseVec outUsedPostSynapses;
    std::vector< vec3 > preOut;
    std::vector< vec3 > postOut;

    _pathFinder.configure(
      synapses ,
      preNeuronsWithAllPaths ,
      postNeuronsWithAllPaths ,
      preNeuronsWithConnectedPaths ,
      postNeuronsWithConnectedPaths ,
      pointSize ,
      preOut ,
      postOut
    );

    _particleManager.setPaths( preOut , postOut );
  }
}

void OpenGLWidget::initRenderToTexture( void )
{
  _oglFunctions = context( )->versionFunctions< QOpenGLFunctions_4_0_Core >( );
  _oglFunctions->initializeOpenGLFunctions( );

  glBindFramebuffer( GL_FRAMEBUFFER , defaultFramebufferObject( ));

  _screenPlaneShader = new reto::ShaderProgram( );
  _screenPlaneShader->loadVertexShaderFromText( SHADER_SCREEN_VERTEX );
  _screenPlaneShader->loadFragmentShaderFromText( SHADER_SCREEN_FRAGMENT );
  _screenPlaneShader->create( );
  _screenPlaneShader->link( );
  _screenPlaneShader->autocatching( true );
  _screenPlaneShader->use( );
  _screenPlaneShader->sendUniformi( "accumulation" , 0 );
  _screenPlaneShader->sendUniformi( "reveal" , 1 );
  _screenPlaneShader->sendUniformi( "opaque" , 2 );

  // Generate the opaque framebuffer

  _oglFunctions->glGenFramebuffers( 1 , &_opaqueFrameBuffer );
  _oglFunctions->glBindFramebuffer( GL_FRAMEBUFFER , _opaqueFrameBuffer );

  glGenTextures( 1 , &_opaqueColorTexture );
  glBindTexture( GL_TEXTURE_2D , _opaqueColorTexture );
  glTexImage2D( GL_TEXTURE_2D , 0 , GL_RGBA16F , width( ) , height( ) , 0 ,
                GL_RGBA , GL_HALF_FLOAT , NULL );
  glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );
  glBindTexture( GL_TEXTURE_2D , 0 );

  glGenTextures( 1 , &_opaqueDepthTexture );
  glBindTexture( GL_TEXTURE_2D , _opaqueDepthTexture );
  glTexImage2D( GL_TEXTURE_2D , 0 , GL_DEPTH_COMPONENT , width( ) , height( ) ,
                0 , GL_DEPTH_COMPONENT , GL_FLOAT , NULL );
  glBindTexture( GL_TEXTURE_2D , 0 );

  _oglFunctions->glFramebufferTexture2D( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 ,
                                         GL_TEXTURE_2D , _opaqueColorTexture ,
                                         0 );
  _oglFunctions->glFramebufferTexture2D( GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT ,
                                         GL_TEXTURE_2D , _opaqueDepthTexture ,
                                         0 );

  if ( _oglFunctions->glCheckFramebufferStatus( GL_FRAMEBUFFER ) !=
       GL_FRAMEBUFFER_COMPLETE )
    std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
              << std::endl;

  // Generate the OIR framebuffer

  _oglFunctions->glGenFramebuffers( 1 , &_weightFrameBuffer );
  _oglFunctions->glBindFramebuffer( GL_FRAMEBUFFER , _weightFrameBuffer );

  glGenTextures( 1 , &_accumulationTexture );
  glBindTexture( GL_TEXTURE_2D , _accumulationTexture );
  glTexImage2D( GL_TEXTURE_2D , 0 , GL_RGBA32F , width( ) , height( ) , 0 ,
                GL_RGBA , GL_FLOAT , nullptr );
  glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );

  glGenTextures( 1 , &_revealTexture );
  glBindTexture( GL_TEXTURE_2D , _revealTexture );
  glTexImage2D( GL_TEXTURE_2D , 0 , GL_R8 , width( ) , height( ) , 0 ,
                GL_RED , GL_FLOAT , nullptr );
  glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );

  _oglFunctions->glFramebufferTexture2D( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 ,
                                         GL_TEXTURE_2D , _accumulationTexture ,
                                         0 );
  _oglFunctions->glFramebufferTexture2D( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT1 ,
                                         GL_TEXTURE_2D , _revealTexture , 0 );

  // We must include the opaque depth texture!
  _oglFunctions->glFramebufferTexture2D( GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT ,
                                         GL_TEXTURE_2D , _opaqueDepthTexture ,
                                         0 );

  const GLenum transparentDrawBuffers[] = { GL_COLOR_ATTACHMENT0 ,
                                            GL_COLOR_ATTACHMENT1 };
  _oglFunctions->glDrawBuffers( 2 , transparentDrawBuffers );

  if ( _oglFunctions->glCheckFramebufferStatus( GL_FRAMEBUFFER ) !=
       GL_FRAMEBUFFER_COMPLETE )
    std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!"
              << std::endl;

  _oglFunctions->glBindFramebuffer( GL_FRAMEBUFFER ,
                                    defaultFramebufferObject( ));

  float quadVertices[] = {
    // positions
    -1.0f , -1.0f , 0.0f ,
    1.0f , -1.0f , 0.0f ,
    1.0f , 1.0f , 0.0f ,

    1.0f , 1.0f , 0.0f ,
    -1.0f , 1.0f , 0.0f ,
    -1.0f , -1.0f , 0.0f
  };

  // quad VAO
  unsigned int quadVBO;
  _oglFunctions->glGenVertexArrays( 1 , &_quadVAO );
  _oglFunctions->glGenBuffers( 1 , &quadVBO );
  _oglFunctions->glBindVertexArray( _quadVAO );
  _oglFunctions->glBindBuffer( GL_ARRAY_BUFFER , quadVBO );
  _oglFunctions->glBufferData( GL_ARRAY_BUFFER , sizeof( quadVertices ) ,
                               quadVertices ,
                               GL_STATIC_DRAW );
  _oglFunctions->glEnableVertexAttribArray( 0 );
  _oglFunctions->glVertexAttribPointer( 0 , 3 , GL_FLOAT , GL_FALSE ,
                                        3 * sizeof( float ) ,
                                        ( void* ) 0 );
  _oglFunctions->glBindVertexArray( 0 );
}

void OpenGLWidget::recalculateTextureDimensions( int width_ , int height_ )
{
  glBindTexture( GL_TEXTURE_2D , _opaqueColorTexture );
  glTexImage2D( GL_TEXTURE_2D , 0 , GL_RGBA16F , width_ , height_ , 0 ,
                GL_RGBA , GL_HALF_FLOAT , nullptr );

  glBindTexture( GL_TEXTURE_2D , _opaqueDepthTexture );
  glTexImage2D( GL_TEXTURE_2D , 0 , GL_DEPTH_COMPONENT , width_ , height_ , 0 ,
                GL_DEPTH_COMPONENT , GL_FLOAT , 0 );

  glBindTexture( GL_TEXTURE_2D , _accumulationTexture );
  glTexImage2D( GL_TEXTURE_2D , 0 , GL_RGBA32F , width_ , height_ , 0 ,
                GL_RGBA , GL_FLOAT , nullptr );

  glBindTexture( GL_TEXTURE_2D , _revealTexture );
  glTexImage2D( GL_TEXTURE_2D , 0 , GL_R8 , width_ , height_ , 0 ,
                GL_RED , GL_FLOAT , nullptr );

  glBindTexture( GL_TEXTURE_2D , 0 );
}

void OpenGLWidget::paintMorphologies( void )
{
  if ( _mode != syncopa::PATHS ) return;
  glDisable( GL_CULL_FACE );
  glFrontFace( GL_CCW );
  glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
  glEnable( GL_DEPTH_TEST );

  const Eigen::Matrix4f projection( _camera->camera( )->projectionMatrix( ));
  _nlrenderer->projectionMatrix( ) = projection;
  const Eigen::Matrix4f view( _camera->camera( )->viewMatrix( ));
  _nlrenderer->viewMatrix( ) = view;

  for ( const auto& model: _neuronModel )
  {
    _nlrenderer->render( std::get< syncopa::MESH >( model ) ,
                         std::get< syncopa::MATRIX >( model ) ,
                         std::get< syncopa::COLOR >( model ) ,
                         false ,
                         std::get< syncopa::SHOW_SOMA >( model ) ,
                         std::get< syncopa::SHOW_MORPHOLOGIES >( model ));
  }
}

void OpenGLWidget::paintParticles( void )
{
  const auto pos = _camera->position( );
  glm::vec3 cameraPosition( pos[ 0 ] , pos[ 1 ] , pos[ 2 ] );
  _lastCameraPosition = cameraPosition;

  _particleManager.draw( _mode == PATHS , _mode == PATHS && _dynamicActive );
}

void OpenGLWidget::paintGL( void )
{
  // First, let's flush the loaded meshes.
  if ( _neuronScene != nullptr )
  {
    _neuronScene->flushMeshesOnCPU( );
  }

  std::chrono::time_point< std::chrono::system_clock > now =
    std::chrono::system_clock::now( );

  unsigned int elapsedMicroseconds =
    std::chrono::duration_cast< std::chrono::microseconds >
      ( now - _lastFrame ).count( );

  _lastFrame = now;

  _deltaTime = elapsedMicroseconds * 0.000001;

  _elapsedTimeRenderAcc += elapsedMicroseconds;

  _frameCount++;

  if ( _paint )
  {
    _camera->anim( _deltaTime );

//    if( _psManager && _psManager->particleSystem( ))
    {

      if ( _elapsedTimeRenderAcc >= _renderPeriodMicroseconds )
      {
        if ( _dynamicMovement )
        {
          const float delta = _elapsedTimeRenderAcc * 0.000001;
          _particleManager.getDynamicModel( )->addTime( delta );
        }
        _elapsedTimeRenderAcc = 0.0f;
      }

      // DRAW

      glViewport( 0 , 0 , width( ) , height( ));

      if ( _particleManager.isAccumulativeMode( ))
      {
        glBindFramebuffer( GL_FRAMEBUFFER , defaultFramebufferObject( ));
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glDisable( GL_DEPTH_TEST );
        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA , GL_ONE_MINUS_CONSTANT_ALPHA );
        glBlendEquation( GL_FUNC_ADD );
        paintMorphologies( );
        paintParticles( );
      }
      else
      {
        // First, we draw the opaque elements (the morphologies)
        // in the default frame buffer object.
        glEnable( GL_DEPTH_TEST );
        glDepthFunc( GL_LESS );
        glDisable( GL_BLEND );
        glDepthMask( GL_TRUE );

        glBindFramebuffer( GL_FRAMEBUFFER , _opaqueFrameBuffer );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        paintMorphologies( );

        // Then, we draw the transparent elements (the synapses)
        // in the weighted frame buffer object.
        glDepthMask( GL_FALSE );
        glEnable( GL_BLEND );

        glBindFramebuffer( GL_FRAMEBUFFER , _weightFrameBuffer );
        // Keep the depth text, avoiding new writes to the depth texture.
        _oglFunctions->glBlendFunci( 0 , GL_ONE , GL_ONE );
        _oglFunctions->glBlendFunci( 1 , GL_ZERO , GL_ONE_MINUS_SRC_COLOR );
        glBlendEquation( GL_FUNC_ADD );

        // Clear textures. We mustn't clear the depth texture.
        glm::vec4 zeroFillerVec( 0.0f );
        glm::vec4 oneFillerVec( 1.0f );
        _oglFunctions->glClearBufferfv( GL_COLOR , 0 , &zeroFillerVec[ 0 ] );
        _oglFunctions->glClearBufferfv( GL_COLOR , 1 , &oneFillerVec[ 0 ] );

        paintParticles( );

        // Now, we paint the transparent part over the opaque part.
        // We can use the default frame buffer object directly for this part.
        glBindFramebuffer( GL_FRAMEBUFFER , defaultFramebufferObject( ));
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glDisable( GL_DEPTH_TEST );
        glDisable( GL_BLEND );

        _screenPlaneShader->use( );
        _oglFunctions->glActiveTexture( GL_TEXTURE0 );
        _oglFunctions->glBindTexture( GL_TEXTURE_2D , _accumulationTexture );
        _oglFunctions->glActiveTexture( GL_TEXTURE1 );
        _oglFunctions->glBindTexture( GL_TEXTURE_2D , _revealTexture );
        _oglFunctions->glActiveTexture( GL_TEXTURE2 );
        _oglFunctions->glBindTexture( GL_TEXTURE_2D , _opaqueColorTexture );

        _oglFunctions->glBindVertexArray( _quadVAO );
        glDrawArrays( GL_TRIANGLES , 0 , 6 );
        update( );
      }
    }

    glUseProgram( 0 );
//    glFlush( );

  }

#define FRAMES_PAINTED_TO_MEASURE_FPS 30
  if ( _frameCount == FRAMES_PAINTED_TO_MEASURE_FPS )
  {
    const auto duration =
      std::chrono::duration_cast< std::chrono::milliseconds >( now - _then );
    _then = now;

    MainWindow* mainWindow = dynamic_cast< MainWindow* >( parent( ));
    if ( mainWindow )
    {

      if ( _showFps )
      {
        const unsigned int ellapsedMiliseconds = duration.count( );

        const unsigned int fps = roundf( 1000.0f *
                                         float(
                                           FRAMES_PAINTED_TO_MEASURE_FPS ) /
                                         float( ellapsedMiliseconds ));

        _fpsLabel.setVisible( true );
        _fpsLabel.setText( QString::number( fps ) + QString( " FPS" ));
        _fpsLabel.adjustSize( );
      }
      else
        _fpsLabel.setVisible( false );
    }

    _frameCount = 0;
  }

  if ( _idleUpdate )
  {
    update( );
  }
  else
  {
    _fpsLabel.setVisible( false );
  }
}

void OpenGLWidget::idleUpdate( bool idleUpdate_ )
{
  _idleUpdate = idleUpdate_;
}

void OpenGLWidget::showFps( bool showFps_ )
{
  _showFps = showFps_;
}

void OpenGLWidget::resizeGL( int w , int h )
{
  _currentWidth = w;
  _currentHeight = h;

  _inverseResolution =
    Eigen::Vector2f( 1.0 / _currentWidth , 1.0 / _currentHeight );

  _camera->windowSize( w , h );
  glViewport( 0 , 0 , w , h );

  recalculateTextureDimensions( w , h );
}

void OpenGLWidget::mousePressEvent( QMouseEvent* event_ )
{
  if ( event_->button( ) == Qt::LeftButton )
  {
    _rotation = true;
    _mouseX = event_->x( );
    _mouseY = event_->y( );
  }

  if ( event_->button( ) == Qt::RightButton )
  {
    _translation = true;
    _mouseX = event_->x( );
    _mouseY = event_->y( );
  }

  update( );
}

void OpenGLWidget::mouseReleaseEvent( QMouseEvent* event_ )
{
  if ( event_->button( ) == Qt::LeftButton )
  {
    _rotation = false;
  }

  if ( event_->button( ) == Qt::RightButton )
  {
    _translation = false;
  }

  update( );
}

void OpenGLWidget::mouseMoveEvent( QMouseEvent* event_ )
{
  constexpr float TRANSLATION_FACTOR = 0.001f;
  constexpr float ROTATION_FACTOR = 0.01f;

  const auto diffX = static_cast<float>(event_->x( ) - _mouseX);
  const auto diffY = static_cast<float>(event_->y( ) - _mouseY);

  auto updateLastEventCoords = [ this ]( const QMouseEvent* e )
  {
    _mouseX = e->x( );
    _mouseY = e->y( );
  };

  if ( _rotation )
  {
    _camera->rotate(
      Eigen::Vector3f(
        diffX * ROTATION_FACTOR ,
        diffY * ROTATION_FACTOR ,
        0.0f
      )
    );

    updateLastEventCoords( event_ );
  }

  if ( _translation )
  {
    const float xDis = diffX * TRANSLATION_FACTOR * _camera->radius( );
    const float yDis = diffY * TRANSLATION_FACTOR * _camera->radius( );

    _camera->localTranslate( Eigen::Vector3f( -xDis , yDis , 0.0f ));
    updateLastEventCoords( event_ );
  }

  update( );
}


void OpenGLWidget::wheelEvent( QWheelEvent* event_ )
{

  int delta = event_->angleDelta( ).y( );

  if ( delta > 0 )
    _camera->radius( _camera->radius( ) / 1.1f );
  else
    _camera->radius( _camera->radius( ) * 1.1f );

  update( );
}

void OpenGLWidget::keyPressEvent( QKeyEvent* event_ )
{
  makeCurrent( );

  switch ( event_->key( ))
  {
    case Qt::Key_C:
      _camera->position( Eigen::Vector3f( 0.0f , 0.0f , 0.0f ));
      _camera->radius( 1000.0f );
      _camera->rotate( Eigen::Vector3f( 0.0f , 0.0f , 0.0f ));
      update( );
      break;
  }
}

void OpenGLWidget::changeClearColor( void )
{
  QColor color =
    QColorDialog::getColor( _currentClearColor , parentWidget( ) ,
                            "Choose new background color" ,
                            QColorDialog::DontUseNativeDialog );

  if ( color.isValid( ))
  {
    _currentClearColor = color;

    makeCurrent( );
    glClearColor( float( _currentClearColor.red( )) / 255.0f ,
                  float( _currentClearColor.green( )) / 255.0f ,
                  float( _currentClearColor.blue( )) / 255.0f ,
                  float( _currentClearColor.alpha( )) / 255.0f );
    update( );
  }
}


void OpenGLWidget::toggleUpdateOnIdle( void )
{
  _idleUpdate = !_idleUpdate;
  if ( _idleUpdate )
    update( );
}

void OpenGLWidget::toggleShowFPS( void )
{
  _showFps = !_showFps;
  if ( _idleUpdate )
    update( );
}

nsol::DataSet* OpenGLWidget::dataset( void )
{
  return _dataset;
}

const std::vector< nsol::MorphologySynapsePtr >&
OpenGLWidget::currentSynapses( void )
{
  return _domainManager->getSynapses( );
}

void OpenGLWidget::colorSynapsesPre( const syncopa::vec3& color )
{
  auto in = _particleManager.getSynapseModel( )->getParticlePreColor( );
  in.r = color.x( );
  in.g = color.y( );
  in.b = color.z( );
  _particleManager.getSynapseModel( )->setParticlePreColor( in );
}

void OpenGLWidget::colorSynapsesPost( const syncopa::vec3& color )
{
  auto in = _particleManager.getSynapseModel( )->getParticlePostColor( );
  in.r = color.x( );
  in.g = color.y( );
  in.b = color.z( );
  _particleManager.getSynapseModel( )->setParticlePostColor( in );
}

void OpenGLWidget::colorPathsPre( const syncopa::vec3& color )
{
  auto in = _particleManager.getPathModel( )->getParticlePreColor( );
  in.r = color.x( );
  in.g = color.y( );
  in.b = color.z( );
  _particleManager.getPathModel( )->setParticlePreColor( in );
}

void OpenGLWidget::colorPathsPost( const syncopa::vec3& color )
{
  auto in = _particleManager.getPathModel( )->getParticlePostColor( );
  in.r = color.x( );
  in.g = color.y( );
  in.b = color.z( );
  _particleManager.getPathModel( )->setParticlePostColor( in );
}

void OpenGLWidget::colorDynamicPre( const syncopa::vec3& color )
{
  auto in = _particleManager.getDynamicModel( )->getParticlePreColor( );
  in.r = color.x( );
  in.g = color.y( );
  in.b = color.z( );
  _particleManager.getDynamicModel( )->setParticlePreColor( in );
}

void OpenGLWidget::colorDynamicPost( const syncopa::vec3& color )
{
  auto in = _particleManager.getDynamicModel( )->getParticlePostColor( );
  in.r = color.x( );
  in.g = color.y( );
  in.b = color.z( );
  _particleManager.getDynamicModel( )->setParticlePostColor( in );
}

const syncopa::vec3 OpenGLWidget::colorSynapsesPre( void ) const
{
  auto color = _particleManager.getSynapseModel( )->getParticlePreColor( );
  return syncopa::vec3( color.r , color.g , color.b );
}

const syncopa::vec3 OpenGLWidget::colorSynapsesPost( void ) const
{
  auto color = _particleManager.getSynapseModel( )->getParticlePostColor( );
  return syncopa::vec3( color.r , color.g , color.b );
}

const syncopa::vec3 OpenGLWidget::colorPathsPre( void ) const
{
  auto color = _particleManager.getPathModel( )->getParticlePreColor( );
  return syncopa::vec3( color.r , color.g , color.b );
}

const syncopa::vec3 OpenGLWidget::colorPathsPost( void ) const
{
  auto color = _particleManager.getPathModel( )->getParticlePostColor( );
  return syncopa::vec3( color.r , color.g , color.b );
}

const syncopa::vec3 OpenGLWidget::colorDynamicPre( void ) const
{
  auto color = _particleManager.getDynamicModel( )->getParticlePreColor( );
  return syncopa::vec3( color.r , color.g , color.b );
}

const syncopa::vec3 OpenGLWidget::colorDynamicPost( void ) const
{
  auto color = _particleManager.getDynamicModel( )->getParticlePostColor( );
  return syncopa::vec3( color.r , color.g , color.b );
}

void OpenGLWidget::colorSynapseMap( const tQColorVec& qcolors )
{
  _colorSynMap = qcolors;

  tColorVec colors;
  colors.reserve( qcolors.size( ));

  for ( auto color: qcolors )
  {
    glm::vec4 composedColor = qtToGLM( color.second );
    composedColor.w = _alphaSynapsesMap;

    colors.emplace_back( std::make_pair( color.first , composedColor ));
  }

  _particleManager.getSynapseGradientModel( )->setGradient( colors );
}

tQColorVec OpenGLWidget::colorSynapseMap( void ) const
{
  return _colorSynMap;
}

void OpenGLWidget::alphaSynapsesPre( float transparency )
{
  auto color = _particleManager.getSynapseModel( )->getParticlePreColor( );
  color.a = transparency;
  _particleManager.getSynapseModel( )->setParticlePreColor( color );
}

void OpenGLWidget::alphaSynapsesPost( float transparency )
{
  auto color = _particleManager.getSynapseModel( )->getParticlePostColor( );
  color.a = transparency;
  _particleManager.getSynapseModel( )->setParticlePostColor( color );
}

void OpenGLWidget::alphaPathsPre( float transparency )
{
  auto color = _particleManager.getPathModel( )->getParticlePreColor( );
  color.a = transparency;
  _particleManager.getPathModel( )->setParticlePreColor( color );
}

void OpenGLWidget::alphaPathsPost( float transparency )
{
  auto color = _particleManager.getPathModel( )->getParticlePostColor( );
  color.a = transparency;
  _particleManager.getPathModel( )->setParticlePostColor( color );
}

void
OpenGLWidget::alphaSynapseMap( const tQColorVec& colors , float transparency )
{
  _alphaSynapsesMap = transparency;
  colorSynapseMap( colors );
}

float OpenGLWidget::alphaSynapsesPre( void ) const
{
  return _particleManager.getSynapseModel( )->getParticlePreColor( ).a;
}

float OpenGLWidget::alphaSynapsesPost( void ) const
{
  return _particleManager.getSynapseModel( )->getParticlePostColor( ).a;
}

float OpenGLWidget::alphaPathsPre( void ) const
{
  return _particleManager.getPathModel( )->getParticlePreColor( ).a;
}

float OpenGLWidget::alphaPathsPost( void ) const
{
  return _particleManager.getPathModel( )->getParticlePostColor( ).a;
}

float OpenGLWidget::alphaSynapsesMap( void ) const
{
  return _alphaSynapsesMap;
}

void OpenGLWidget::showSynapsesPre( bool state )
{
  _particleManager.getSynapseModel( )->setParticlePreVisibility( state );
}

void OpenGLWidget::showSynapsesPost( bool state )
{
  _particleManager.getSynapseModel( )->setParticlePostVisibility( state );
}

void OpenGLWidget::showPathsPre( bool state )
{
  _particleManager.getPathModel( )->setParticlePreVisibility( state );
}

void OpenGLWidget::showPathsPost( bool state )
{
  _particleManager.getPathModel( )->setParticlePostVisibility( state );
}

bool OpenGLWidget::dynamicActive( void ) const
{
  return _dynamicActive;
}

void OpenGLWidget::startDynamic( void )
{
  if ( _mode != PATHS || _dynamicActive )
    return;

  stopDynamic( );
  auto particles = DynamicPathGenerator::generateParticles(
    _pathFinder , 0.002f , 200.0f );

  auto& model = _particleManager.getDynamicModel( );
  model->setMaxTime( particles.second );
  model->setTimestamp( 0.0f );

  _particleManager.setDynamic( particles.first );

  _dynamicMovement = true;
  _dynamicActive = true;
}

bool OpenGLWidget::toggleDynamicMovement( void )
{
  _dynamicMovement = !_dynamicMovement;

  return _dynamicMovement;
}

void OpenGLWidget::stopDynamic( void )
{
  _particleManager.clearDynamic( );
  _dynamicActive = false;
}

void OpenGLWidget::setSynapseMappingState( bool state )
{
  if ( !_dataset )
    return;
  _mapSynapseValues = state;
  _domainManager->synapseMappingAttrib( _currentSynapseAttrib );
  setupSynapses( );
}

void OpenGLWidget::setSynapseMapping( int attrib )
{
  if ( !_dataset )
    return;

  if ( attrib < 0 || ( attrib > static_cast<int>(TBSA_SYNAPSE_OTHER)))
    return;

  _currentSynapseAttrib = static_cast<TBrainSynapseAttribs>(attrib);

  _domainManager->synapseMappingAttrib( _currentSynapseAttrib );

  setupSynapses( );
}

const QPolygonF& OpenGLWidget::getSynapseMappingPlot( void ) const
{
  return _domainManager->getSynapseMappingPlot( );
}

void OpenGLWidget::filteringState( bool state )
{
  _domainManager->setSynapseFilteringState( state );

  setupSynapses( );
  setupPaths( );
}

void OpenGLWidget::filteringBounds( float min , float max )
{
  _domainManager->setSynapseFilter( min , max , false );

  setupSynapses( );
  setupPaths( );
}

std::pair< float , float > OpenGLWidget::rangeBounds( void ) const
{
  return _domainManager->rangeBounds( );
}


void OpenGLWidget::alphaMode( bool alphaAccumulative )
{
  _particleManager.setAccumulativeMode( alphaAccumulative );

  // This update is required to refresh the interface.
  update( );
}

void OpenGLWidget::mode( TMode mode_ )
{
  _mode = mode_;
  setupSynapses( );
  setupPaths( );
  home( );
}

TMode OpenGLWidget::mode( void ) const
{
  return _mode;
}

void OpenGLWidget::sizeSynapsesPre( float newSize )
{
  _particleManager.getSynapseModel( )->setParticlePreSize( newSize );
}

void OpenGLWidget::sizeSynapsesPost( float newSize )
{
  _particleManager.getSynapseModel( )->setParticlePostSize( newSize );
}

void OpenGLWidget::sizePathsPre( float newSize )
{
  _particleManager.getPathModel( )->setParticlePreSize( newSize );
}

void OpenGLWidget::sizePathsPost( float newSize )
{
  _particleManager.getPathModel( )->setParticlePostSize( newSize );
}

void OpenGLWidget::sizeSynapseMap( float newSize )
{
  _particleManager.getSynapseGradientModel( )->setParticlePreSize( newSize );
  _particleManager.getSynapseGradientModel( )->setParticlePostSize( newSize );
}

void OpenGLWidget::sizeDynamic( float newSize )
{
  _particleManager.getDynamicModel( )->setParticlePreSize( newSize );
  _particleManager.getDynamicModel( )->setParticlePostSize( newSize );
}

float OpenGLWidget::sizeSynapsesPre( void ) const
{
  return _particleManager.getSynapseModel( )->getParticlePreSize( );
}

float OpenGLWidget::sizeSynapsesPost( void ) const
{
  return _particleManager.getSynapseModel( )->getParticlePostSize( );
}

float OpenGLWidget::sizePathsPre( void ) const
{
  return _particleManager.getPathModel( )->getParticlePreSize( );
}

float OpenGLWidget::sizePathsPost( void ) const
{
  return _particleManager.getPathModel( )->getParticlePostSize( );
}

float OpenGLWidget::sizeSynapseMap( void ) const
{
  return _particleManager.getSynapseGradientModel( )->getParticlePostSize( );
}

float OpenGLWidget::sizeDynamic( void ) const
{
  return _particleManager.getDynamicModel( )->getParticlePreSize( );
}

void OpenGLWidget::loadPostprocess( )
{
  makeCurrent( );

  _neuronScene->color( vec3( 1 , 1 , 1 ) , PRESYNAPTIC );
  _neuronScene->color( vec3( 0 , 1 , 1 ) , POSTSYNAPTIC );

  _gidsAll.clear( );
  for ( const auto& neuron: _dataset->neurons( ))
  {
    _gidsAll.insert( neuron.first );
  }

  createParticleSystem( );
}

DomainManager* OpenGLWidget::getDomainManager( ) const
{
  return _domainManager;
}

const gidUSet& OpenGLWidget::getGidsAll( ) const
{
  return _gidsAll;
}
