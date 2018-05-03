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
, _idleUpdate( true )
, _paint( true )
, _currentClearColor( 20, 20, 20, 0 )
, _particlesShader( nullptr )
, _particleSystem( nullptr )
, _nlrenderer( nullptr )
, _scene( nullptr )
, _dataset( nullptr )
, _elapsedTimeRenderAcc( 0.0f )
, _alphaBlendingAccumulative( false )
{
  _camera = new reto::Camera( );

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

  _renderSpeed = 1.f;
}


OpenGLWidget::~OpenGLWidget( void )
{
  delete _camera;

  if( _particlesShader )
    delete _particlesShader;

  if( _particleSystem )
    delete _particleSystem;
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


void OpenGLWidget::createParticleSystem( unsigned int maxParticles, unsigned int maxEmitters )
{
  makeCurrent( );
  prefr::Config::init( );

  _particleSystem = new prefr::ParticleSystem( maxParticles );

  //TODO
  _particleSystem->parallel( true );

  unsigned int i = 0;
  std::vector< glm::vec3 >positions( maxEmitters );
  for( auto& pos : positions )
  {
    pos = glm::vec3( i * 10.f, 0.0f, 0.0f );
    i++;
  }

  _particlesShader = new reto::ShaderProgram( );
  _particlesShader->loadVertexShaderFromText( prefr::prefrVertexShader );
  _particlesShader->loadFragmentShaderFromText( prefr::prefrFragmentShader );
  _particlesShader->create( );
  _particlesShader->link( );

  prefr::Model* model = new prefr::Model( 5.0f, 10.0f );

  model->color.Insert( 0.0f, ( glm::vec4(0.1f, 0.1f, 0.1f, 0.5f)));

  model->velocity.Insert( 0.0f, 0.0f );

  model->size.Insert( 1.0f, 10.0f );

  _particleSystem->addModel( model );

  prefr::Updater* updater;

  updater = new prefr::Updater( );

  model->color.Insert( 0.0f, ( glm::vec4(0.f, 1.f, 0.f, 0.2)));
  model->color.Insert( 0.35f, ( glm::vec4(1, 0, 0, 0.2 )));
  model->color.Insert( 0.7f, ( glm::vec4(1.f, 1.f, 0, 0.2 )));
  model->color.Insert( 1.0f, ( glm::vec4(0, 0, 1.f, 0.2 )));

  model->velocity.Insert( 0.0f, 0.0f );
  model->velocity.Insert( 1.0f, 10.0f );

  model->size.Insert( 0.0f, 20.0f );
  model->size.Insert( 1.0f, 10.0f );


  prefr::Cluster* cluster;
  prefr::Source* source;
  prefr::SphereSampler* sampler = new prefr::SphereSampler( 3.0f );

  int partPerEmitter = maxParticles / maxEmitters;

  unsigned int start;

  glm::vec3 boundingBoxMin( std::numeric_limits< float >::max( ),
                            std::numeric_limits< float >::max( ),
                            std::numeric_limits< float >::max( ));
  glm::vec3 boundingBoxMax( std::numeric_limits< float >::min( ),
                            std::numeric_limits< float >::min( ),
                            std::numeric_limits< float >::min( ));

  i = 0;
  glm::vec3 cameraPivot;
  for ( auto position : positions )
  {
    cluster = new prefr::Cluster( );
    cluster->model( model );
    cluster->updater( updater );
    cluster->active( true );

    cameraPivot += position;
    ExpandBoundingBox( boundingBoxMin, boundingBoxMax, position );
    start = i * partPerEmitter;

    source = new prefr::Source( 0.3f, position );

    source->sampler( sampler );
    cluster->source( source );

    _particleSystem->addSource( source );
    _particleSystem->addCluster( cluster, start, partPerEmitter );

    i++;
  }

  cameraPivot /= i;

  _camera->pivot( Eigen::Vector3f( cameraPivot.x,
                                   cameraPivot.y,
                                   cameraPivot.z ));

  glm::vec3 center = ( boundingBoxMax + boundingBoxMin ) * 0.5f;
  float radius = glm::length( boundingBoxMax - center );

  _camera->targetPivotRadius( Eigen::Vector3f( center.x, center.y, center.z ), radius );


  prefr::Sorter* sorter = new prefr::Sorter( );

  prefr::GLRenderer* renderer = new prefr::GLRenderer( );

  _particleSystem->addUpdater( updater );
  _particleSystem->sorter( sorter );
  _particleSystem->renderer( renderer );

  _particleSystem->start();

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
  if( !_particleSystem )
    return;

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

  _particleSystem->updateCameraDistances( cameraPosition );

  _lastCameraPosition = cameraPosition;

  _particleSystem->updateRender( );
  _particleSystem->render( );

  _particlesShader->unuse( );

}

void OpenGLWidget::paintGL( void )
{
  std::chrono::time_point< std::chrono::system_clock > now =
      std::chrono::system_clock::now( );

  unsigned int elapsedMilliseconds =
      std::chrono::duration_cast< std::chrono::milliseconds >
        ( now - _lastFrame ).count( );

  _lastFrame = now;

  _deltaTime = elapsedMilliseconds * 0.001f;

  _elapsedTimeRenderAcc += _deltaTime;

  _frameCount++;
  glDepthMask(GL_TRUE);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  if ( _paint )
  {
    _camera->anim( );

    if ( _particleSystem )
    {

      if( _elapsedTimeRenderAcc >= _renderPeriod )
      {_particleSystem->update( 0.1f );
        _elapsedTimeRenderAcc = 0.0f;
      }

      paintMorphologies( );
//      paintParticles( );

    }
    glUseProgram( 0 );
    glFlush( );

  }

  #define FRAMES_PAINTED_TO_MEASURE_FPS 10
  if ( _frameCount == FRAMES_PAINTED_TO_MEASURE_FPS )
  {

    auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>( now - _then );
    _then = now;

    MainWindow* mainWindow = dynamic_cast< MainWindow* >( parent( ));
    if ( mainWindow )
    {

      unsigned int ellapsedMiliseconds = duration.count( );

      unsigned int fps = roundf( 1000.0f *
                                 float( FRAMES_PAINTED_TO_MEASURE_FPS ) /
                                 float( ellapsedMiliseconds ));

      if ( _showFps )
      {
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

  _scene = new synvis::Scene( _dataset );
  std::cout << "Generating meshes..." << std::endl;
  _scene->generateMeshes( );

  std::vector< unsigned int > gids;
  for( auto neuron : _dataset->neurons( ))
  {
    gids.push_back( neuron.first );
  }

  _renderConfig = _scene->getRender( gids );
  home( );
}

void OpenGLWidget::home( void )
{
  _scene->computeBoundingBox( );
  _camera->targetPivot( _scene->boundingBox( ).center( ));
  _camera->targetRadius( _scene->boundingBox( ).radius( ) /
                         sin( _camera->fov( )));
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
