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

#include <sstream>
#include <string>
#include <iostream>
#include <glm/glm.hpp>

#include <map>

#include "MainWindow.h"

#include "prefr/PrefrShaders.h"

using namespace synvis;

OpenGLWidget::OpenGLWidget( QWidget* parent_,
                            Qt::WindowFlags windowsFlags_ )
: QOpenGLWidget( parent_, windowsFlags_ )
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
, _currentClearColor( 20, 20, 20, 0 )
, _particlesShader( nullptr )
, _nlrenderer( nullptr )
, _dataset( nullptr )
, _neuronScene( nullptr )
, _psManager( nullptr )
, _pathFinder( nullptr )
, _particleSizeThreshold( 1.0f )
, _elapsedTimeRenderAcc( 0.0f )
, _alphaBlendingAccumulative( false )
, _colorSelectedPre( 1, 1, 1 )
, _colorSelectedPost( 0.8, 0.6, 0.6 )
, _colorRelated( 0.7, 0.5, 0 )
, _colorContext( 0.3, 0.3, 0.3 )
, _colorSynapsesPre( 1, 0, 0 )
, _colorSynapsesPost( 1, 0, 1 )
, _colorPathsPre( 0, 1, 0 )
, _colorPathsPost( 0, 0, 1 )
, _alphaSynapsesPre( 0.35f )
, _alphaSynapsesPost( 0.35f )
, _alphaPathsPre( 0.1f )
, _alphaPathsPost( 0.1f )
{
  _camera = new reto::Camera( );
  _camera->farPlane( 50000 );

  _cameraTimer = new QTimer( );
  _cameraTimer->start(( 1.0f / 60.f ) * 100 );
  connect( _cameraTimer, SIGNAL( timeout( )), this, SLOT( timerUpdate( )));

  _pathFinder = new synvis::PathFinder( );

  _lastCameraPosition = glm::vec3( 0, 0, 0 );

  _fpsLabel.setStyleSheet(
    "QLabel { background-color : #333;"
    "color : white;"
    "padding: 3px;"
    "margin: 10px;"
    " border-radius: 10px;}" );

  // This is needed to get key evends
  this->setFocusPolicy( Qt::WheelFocus );

  _maxFPS = 60.0f;
  _renderPeriod = 1.0f / _maxFPS;
  _renderPeriodMicroseconds = _renderPeriod * 1000000;

  _renderSpeed = 1.f;
}


OpenGLWidget::~OpenGLWidget( void )
{
  delete _camera;

  if( _particlesShader )
    delete _particlesShader;

  if( _psManager )
    delete _psManager;

  if( _pathFinder )
    delete _pathFinder;
}


void OpenGLWidget::initializeGL( void )
{
  initializeOpenGLFunctions( );

  glEnable( GL_DEPTH_TEST );
  glClearColor( float( _currentClearColor.red( )) / 255.0f,
                float( _currentClearColor.green( )) / 255.0f,
                float( _currentClearColor.blue( )) / 255.0f,
                float( _currentClearColor.alpha( )) / 255.0f );
  glPolygonMode( GL_FRONT_AND_BACK , GL_FILL );
  glEnable( GL_CULL_FACE );

  glLineWidth( 1.5 );

  _then = std::chrono::system_clock::now( );
  _lastFrame = std::chrono::system_clock::now( );


  QOpenGLWidget::initializeGL( );
  nlrender::Config::init( );
  _nlrenderer = new nlrender::Renderer( );
  _nlrenderer->tessCriteria( ) = nlrender::Renderer::LINEAR;
  _nlrenderer->lod( ) = 4;
  _nlrenderer->maximumDistance( ) = 10;

}


void ExpandBoundingBox( glm::vec3& minBounds,
                        glm::vec3& maxBounds,
                        const glm::vec3& position)
{
  for( unsigned int i = 0; i < 3; ++i )
  {
    minBounds[ i ] = std::min( minBounds[ i ], position[ i ] );
    maxBounds[ i ] = std::max( maxBounds[ i ], position[ i ] );
  }
}


void OpenGLWidget::loadBlueConfig( const std::string& blueConfigFilePath,
                                   const std::string& target )
{

  if( _dataset )
    delete _dataset;

  _dataset = new nsol::DataSet( );

  _pathFinder->dataset( _dataset );

  std::cout << "Loading data hierarchy..." << std::endl;
  _dataset->loadBlueConfigHierarchy( blueConfigFilePath, target );

  std::cout << "Loading morphologies..." << std::endl;
  _dataset->loadAllMorphologies( );

  std::cout << "Loading connectivity..." << std::endl;
  _dataset->loadBlueConfigConnectivityWithMorphologies( );

  _neuronScene = new synvis::NeuronScene( _dataset );
  std::cout << "Generating meshes..." << std::endl;
  _neuronScene->generateMeshes( );

  _neuronScene->color( vec3( 1, 1, 1 ), PRESYNAPTIC );
  _neuronScene->color( vec3( 0, 1, 1 ), POSTSYNAPTIC );


  std::set< unsigned int > gids;
  for( auto neuron : _dataset->neurons( ))
  {
    gids.insert( neuron.first );
  }

  _gidsAll = gids;

  createParticleSystem( );
  setupSynapses( gids );

  home( );
}

void OpenGLWidget::createParticleSystem( void )
{
  makeCurrent( );
  prefr::Config::init( );

  _particlesShader = new reto::ShaderProgram( );
  _particlesShader->loadVertexShaderFromText( prefr::prefrVertexShader );
  _particlesShader->loadFragmentShaderFromText( prefr::prefrFragmentShader );
  _particlesShader->create( );
  _particlesShader->link( );
  _particlesShader->autocatching( true );

  _psManager = new synvis::PSManager( );
  _psManager->init( 500000 );
//
//  //TODO add colors for different synapse tyoes
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

  _psManager->sizeSynapses( 8.0, PRESYNAPTIC );
  _psManager->sizeSynapses( 8.0, POSTSYNAPTIC );

  _psManager->sizePaths( 3.0, PRESYNAPTIC );
  _psManager->sizePaths( 3.0, POSTSYNAPTIC );


}

void OpenGLWidget::setupSynapses( const std::set< unsigned int >& gidsPre,
                                  const std::set< unsigned int >& gidsPost)
{
  auto& circuit = _dataset->circuit( );
  auto synapses = circuit.synapses( gidsPre,
                                    nsol::Circuit::PRESYNAPTICCONNECTIONS );

  std::vector< vec3 > positionsPre;
  std::vector< vec3 > positionsPost;
  positionsPre.reserve( synapses.size( ));
  positionsPre.reserve( synapses.size( ));

  _currentSynapses.clear( );

  unsigned int counter = 0;
  for( auto syn : synapses )
  {
    unsigned int postGid = syn->postSynapticNeuron( );
    if( gidsPost.size( ) > 0 && gidsPost.find( postGid ) == gidsPost.end( ))
      continue;

    auto morphoSyn = dynamic_cast< nsol::MorphologySynapsePtr >( syn );

    positionsPre.push_back( morphoSyn->preSynapticSurfacePosition( ));
    positionsPost.push_back( morphoSyn->postSynapticSurfacePosition( ));

    _currentSynapses.push_back( morphoSyn );

    ++counter;
  }

  std::cout << " Loaded " << counter << " synapses." << std::endl;

  _psManager->setupSynapses( positionsPre, PRESYNAPTIC );
  _psManager->setupSynapses( positionsPost, POSTSYNAPTIC );
}

void OpenGLWidget::setupPaths( const std::set< unsigned int >& gidsPre,
                               const std::set< unsigned int >& gidsPost )
{

  float distance = _psManager->sizePaths( ) * _particleSizeThreshold * 0.5;

  // TODO FIX MULTIPLE PRESYNAPTIC SELECTION
  unsigned int gidPre = *gidsPre.begin( );
  auto points =
      _pathFinder->getAllPathsPoints( gidPre, gidsPost, distance, PRESYNAPTIC );

  _psManager->setupPath( points, PRESYNAPTIC );

  points = _pathFinder->getAllPathsPoints( gidPre, gidsPost, distance, POSTSYNAPTIC );

  _psManager->setupPath( points, POSTSYNAPTIC );
}

void OpenGLWidget::home( void )
{
//  _neuronScene->computeBoundingBox( );
//  _camera->targetPivot( _neuronScene->boundingBox( ).center( ));
//  _camera->targetRadius( _neuronScene->boundingBox( ).radius( ) /
//                         sin( _camera->fov( )));
  _camera->targetPivot( _psManager->boundingBox( ).center( ));
  _camera->targetRadius( _psManager->boundingBox( ).radius( ) /
                         sin( _camera->fov( )));

}

void OpenGLWidget::clear( void )
{
  _neuronsSelectedPre = synvis::TRenderMorpho( );
  _neuronsSelectedPost = synvis::TRenderMorpho( );
  _neuronsRelated = _neuronScene->getRender( _gidsAll );
  _neuronsContext = synvis::TRenderMorpho( );

  _psManager->clear( );
}

void setColor( synvis::TRenderMorpho& renderConfig, const vec3& color )
{
  for( auto& c : std::get< synvis::COLOR >( renderConfig ))
    c = color;
}

void OpenGLWidget::setupNeuronMorphologies( void )
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

void OpenGLWidget::selectPresynapticNeuron( unsigned int gid )
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


//  _gidsOther.erase( _gidsSelectedPre.begin( ), _gidsSelectedPre.end( ));
//  _gidsOther.erase( _gidsSelectedPost.begin( ), _gidsSelectedPost.end( ));
//  _gidsOther.erase( _gidsRelated.begin( ), _gidsRelated.end( ));

  setupNeuronMorphologies( );
//  std::vector< unsigned int > gidsvPre = { gid };
//  std::vector< unsigned int > gidsvPost( gidsPost.begin( ), gidsPost.end( ));

  setupSynapses( _gidsSelectedPre );
  setupPaths( _gidsSelectedPre, _gidsRelated );

  home( );
}

void OpenGLWidget::selectPostsynapticNeuron( const std::vector< unsigned int >& gidsv )
{
  _gidsSelectedPost = std::set< unsigned int >( gidsv.begin( ), gidsv.end( ));

  unsigned int gidPre = *_gidsSelectedPre.begin( );
  _gidsRelated = _pathFinder->connectedTo( gidPre );

  for( auto gid : _gidsSelectedPost )
    _gidsRelated.erase( gid );
//  _gidsRelated.erase( _gidsSelectedPost.begin( ), _gidsSelectedPost.end( ));

  _gidsOther.clear( );

  setupNeuronMorphologies( );

//  std::set< unsigned int > gidsPre = { _gidsSelectedPre };
  setupSynapses( _gidsSelectedPre, _gidsSelectedPost );
  setupPaths( _gidsSelectedPre, _gidsSelectedPost );

  home( );

}


void OpenGLWidget::paintMorphologies( void )
{

  Eigen::Matrix4f projection( _camera->projectionMatrix( ));
  _nlrenderer->projectionMatrix( ) = projection;
  Eigen::Matrix4f view( _camera->viewMatrix( ));
  _nlrenderer->viewMatrix( ) = view;

//  std::cout << "Selected PRE " << std::get< synvis::MESH >( _neuronsSelectedPre ).size( )
//            << " == " << std::get< synvis::MATRIX >( _neuronsSelectedPre ).size( )
//            << " == " << std::get< synvis::COLOR >( _neuronsSelectedPre ).size( )
//            << std::endl;


  // Render selected neurons with full morphology
  _nlrenderer->render( std::get< synvis::MESH >( _neuronsSelectedPre ),
                       std::get< synvis::MATRIX >( _neuronsSelectedPre ),
                       std::get< synvis::COLOR >( _neuronsSelectedPre ));


//  std::cout << "Selected POST " << std::get< synvis::MESH >( _neuronsSelectedPost ).size( )
//            << " == " << std::get< synvis::MATRIX >( _neuronsSelectedPost ).size( )
//            << " == " << std::get< synvis::COLOR >( _neuronsSelectedPost ).size( )
//            << std::endl;

  _nlrenderer->render( std::get< synvis::MESH >( _neuronsSelectedPost ),
                       std::get< synvis::MATRIX >( _neuronsSelectedPost ),
                       std::get< synvis::COLOR >( _neuronsSelectedPost ));


//  std::cout << "Related " << std::get< synvis::MESH >( _neuronsRelated ).size( )
//            << " == " << std::get< synvis::MATRIX >( _neuronsRelated ).size( )
//            << " == " << std::get< synvis::COLOR >( _neuronsRelated ).size( )
//            << std::endl;

  _nlrenderer->render( std::get< synvis::MESH >( _neuronsRelated ),
                       std::get< synvis::MATRIX >( _neuronsRelated ),
                       std::get< synvis::COLOR >( _neuronsRelated ), true, false );


//  std::cout << "Context " << std::get< synvis::MESH >( _neuronsContext ).size( )
//            << " == " << std::get< synvis::MATRIX >( _neuronsContext ).size( )
//            << " == " << std::get< synvis::COLOR >( _neuronsContext ).size( )
//            << std::endl;

  _nlrenderer->render( std::get< synvis::MESH >( _neuronsContext ),
                       std::get< synvis::MATRIX >( _neuronsContext ),
                       std::get< synvis::COLOR >( _neuronsContext ), true, false );
}


void OpenGLWidget::paintParticles( void )
{

  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  if( _alphaBlendingAccumulative )
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
  else
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


  glFrontFace(GL_CCW);

  _particlesShader->use( );
      // unsigned int shader;
      // shader = _particlesShader->getID();
  unsigned int shader;
  shader = _particlesShader->program( );

  unsigned int uModelViewProjM;
  unsigned int cameraUp;
  unsigned int cameraRight;
  unsigned int threshold;

//  _particlesShader->sendUniform4m( "modelViewProjM" )
  uModelViewProjM = glGetUniformLocation( shader, "modelViewProjM" );
  glUniformMatrix4fv( uModelViewProjM, 1, GL_FALSE,
                     _camera->viewProjectionMatrix( ));

  cameraUp = glGetUniformLocation( shader, "cameraUp" );
  cameraRight = glGetUniformLocation( shader, "cameraRight" );

  threshold = glGetUniformLocation( shader, "threshold" );

  float* viewM = _camera->viewMatrix( );

  glUniform3f( cameraUp, viewM[1], viewM[5], viewM[9] );
  glUniform3f( cameraRight, viewM[0], viewM[4], viewM[8] );
  glUniform1f( threshold, _particleSizeThreshold );


  glm::vec3 cameraPosition ( _camera->position( )[ 0 ],
                             _camera->position( )[ 1 ],
                             _camera->position( )[ 2 ] );

//  if( cameraPosition != _lastCameraPosition )
//    std::cout << "Camera: " << cameraPosition.x
//              << " " << cameraPosition.y
//              << " " << cameraPosition.z
//              << std::endl;

  _psManager->particleSystem( )->updateCameraDistances( cameraPosition );

  _lastCameraPosition = cameraPosition;

  _psManager->particleSystem( )->updateRender( );
  _psManager->particleSystem( )->render( );

  _particlesShader->unuse( );

}

void OpenGLWidget::paintGL( void )
{
//  makeCurrent( );
  std::chrono::time_point< std::chrono::system_clock > now =
           std::chrono::system_clock::now( );

  unsigned int elapsedMicroseconds =
      std::chrono::duration_cast< std::chrono::microseconds >
  ( now - _lastFrame ).count( );

  _lastFrame = now;

  _deltaTime = elapsedMicroseconds * 0.000001;

  _elapsedTimeRenderAcc += elapsedMicroseconds;

  _frameCount++;
  glDepthMask(GL_TRUE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  if ( _paint )
  {
    _camera->anim( );

//    if( _psManager && _psManager->particleSystem( ))
    {

      if( _psManager && _psManager->particleSystem( ) &&
          _elapsedTimeRenderAcc >= _renderPeriodMicroseconds )
      {
        _psManager->particleSystem( )->update( 0.1 );
        _elapsedTimeRenderAcc = 0.0f;
      }

      paintMorphologies( );
      paintParticles( );

    }

    glUseProgram( 0 );
    glFlush( );

  }

  #define FRAMES_PAINTED_TO_MEASURE_FPS 30
  if ( _frameCount == FRAMES_PAINTED_TO_MEASURE_FPS )
  {

    auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>( now - _then );
    _then = now;

    MainWindow* mainWindow = dynamic_cast< MainWindow* >( parent( ));
    if ( mainWindow )
    {

      if ( _showFps )
      {
        unsigned int ellapsedMiliseconds = duration.count( );

        unsigned int fps = roundf( 1000.0f *
                                 float( FRAMES_PAINTED_TO_MEASURE_FPS ) /
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

void OpenGLWidget::timerUpdate( void )
{
  if( _camera->anim( ))
    this->update( );
}


void OpenGLWidget::idleUpdate( bool idleUpdate_ )
{
  _idleUpdate = idleUpdate_;
}

void OpenGLWidget::resizeGL( int w , int h )
{
  _camera->ratio((( double ) w ) / h );
  glViewport( 0, 0, w, h );
}


void OpenGLWidget::mousePressEvent( QMouseEvent* event_ )
{

  if ( event_->button( ) == Qt::LeftButton )
  {
    _rotation = true;
    _mouseX = event_->x( );
    _mouseY = event_->y( );
  }

  if ( event_->button( ) ==  Qt::MidButton )
  {
    _translation = true;
    _mouseX = event_->x( );
    _mouseY = event_->y( );
  }

  update( );

}

void OpenGLWidget::mouseReleaseEvent( QMouseEvent* event_ )
{
  if ( event_->button( ) == Qt::LeftButton)
  {
    _rotation = false;
  }

  if ( event_->button( ) ==  Qt::MidButton )
  {
    _translation = false;
  }

  update( );

}

void OpenGLWidget::mouseMoveEvent( QMouseEvent* event_ )
{
  if( _rotation )
  {
    _camera->localRotation( -( _mouseX - event_->x( )) * 0.01,
                          ( _mouseY - event_->y( )) * 0.01 );
    _mouseX = event_->x( );
    _mouseY = event_->y( );
  }
  if( _translation )
  {
    _mouseX = event_->x( );
    _mouseY = event_->y( );
  }

  this->update( );
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
    _camera->pivot( Eigen::Vector3f( 0.0f, 0.0f, 0.0f ));
    _camera->radius( 1000.0f );
    _camera->rotation( 0.0f, 0.0f );
    update( );
    break;
  }
}


void OpenGLWidget::changeClearColor( void )
{
  QColor color =
    QColorDialog::getColor( _currentClearColor, parentWidget( ),
                            "Choose new background color",
                            QColorDialog::DontUseNativeDialog);

  if ( color.isValid( ))
  {
    _currentClearColor = color;

    makeCurrent( );
    glClearColor( float( _currentClearColor.red( )) / 255.0f,
                  float( _currentClearColor.green( )) / 255.0f,
                  float( _currentClearColor.blue( )) / 255.0f,
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

const std::vector< nsol::MorphologySynapsePtr >& OpenGLWidget::currentSynapses( void )
{
  return _currentSynapses;
}


void OpenGLWidget::colorSelectedPre( const synvis::vec3& color )
{
  _colorSelectedPre = color;

  setColor( _neuronsSelectedPre, _colorSelectedPre );
}

void OpenGLWidget::colorSelectedPost( const synvis::vec3& color )
{
  _colorSelectedPost = color;

  setColor( _neuronsSelectedPost, _colorSelectedPost );
}

void OpenGLWidget::colorRelated( const synvis::vec3& color )
{
  _colorRelated = color;

  setColor( _neuronsRelated, _colorRelated );
}

void OpenGLWidget::colorContext( const synvis::vec3& color )
{
  _colorContext = color;

  setColor( _neuronsContext, _colorContext );
}

void OpenGLWidget::colorSynapsesPre( const synvis::vec3& color )
{
  _colorSynapsesPre = color;

  synvis::vec4 composedColor( color.x( ), color.y( ), color.z( ), _alphaSynapsesPre );
  _psManager->colorSynapses( composedColor, synvis::PRESYNAPTIC );
}

void OpenGLWidget::colorSynapsesPost( const synvis::vec3& color )
{
  _colorSynapsesPost = color;

  synvis::vec4 composedColor( color.x( ), color.y( ), color.z( ), _alphaSynapsesPre );
  _psManager->colorSynapses( composedColor, synvis::POSTSYNAPTIC );
}

void OpenGLWidget::colorPathsPre( const synvis::vec3& color )
{
  _colorPathsPre = color;

  synvis::vec4 composedColor( color.x( ), color.y( ), color.z( ), _alphaPathsPre );
  _psManager->colorPaths( composedColor, synvis::PRESYNAPTIC );
}

void OpenGLWidget::colorPathsPost( const synvis::vec3& color )
{
  _colorPathsPost = color;

  synvis::vec4 composedColor( color.x( ), color.y( ), color.z( ), _alphaPathsPost );
  _psManager->colorPaths( composedColor, synvis::POSTSYNAPTIC );
}


const synvis::vec3& OpenGLWidget::colorSelectedPre( void ) const
{
  return _colorSelectedPre;
}

const synvis::vec3& OpenGLWidget::colorSelectedPost( void ) const
{
  return _colorSelectedPost;
}

const synvis::vec3& OpenGLWidget::colorRelated( void ) const
{
  return _colorRelated;
}

const synvis::vec3& OpenGLWidget::colorContext( void ) const
{
  return _colorContext;
}

const synvis::vec3& OpenGLWidget::colorSynapsesPre( void ) const
{
//  synvis::vec4 result = _psManager->colorSynapses( synvis::PRESYNAPTIC );
//
//  return synvis::vec3( result.block< 3, 1 >( 0, 0 ));
  return _colorSynapsesPre;
}

const synvis::vec3& OpenGLWidget::colorSynapsesPost( void ) const
{
//  synvis::vec4 result = _psManager->colorSynapses( synvis::POSTSYNAPTIC );
//
//  return synvis::vec3( result.block< 3, 1 >( 0, 0 ));
  return _colorSynapsesPost;
}

const synvis::vec3& OpenGLWidget::colorPathsPre( void ) const
{
  return _colorPathsPre;
}

const synvis::vec3& OpenGLWidget::colorPathsPost( void ) const
{
  return _colorPathsPost;
}

//bool OpenGLWidget::showDialog( QColor& current, const std::string& message )
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
