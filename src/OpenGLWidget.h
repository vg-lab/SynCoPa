/*
 * @file  OpenGLWidget.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#ifndef __VISIMPL__OPENGLWIDGET__
#define __VISIMPL__OPENGLWIDGET__

#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QLabel>
#include <chrono>
#include <unordered_set>
#include <unordered_map>

#define PREFR_SKIP_GLEW_INCLUDE 1
#define NEUROLOTS_SKIP_GLEW_INCLUDE 1

#define SIM_SLIDER_UPDATE_PERIOD 0.5f

#include <nlrender/nlrender.h>
#include <prefr/prefr.h>
#include <reto/reto.h>

#include "Scene.h"
#include "PSManager.h"


class OpenGLWidget
  : public QOpenGLWidget
  , public QOpenGLFunctions
{

  Q_OBJECT;

public:

  OpenGLWidget( QWidget* parent = 0,
                Qt::WindowFlags windowFlags = 0 );

  ~OpenGLWidget( void );

  void createParticleSystem( void );

  void loadBlueConfig( const std::string& blueConfigFilePath,
                       const std::string& target );

  void home( void );

  void idleUpdate( bool idleUpdate_ = true );

public slots:

  void timerUpdate( void );

  void changeClearColor( void );
  void toggleUpdateOnIdle( void );
  void toggleShowFPS( void );

protected:

  virtual void initializeGL( void );
  virtual void paintGL( void );
  virtual void resizeGL( int w, int h );

  virtual void mousePressEvent( QMouseEvent* event );
  virtual void mouseReleaseEvent( QMouseEvent* event );
  virtual void wheelEvent( QWheelEvent* event );
  virtual void mouseMoveEvent( QMouseEvent* event );
  virtual void keyPressEvent( QKeyEvent* event );

  void setupSynapses( void );

  void paintParticles( void );
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

  reto::ShaderProgram* _particlesShader;
//  prefr::ParticleSystem* _particleSystem;
  nlrender::Renderer* _nlrenderer;

  nsol::DataSet* _dataset;
  synvis::Scene* _scene;
  synvis::PSManager* _psManager;

  QTimer* _cameraTimer;

  synvis::TRenderMorpho _renderConfig;

  float _renderSpeed;
  float _maxFPS;
  float _renderPeriod;
  float _elapsedTimeRenderAcc;
  bool _alphaBlendingAccumulative;
};

#endif // __VISIMPL__OPENGLWIDGET__
