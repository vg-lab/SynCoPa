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

#include <prefr/GL/OIGLRenderer.h>
#include <prefr/GL/GLRenderer.h>
#include "MainWindow.h"
#include "prefr/Camera.h"

#include "prefr/PrefrShaders.h"
#include "prefr/ShaderProgram.h"

using namespace syncopa;

constexpr float CAMERA_ANIMATION_DURATION = 0.75; /** camera animation duration in seconds. */

constexpr int SUPER_SAMPLING_SCALE = 2;

const static std::string vertexShaderCode = R"(#version 400
in vec2 inPosition;
out vec2 uv;

void main( void )
{
  uv = (inPosition + vec2( 1.0, 1.0 )) * 0.5;
  gl_Position = vec4( inPosition,  0.1, 1.0 );
})";

const static std::string screenFragment = R"(#version 400
out vec3 FragColor;

in vec2 uv;

uniform sampler2D screenTexture;
uniform sampler2D depthTexture;

uniform float zNear;
uniform float zFar;

float LinearizeDepth(float depth)
{
  float z = depth * 2.0 - 1.0;
  return (2.0 * zNear * zFar) / (zFar + zNear - z * (zFar - zNear));
}

void main( void )
{
  //float depthValue = LinearizeDepth( texture( screenTexture, uv ).r ) / zFar;
  //FragColor = vec3( depthValue, depthValue, depthValue );
  FragColor = texture(screenTexture, uv).rgb;
  gl_FragDepth = texture( depthTexture, uv ).r;
  //FragColor = vec3(vec3(1.0), 1.0);
})";

OpenGLWidget::OpenGLWidget(
  QWidget* parent ,
  Qt::WindowFlags windowFlags
                          )
  : QOpenGLWidget( parent , windowFlags )
  , _fpsLabel( this )
  , _showFps( false )
  , _frameCount( 0 )
  , _deltaTime( 0.0f )
  , _mouseX( 0 )
  , _mouseY( 0 )
  , _rotation( false )
  , _translation( false )
  , _idleUpdate( false )
  , _paint( true )
  , _currentClearColor( 20 , 20 , 20 , 0 )
  , _particlesShader( nullptr )
  , _nlrenderer( nullptr )
  , _dataset( nullptr )
  , _neuronScene( nullptr )
  , _psManager( nullptr )
  , _pathFinder( nullptr )
  , _dynPathManager( nullptr )
  , _domainManager( nullptr )
  , _mode( UNDEFINED )
  , _particleSizeThreshold( 0.45 )
  , _elapsedTimeRenderAcc( 0.0f )
  , _alphaBlendingAccumulative( false )
  , _colorSynapsesPre( 0 , 1 , 0 )
  , _colorSynapsesPost( 0.94 , 0 , 0.5 )
  , _colorPathsPre( 0.75 , 0.35 , 0.09 )
  , _colorPathsPost( 0 , 0 , 1 )
  , _colorSynMapPre( 0.5 , 0 , 0.5 )
  , _colorSynMapPost( 1 , 0 , 0 )
  , _alphaSynapsesPre( 0.55f )
  , _alphaSynapsesPost( 0.55f )
  , _alphaPathsPre( 0.8f )
  , _alphaPathsPost( 0.8f )
  , _alphaSynapsesMap( 0.55f )
  , _showSynapsesPre( true )
  , _showSynapsesPost( true )
  , _showPathsPre( true )
  , _showPathsPost( true )
  , _dynamicActive( false )
  , _dynamicMovement( true )
  , _oglFunctions( nullptr )
  , _screenPlaneShader( nullptr )
  , _screenPlaneVao( 0 )
  , _depthFrameBuffer( 0 )
  , _textureColor( 0 )
  , _textureDepth( 0 )
  , _msaaSamples( 4 )
  , _msaaFrameBuffer( 0 )
  , _msaaTextureColor( 0 )
  , _msaaTextureDepth( 0 )
  , _currentWidth( 0 )
  , _currentHeight( 0 )
  , _mapSynapseValues( false )
  , _currentSynapseAttrib( TBSA_SYNAPSE_DELAY )
{
  _camera = new Camera( );
  _camera->camera( )->farPlane( 5000 );
  _animation = new reto::CameraAnimation( );

  _pathFinder = new syncopa::PathFinder( );
  _dynPathManager = new syncopa::DynamicPathManager( );
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

  _maxFPS = 60.0f;
  _renderPeriod = 1.0f / _maxFPS;
  _renderPeriodMicroseconds = _renderPeriod * 1000000;

  _renderSpeed = 1.f;

  new QShortcut( QKeySequence( Qt::Key_Tab ) , this ,
                 SLOT( toggleDynamicMovement( )));
}

OpenGLWidget::~OpenGLWidget( void )
{
  delete _camera;
  delete _particlesShader;
  delete _psManager;
  delete _pathFinder;
}

void OpenGLWidget::initializeGL( void )
{
  initializeOpenGLFunctions( );

  glEnable( GL_DEPTH_TEST );
  glClearColor( static_cast<float>( _currentClearColor.red( )) / 255.0f ,
                static_cast<float>( _currentClearColor.green( )) / 255.0f ,
                static_cast<float>( _currentClearColor.blue( )) / 255.0f ,
                static_cast<float>( _currentClearColor.alpha( )) / 255.0f );
  glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
  glEnable( GL_CULL_FACE );

  glLineWidth( 1.5 );

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
  if ( _dataset )
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
  _pathFinder->dataset( _dataset , &_domainManager->synapsesInfo( ));

  _neuronScene = new syncopa::NeuronScene( _dataset );

  connect( _neuronScene ,
           SIGNAL( progress(
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
  prefr::Config::init( );

  _particlesShader = new ShaderProgram( );
  _particlesShader->loadVertexShaderFromText( prefr::prefrVertexShader );
  _particlesShader->loadFragmentShaderFromText( prefr::prefrSoftParticles );
  _particlesShader->create( );
  _particlesShader->link( );
  _particlesShader->autocatching( true );

  _psManager = new syncopa::PSManager( );
  _psManager->init( _pathFinder , 500000 , _camera , _particlesShader );

  _psManager->colorSynapses(
    vec4( _colorSynapsesPre.x( ) , _colorSynapsesPre.y( ) ,
          _colorSynapsesPre.z( ) , _alphaSynapsesPre ) ,
    PRESYNAPTIC );

  _psManager->colorSynapses(
    vec4( _colorSynapsesPost.x( ) , _colorSynapsesPost.y( ) ,
          _colorSynapsesPost.z( ) , _alphaSynapsesPost ) ,
    POSTSYNAPTIC );

  _psManager->colorPaths( vec4( _colorPathsPre.x( ) , _colorPathsPre.y( ) ,
                                _colorPathsPre.z( ) , _alphaPathsPre ) ,
                          PRESYNAPTIC );

  _psManager->colorPaths( vec4( _colorPathsPost.x( ) , _colorPathsPost.y( ) ,
                                _colorPathsPost.z( ) , _alphaPathsPost ) ,
                          POSTSYNAPTIC );

  _psManager->sizeSynapses( 8.0 , PRESYNAPTIC );
  _psManager->sizeSynapses( 8.0 , POSTSYNAPTIC );

  _psManager->sizePaths( 3.0 , PRESYNAPTIC );
  _psManager->sizePaths( 3.0 , POSTSYNAPTIC );

  _dynPathManager->init( _pathFinder , _psManager );
}

void OpenGLWidget::setupSynapses( void )
{
  if ( !_mapSynapseValues )
    _psManager->configureSynapses( _domainManager->getSynapses( ));
  else
  {
    _psManager->configureMappedSynapses(
      _domainManager->getFilteredSynapses( ) ,
      _domainManager->getFilteredNormValues( ));
  }
}

void OpenGLWidget::setupPaths( void )
{
  if(_mode == SYNAPSES)
    _psManager->clearPaths( );
}

void OpenGLWidget::home( bool animate )
{
  const float FOV = sin( _camera->camera( )->fieldOfView( ));
  const auto rotation = Eigen::Vector3f{ 0.0f , 0.0f , 0.0f };
  const auto bb = _psManager->boundingBox( );
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
  std::shared_ptr <syncopa::NeuronClusterManager> manager )
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
  std::shared_ptr <syncopa::NeuronClusterManager> manager )
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
  updateSynapsesVisibility( );
}

void OpenGLWidget::updatePathsModel(
  std::shared_ptr <syncopa::NeuronClusterManager> manager )
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

  const bool empty = preNeuronsWithAllPaths.empty( ) &&
                     preNeuronsWithConnectedPaths.empty( ) &&
                     postNeuronsWithAllPaths.empty( ) &&
                     postNeuronsWithConnectedPaths.empty( );

  if ( empty )
  {
    _psManager->clearPaths( );
  }
  else
  {
    const float pointSize =
      _psManager->sizePaths( ) * _particleSizeThreshold * 0.5f;
    auto& synapses = _dataset->circuit( ).synapses( );

    tsynapseVec outUsedSynapses;
    tsynapseVec outUsedPreSynapses;
    tsynapseVec outUsedPostSynapses;
    std::vector <vec3> preOut;
    std::vector <vec3> postOut;

    _pathFinder->configure(
      synapses ,
      preNeuronsWithAllPaths ,
      postNeuronsWithAllPaths ,
      preNeuronsWithConnectedPaths ,
      postNeuronsWithConnectedPaths ,
      pointSize ,
      preOut ,
      postOut
    );

    _psManager->setupPath( preOut , PRESYNAPTIC);
    _psManager->setupPath( postOut , POSTSYNAPTIC , false );
  }

  updatePathsVisibility( );
}

void OpenGLWidget::initRenderToTexture( void )
{
  _oglFunctions = context( )->versionFunctions< QOpenGLFunctions_4_0_Core >( );
  _oglFunctions->initializeOpenGLFunctions( );

  glBindFramebuffer( GL_FRAMEBUFFER , defaultFramebufferObject( ));

  _screenPlaneShader = new reto::ShaderProgram( );
  _screenPlaneShader->loadVertexShaderFromText( vertexShaderCode );
  _screenPlaneShader->loadFragmentShaderFromText( screenFragment );
  _screenPlaneShader->create( );
  _screenPlaneShader->link( );
  _screenPlaneShader->autocatching( true );

  // Create MSAA framebuffer and textures
  glGenFramebuffers( 1 , &_msaaFrameBuffer );
  glBindFramebuffer( GL_FRAMEBUFFER , _msaaFrameBuffer );

  glGenTextures( 1 , &_msaaTextureColor );
  glBindTexture( GL_TEXTURE_2D_MULTISAMPLE , _msaaTextureColor );
  _oglFunctions->glTexImage2DMultisample(
    GL_TEXTURE_2D_MULTISAMPLE , _msaaSamples , GL_RGB ,
    width( ) * SUPER_SAMPLING_SCALE ,
    height( ) * SUPER_SAMPLING_SCALE ,
    GL_TRUE );

  glGenTextures( 1 , &_msaaTextureDepth );
  glBindTexture( GL_TEXTURE_2D_MULTISAMPLE , _msaaTextureDepth );
  _oglFunctions->glTexImage2DMultisample(
    GL_TEXTURE_2D_MULTISAMPLE , _msaaSamples , GL_DEPTH_COMPONENT ,
    width( ) * SUPER_SAMPLING_SCALE ,
    height( ) * SUPER_SAMPLING_SCALE , GL_TRUE );


  // Bind Multisample textures to MSAA framebuffer
  glBindTexture( GL_TEXTURE_2D_MULTISAMPLE , 0 );
  glFramebufferTexture2D( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 ,
                          GL_TEXTURE_2D_MULTISAMPLE , _msaaTextureColor , 0 );
  glFramebufferTexture2D( GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT ,
                          GL_TEXTURE_2D_MULTISAMPLE , _msaaTextureDepth , 0 );

  if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
  {
    std::cerr << "Error: creating MSAA FrameBuffer" << std::endl;
  }

  glBindFramebuffer( GL_FRAMEBUFFER , defaultFramebufferObject( ));


  // Create intermediate framebuffer and color and depth textures
  glActiveTexture( GL_TEXTURE0 );
  glGenTextures( 1 , &_textureColor );
  glBindTexture( GL_TEXTURE_2D , _textureColor );
  glTexImage2D( GL_TEXTURE_2D , 0 , GL_RGB , width( ) * SUPER_SAMPLING_SCALE ,
                height( ) * SUPER_SAMPLING_SCALE , 0 ,
                GL_RGB ,
                GL_UNSIGNED_BYTE , 0 );
  glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );

  glGenTextures( 1 , &_textureDepth );
  glBindTexture( GL_TEXTURE_2D , _textureDepth );
  glTexImage2D( GL_TEXTURE_2D , 0 , GL_DEPTH_COMPONENT ,
                width( ) * SUPER_SAMPLING_SCALE ,
                height( ) * SUPER_SAMPLING_SCALE , 0 , GL_DEPTH_COMPONENT ,
                GL_FLOAT ,
                NULL );

  glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_NEAREST );

  glGenFramebuffers( 1 , &_depthFrameBuffer );
  glBindFramebuffer( GL_FRAMEBUFFER , _depthFrameBuffer );

  glFramebufferTexture2D( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 ,
                          GL_TEXTURE_2D ,
                          _textureColor , 0 );

  glFramebufferTexture2D( GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT , GL_TEXTURE_2D ,
                          _textureDepth , 0 );

  if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
  {
    std::cerr << "Error: creating intermediate FrameBuffer" << std::endl;
  }

  glBindFramebuffer( GL_FRAMEBUFFER , defaultFramebufferObject( ));

  static const float quadBufferData[] =
    {
      -1.0f , 1.0f ,
      -1.0f , -1.0f ,
      1.0f , -1.0f ,
      1.0f , 1.0f ,
    };

  static const unsigned int quadIndices[] =
    {
      0 , 1 , 2 ,
      0 , 2 , 3 ,
    };

  _oglFunctions->glGenVertexArrays( 1 , &_screenPlaneVao );
  _oglFunctions->glBindVertexArray( _screenPlaneVao );

  unsigned int vbo[2];
  glGenBuffers( 2 , vbo );
  glBindBuffer( GL_ARRAY_BUFFER , vbo[ 0 ] );
  glBufferData( GL_ARRAY_BUFFER , sizeof( quadBufferData ) , quadBufferData ,
                GL_STATIC_DRAW );
  glVertexAttribPointer( 0 , 2 , GL_FLOAT , GL_FALSE , 0 , 0 );
  glEnableVertexAttribArray( 0 );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER , vbo[ 1 ] );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER , sizeof( quadIndices ) , quadIndices ,
                GL_STATIC_DRAW );
}

void OpenGLWidget::generateDepthTexture( int width_ , int height_ )
{
  glBindTexture( GL_TEXTURE_2D , _textureColor );
  glTexImage2D( GL_TEXTURE_2D , 0 , GL_RGB , width_ * SUPER_SAMPLING_SCALE ,
                height_ * SUPER_SAMPLING_SCALE , 0 ,
                GL_RGB , GL_UNSIGNED_BYTE , 0 );

  glBindTexture( GL_TEXTURE_2D , _textureDepth );
  glTexImage2D( GL_TEXTURE_2D , 0 , GL_DEPTH_COMPONENT ,
                width_ * SUPER_SAMPLING_SCALE , height_ * SUPER_SAMPLING_SCALE ,
                0 , GL_DEPTH_COMPONENT , GL_FLOAT ,
                0 );

  glBindTexture( GL_TEXTURE_2D , 0 );

  glBindTexture( GL_TEXTURE_2D_MULTISAMPLE , _msaaTextureColor );
  _oglFunctions->glTexImage2DMultisample(
    GL_TEXTURE_2D_MULTISAMPLE , _msaaSamples , GL_RGB ,
    width( ) * SUPER_SAMPLING_SCALE ,
    height( ) * SUPER_SAMPLING_SCALE ,
    GL_TRUE );

  glBindTexture( GL_TEXTURE_2D_MULTISAMPLE , _msaaTextureDepth );
  _oglFunctions->glTexImage2DMultisample(
    GL_TEXTURE_2D_MULTISAMPLE , _msaaSamples , GL_DEPTH_COMPONENT ,
    width( ) * SUPER_SAMPLING_SCALE ,
    height( ) * SUPER_SAMPLING_SCALE , GL_TRUE );

  glBindTexture( GL_TEXTURE_2D_MULTISAMPLE , 0 );
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
                         std::get< syncopa::SHOW_SOMA >( model ) ,
                         std::get< syncopa::SHOW_SOMA >( model ) ,
                         std::get< syncopa::SHOW_MORPHOLOGIES >( model ));
  }
}


void OpenGLWidget::performMSAA( void )
{
  glBindFramebuffer( GL_READ_FRAMEBUFFER , _msaaFrameBuffer );
  glBindFramebuffer( GL_DRAW_FRAMEBUFFER , _depthFrameBuffer );
  _oglFunctions->glBlitFramebuffer( 0 , 0 , width( ) * SUPER_SAMPLING_SCALE ,
                                    height( ) * SUPER_SAMPLING_SCALE , 0 ,
                                    0 ,
                                    width( ) * SUPER_SAMPLING_SCALE ,
                                    height( ) * SUPER_SAMPLING_SCALE ,
                                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ,
                                    GL_NEAREST );
}

void OpenGLWidget::paintParticles( void )
{
  if ( !_particlesShader ) return;

  _particlesShader->use( );

  unsigned int shader;
  shader = _particlesShader->program( );

  GLint threshold = glGetUniformLocation( shader , "threshold" );
  glUniform1f( threshold , _particleSizeThreshold );

  const auto pos = _camera->position( );
  glm::vec3 cameraPosition( pos[ 0 ] , pos[ 1 ] , pos[ 2 ] );

  _lastCameraPosition = cameraPosition;
  _psManager->particleSystem( )->updateRender( );
  _psManager->particleSystem( )->render( );

  _particlesShader->unuse( );
}

void OpenGLWidget::paintGL( void )
{
  std::chrono::time_point <std::chrono::system_clock> now =
    std::chrono::system_clock::now( );

  unsigned int elapsedMicroseconds =
    std::chrono::duration_cast< std::chrono::microseconds >
      ( now - _lastFrame ).count( );

  _lastFrame = now;

  _deltaTime = elapsedMicroseconds * 0.000001;

  _elapsedTimeRenderAcc += elapsedMicroseconds;

  _frameCount++;
  glDepthMask( GL_TRUE );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glDisable( GL_BLEND );
  glEnable( GL_DEPTH_TEST );
  glEnable( GL_CULL_FACE );

  _dynPathManager->processFinishedPaths( );
  _dynPathManager->processPendingSections( );
  _dynPathManager->processPendingSynapses( );

  if ( _paint )
  {
    _camera->anim( _deltaTime );

//    if( _psManager && _psManager->particleSystem( ))
    {

      if ( _psManager && _psManager->particleSystem( ) &&
           _elapsedTimeRenderAcc >= _renderPeriodMicroseconds )
      {
        //TODO
        const float delta = _elapsedTimeRenderAcc * 0.000001;

        _psManager->particleSystem( )->update(
          _dynamicMovement ? delta : 0.0f );
        _elapsedTimeRenderAcc = 0.0f;
      }

      glBindFramebuffer( GL_FRAMEBUFFER , _msaaFrameBuffer );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      glViewport( 0 , 0 , size( ).width( ) * SUPER_SAMPLING_SCALE ,
                  size( ).height( ) * SUPER_SAMPLING_SCALE );

      paintMorphologies( );
      paintParticles( );

      performMSAA( );
      glBindFramebuffer( GL_FRAMEBUFFER , defaultFramebufferObject( ));
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
      glViewport( 0 , 0 , width( ) , height( ));
      glDisable( GL_DEPTH_TEST );
      glDisable( GL_BLEND );
      glClear( GL_COLOR_BUFFER_BIT );

      _screenPlaneShader->use( );
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D , _textureColor );

      GLuint texID = glGetUniformLocation( _screenPlaneShader->program( ) ,
                                           "screenTexture" );
      glUniform1i( texID , 0 );

      glActiveTexture( GL_TEXTURE1 );
      glBindTexture( GL_TEXTURE_2D , _textureColor );

      GLint depthID = glGetUniformLocation( _screenPlaneShader->program( ) ,
                                            "depthTexture" );
      GLint nearID = glGetUniformLocation( _screenPlaneShader->program( ) ,
                                           "zNear" );
      GLint farID = glGetUniformLocation( _screenPlaneShader->program( ) ,
                                          "zFar" );

      glUniform1i( depthID , 1 );
      glUniform1f( nearID , _camera->camera( )->nearPlane( ));
      glUniform1f( farID , _camera->camera( )->farPlane( ));

      _oglFunctions->glBindVertexArray( _screenPlaneVao );

      glDrawElements( GL_TRIANGLES , 6 , GL_UNSIGNED_INT , 0 );

      update( );
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

  generateDepthTexture( w , h );
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
    case Qt::Key_R:
      auto system = _psManager->particleSystem( );
      auto old = system->renderer( );
      if ( dynamic_cast<prefr::OIGLRenderer*>(old) != nullptr )
      {
        system->renderer( new prefr::GLRenderer( _particlesShader ));
      }
      else
      {
        system->renderer( new prefr::OIGLRenderer( _particlesShader ));
      }
      system->renderer( )->enableAccumulativeMode( _alphaBlendingAccumulative );
      delete old;
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

const std::vector <nsol::MorphologySynapsePtr>&
OpenGLWidget::currentSynapses( void )
{
  return _domainManager->getSynapses( );
}

void OpenGLWidget::colorSynapsesPre( const syncopa::vec3& color )
{
  _colorSynapsesPre = color;

  syncopa::vec4 composedColor( color.x( ) , color.y( ) , color.z( ) ,
                               _alphaSynapsesPre );
  _psManager->colorSynapses( composedColor , syncopa::PRESYNAPTIC );
}

void OpenGLWidget::colorSynapsesPost( const syncopa::vec3& color )
{
  _colorSynapsesPost = color;

  syncopa::vec4 composedColor( color.x( ) , color.y( ) , color.z( ) ,
                               _alphaSynapsesPre );
  _psManager->colorSynapses( composedColor , syncopa::POSTSYNAPTIC );
}

void OpenGLWidget::colorPathsPre( const syncopa::vec3& color )
{
  _colorPathsPre = color;

  syncopa::vec4 composedColor( color.x( ) , color.y( ) , color.z( ) ,
                               _alphaPathsPre );
  _psManager->colorPaths( composedColor , syncopa::PRESYNAPTIC );
}

void OpenGLWidget::colorPathsPost( const syncopa::vec3& color )
{
  _colorPathsPost = color;

  syncopa::vec4 composedColor( color.x( ) , color.y( ) , color.z( ) ,
                               _alphaPathsPost );
  _psManager->colorPaths( composedColor , syncopa::POSTSYNAPTIC );
}

const syncopa::vec3& OpenGLWidget::colorSynapsesPre( void ) const
{
  return _colorSynapsesPre;
}

const syncopa::vec3& OpenGLWidget::colorSynapsesPost( void ) const
{
  return _colorSynapsesPost;
}

const syncopa::vec3& OpenGLWidget::colorPathsPre( void ) const
{
  return _colorPathsPre;
}

const syncopa::vec3& OpenGLWidget::colorPathsPost( void ) const
{
  return _colorPathsPost;
}

const syncopa::vec3& OpenGLWidget::colorSynapseMapPre( void ) const
{
  return _colorSynMapPre;
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

  _psManager->colorSynapseMap( colors );
}

tQColorVec OpenGLWidget::colorSynapseMap( void ) const
{
  return _colorSynMap;
}

void OpenGLWidget::alphaSynapsesPre( float transparency )
{
  _alphaSynapsesPre = transparency;

  syncopa::vec4 composedColor( _colorSynapsesPre.x( ) , _colorSynapsesPre.y( ) ,
                               _colorSynapsesPre.z( ) , _alphaSynapsesPre );

  _psManager->colorSynapses( composedColor , syncopa::PRESYNAPTIC );
}

void OpenGLWidget::alphaSynapsesPost( float transparency )
{
  _alphaSynapsesPost = transparency;

  syncopa::vec4 composedColor( _colorSynapsesPost.x( ) ,
                               _colorSynapsesPost.y( ) ,
                               _colorSynapsesPost.z( ) , _alphaSynapsesPost );

  _psManager->colorSynapses( composedColor , syncopa::POSTSYNAPTIC );
}

void OpenGLWidget::alphaPathsPre( float transparency )
{
  _alphaPathsPre = transparency;

  syncopa::vec4 composedColor( _colorPathsPre.x( ) , _colorPathsPre.y( ) ,
                               _colorPathsPre.z( ) , _alphaPathsPre );

  _psManager->colorPaths( composedColor , syncopa::PRESYNAPTIC );
}

void OpenGLWidget::alphaPathsPost( float transparency )
{
  _alphaPathsPost = transparency;

  syncopa::vec4 composedColor( _colorPathsPost.x( ) , _colorPathsPost.y( ) ,
                               _colorPathsPost.z( ) , _alphaPathsPost );

  _psManager->colorPaths( composedColor , syncopa::POSTSYNAPTIC );
}

void
OpenGLWidget::alphaSynapseMap( const tQColorVec& colors , float transparency )
{
  _alphaSynapsesMap = transparency;

  colorSynapseMap( colors );
}

float OpenGLWidget::alphaSynapsesPre( void ) const
{
  return _alphaSynapsesPre;
}

float OpenGLWidget::alphaSynapsesPost( void ) const
{
  return _alphaSynapsesPost;
}

float OpenGLWidget::alphaPathsPre( void ) const
{
  return _alphaPathsPre;
}

float OpenGLWidget::alphaPathsPost( void ) const
{
  return _alphaPathsPost;
}

float OpenGLWidget::alphaSynapsesMap( void ) const
{
  return _alphaSynapsesMap;
}

void OpenGLWidget::updatePathsVisibility( void )
{
  _psManager->showPaths( _mode == syncopa::PATHS && _showPathsPre ,
                         PRESYNAPTIC );
  _psManager->showPaths( _mode == syncopa::PATHS && _showPathsPost ,
                         POSTSYNAPTIC );
}

void OpenGLWidget::updateSynapsesVisibility( void )
{
  _psManager->showSynapses( _showSynapsesPre , PRESYNAPTIC );
  _psManager->showSynapses( _showSynapsesPost , POSTSYNAPTIC );
}

void OpenGLWidget::showSynapsesPre( bool state )
{
  _showSynapsesPre = state;
  _psManager->showSynapses( state , syncopa::PRESYNAPTIC );
}

void OpenGLWidget::showSynapsesPost( bool state )
{
  _showSynapsesPost = state;
  _psManager->showSynapses( state , syncopa::POSTSYNAPTIC );
}

void OpenGLWidget::showPathsPre( bool state )
{
  _showPathsPre = state;
  _psManager->showPaths( state , syncopa::PRESYNAPTIC );
}

void OpenGLWidget::showPathsPost( bool state )
{
  _showPathsPost = state;
  _psManager->showPaths( state , syncopa::POSTSYNAPTIC );
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
  _dynPathManager->createRootSources( );
  _dynamicMovement = true;
  _dynamicActive = true;
}

void OpenGLWidget::toggleDynamicMovement( void )
{
  _dynamicMovement = !_dynamicMovement;
}

void OpenGLWidget::stopDynamic( void )
{
  _dynPathManager->clear( );
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
  _alphaBlendingAccumulative = alphaAccumulative;
  auto system = _psManager->particleSystem( );
  system->renderer( )->enableAccumulativeMode( alphaAccumulative );
}

void OpenGLWidget::mode( TMode mode_ )
{
  _mode = mode_;

  _dynPathManager->clear( );
  _alphaBlendingAccumulative = (_mode == PATHS);

  setupSynapses( );
  setupPaths( );

  updateSynapsesVisibility( );
  updatePathsVisibility( );

  home( );
}

TMode OpenGLWidget::mode( void ) const
{
  return _mode;
}

void OpenGLWidget::sizeSynapsesPre( float newSize )
{
  _psManager->sizeSynapses( newSize , PRESYNAPTIC );
}

void OpenGLWidget::sizeSynapsesPost( float newSize )
{
  _psManager->sizeSynapses( newSize , POSTSYNAPTIC );
}

void OpenGLWidget::sizePathsPre( float newSize )
{
  _psManager->sizePaths( newSize , PRESYNAPTIC );
}

void OpenGLWidget::sizePathsPost( float newSize )
{
  _psManager->sizePaths( newSize , POSTSYNAPTIC );
}

void OpenGLWidget::sizeSynapseMap( float newSize )
{
  _psManager->sizeSynapsesMap( newSize , ALL_CONNECTIONS );
}

void OpenGLWidget::sizeDynamic( float newSize )
{
  _psManager->sizeSynapses( newSize , PRESYNAPTIC );
}

float OpenGLWidget::sizeSynapsesPre( void ) const
{
  return _psManager->sizeSynapses( PRESYNAPTIC );
}

float OpenGLWidget::sizeSynapsesPost( void ) const
{
  return _psManager->sizeSynapses( POSTSYNAPTIC );
}

float OpenGLWidget::sizePathsPre( void ) const
{
  return _psManager->sizePaths( PRESYNAPTIC );
}

float OpenGLWidget::sizePathsPost( void ) const
{
  return _psManager->sizePaths( POSTSYNAPTIC );
}

float OpenGLWidget::sizeSynapseMap( void ) const
{
  return _psManager->sizeSynapseMap( );
}

float OpenGLWidget::sizeDynamic( void ) const
{
  return _psManager->sizeDynamic( );
}

void OpenGLWidget::loadPostprocess( )
{
  makeCurrent( );

  _neuronScene->uploadMeshes( );
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

PSManager* OpenGLWidget::getPsManager( ) const
{
  return _psManager;
}
