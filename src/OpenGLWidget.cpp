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
, _elapsedTimeRenderAcc( 0.0f )
, _alphaBlendingAccumulative( false )
{
  _camera = new reto::Camera( );
  _camera->farPlane( 50000 );

  _cameraTimer = new QTimer( );
  _cameraTimer->start(( 1.0f / 60.f ) * 100 );
  connect( _cameraTimer, SIGNAL( timeout( )), this, SLOT( timerUpdate( )));

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

  std::cout << "Loading data hierarchy..." << std::endl;
  _dataset->loadBlueConfigHierarchy( blueConfigFilePath, target );

  std::cout << "Loading morphologies..." << std::endl;
  _dataset->loadAllMorphologies( );

  std::cout << "Loading connectivity..." << std::endl;
  _dataset->loadBlueConfigConnectivityWithMorphologies( );

  _neuronScene = new synvis::NeuronScene( _dataset );
  std::cout << "Generating meshes..." << std::endl;
  _neuronScene->generateMeshes( );

  std::set< unsigned int > gids;
  for( auto neuron : _dataset->neurons( ))
  {
    gids.insert( neuron.first );
  }

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
  _psManager->colorSynapses( vec4( 1, 0, 0, 0.25 ), PRESYNAPTIC );
  _psManager->colorSynapses( vec4( 1, 0, 0, 0.25 ), POSTSYNAPTIC );
  _psManager->sizeSynapses( 10.0, PRESYNAPTIC );
  _psManager->sizeSynapses( 10.0, POSTSYNAPTIC );

}

void OpenGLWidget::setupSynapses( const std::set< unsigned int >& gids )
{
  auto& circuit = _dataset->circuit( );
  auto synapses = circuit.synapses( *gids.begin( ), nsol::Circuit::PRESYNAPTICCONNECTIONS );

  std::cout << " Loaded " << synapses.size( ) << " synapses." << std::endl;

  std::vector< vec3 > positionsPre;
  std::vector< vec3 > positionsPost;
  positionsPre.reserve( synapses.size( ));
  positionsPre.reserve( synapses.size( ));

  unsigned int counter = 0;
  for( auto syn : synapses )
  {
    auto morphoSyn = dynamic_cast< nsol::MorphologySynapsePtr >( syn );

//    vec3 position = morphoSyn->preSynapticSurfacePosition( );

//    std::cout << "Syn " << morphoSyn << " " << morphoSyn->preSynapticNeuron( )
//              << " " << morphoSyn->postSynapticNeuron( );
//    std::cout << " (" << position.x( ) << " " << position.y( ) << " " << position.z( ) << ")";

    positionsPre.push_back( morphoSyn->preSynapticSurfacePosition( ));
    positionsPost.push_back( morphoSyn->postSynapticSurfacePosition( ));

    ++counter;
  }
  std::cout << std::endl;

  _psManager->setupSynapses( positionsPre, PRESYNAPTIC );
//  _psManager->setupSynapses( positionsPost, POSTSYNAPTIC );
}

void OpenGLWidget::home( void )
{
  _neuronScene->computeBoundingBox( );
  _camera->targetPivot( _neuronScene->boundingBox( ).center( ));
  _camera->targetRadius( _neuronScene->boundingBox( ).radius( ) /
                         sin( _camera->fov( )));
//  _camera->targetPivot( _psManager->boundingBox( ).center( ));
//  _camera->targetRadius( _psManager->boundingBox( ).radius( ) /
//                         sin( _camera->fov( )));

}

void OpenGLWidget::clear( void )
{
  _psManager->clear( );
}

void OpenGLWidget::selectPresynapticNeuron( unsigned int gid )
{
  std::set< unsigned int > gids = { gid };
  std::vector< unsigned int > gidsv = { gid };
  setupSynapses( gids );

  _renderConfig = _neuronScene->getRender( gidsv );


}



void OpenGLWidget::paintMorphologies( void )
{

  Eigen::Matrix4f projection( _camera->projectionMatrix( ));
  _nlrenderer->projectionMatrix( ) = projection;
  Eigen::Matrix4f view( _camera->viewMatrix( ));
  _nlrenderer->viewMatrix( ) = view;

  _nlrenderer->render( std::get< synvis::MESH >( _renderConfig ),
                       std::get< synvis::MATRIX >( _renderConfig ),
                       std::get< synvis::COLOR >( _renderConfig ));
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

  unsigned int uModelViewProjM, cameraUp, cameraRight;

//  _particlesShader->sendUniform4m( "modelViewProjM" )
  uModelViewProjM = glGetUniformLocation( shader, "modelViewProjM" );
  glUniformMatrix4fv( uModelViewProjM, 1, GL_FALSE,
                     _camera->viewProjectionMatrix( ));

  cameraUp = glGetUniformLocation( shader, "cameraUp" );
  cameraRight = glGetUniformLocation( shader, "cameraRight" );

  float* viewM = _camera->viewMatrix( );

  glUniform3f( cameraUp, viewM[1], viewM[5], viewM[9] );
  glUniform3f( cameraRight, viewM[0], viewM[4], viewM[8] );

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
//  if( _camera->anim( ))
//    this->update( );
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
