/*
 * @file  OpenGLWidget.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#ifndef __SYNCOPA__OPENGLWIDGET__
#define __SYNCOPA__OPENGLWIDGET__

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_0_Core>
#include <QLabel>
#include <chrono>
#include <unordered_set>
#include <unordered_map>

#define SYNCOPA_SKIP_GLEW_INCLUDE 1
#define PREFR_SKIP_GLEW_INCLUDE 1
#define NEUROLOTS_SKIP_GLEW_INCLUDE 1
#define RETO_SKIP_GLEW_INCLUDE 1


#include <nlrender/nlrender.h>
#include <prefr/prefr.h>
#include <reto/reto.h>
#include <nlgeometry/nlgeometry.h>



#define SIM_SLIDER_UPDATE_PERIOD 0.5f

//#include "types.h"

//#include <nsol/nsol.h>
//
#include "DomainManager.h"
#include "NeuronScene.h"
//#include "PSManager.h"
//#include "PathFinder.h"
//#include "DynamicPathManager.h"

class OpenGLWidget
  : public QOpenGLWidget
  , public QOpenGLFunctions
{

  Q_OBJECT;

public:

  OpenGLWidget( QWidget* parent = 0,
                Qt::WindowFlags windowFlags = 0 );

  ~OpenGLWidget( void );

  void init( DomainManager* domainManager );

  void home( nlgeometry::AxisAlignedBoundingBox boundingBox );

  void idleUpdate( bool idleUpdate_ = true );

  nlrender::Renderer* nlRenderer( void ) const;

public slots:

  void timerUpdate( void );

  void changeClearColor( void );
  void toggleUpdateOnIdle( void );
  void toggleShowFPS( void );


protected:

  virtual void initializeGL( void );
  virtual void paintGL( void );
  virtual void resizeGL( int w, int h );

//  bool showDialog( QColor& current, const std::string& message = "" );

  virtual void mousePressEvent( QMouseEvent* event );
  virtual void mouseReleaseEvent( QMouseEvent* event );
  virtual void wheelEvent( QWheelEvent* event );
  virtual void mouseMoveEvent( QMouseEvent* event );
  virtual void keyPressEvent( QKeyEvent* event );

  void paintParticles( void );

  void initRenderToTexture( void );
  void generateDepthTexture( int width_, int height_ );
  void performMSAA( void );
  void paintMorphologies( void );

  QLabel _fpsLabel;
  bool _showFps;

  reto::Camera* _camera;
  glm::vec3 _lastCameraPosition;

  unsigned int _frameCount;
  float _deltaTime;

  int _mouseX, _mouseY;
  bool _rotation, _translation;

  bool _idleUpdate;
  bool _paint;

  QColor _currentClearColor;

  std::chrono::time_point< std::chrono::system_clock > _then;
  std::chrono::time_point< std::chrono::system_clock > _lastFrame;

  syncopa::DomainManager* _domainManager;
  reto::ShaderProgram* _particlesShader;
  prefr::ParticleSystem* _particleSystem;
  nlrender::Renderer* _nlrenderer;

  syncopa::NeuronScene* _neuronScene;

//  QTimer* _cameraTimer;

  float _renderSpeed;
  float _maxFPS;
  float _renderPeriod;
  float _elapsedTimeRenderAcc;
  float _renderPeriodMicroseconds;
  bool _alphaBlendingAccumulative;

  // Render to texture
  QOpenGLFunctions_4_0_Core* _oglFunctions;

  reto::ShaderProgram* _screenPlaneShader;

  unsigned int _screenPlaneVao;

  unsigned int _depthFrameBuffer;
  unsigned int _textureColor;
  unsigned int _textureDepth;

  unsigned int _msaaSamples;
  unsigned int _msaaFrameBuffer;
  unsigned int _msaaTextureColor;
  unsigned int _msaaTextureDepth;

  int _currentWidth;
  int _currentHeight;

  Eigen::Vector2f _inverseResolution;

};

#endif // __VISIMPL__OPENGLWIDGET__
