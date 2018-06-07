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
#include <nsol/nsol.h>

#include "NeuronScene.h"
#include "PSManager.h"
#include "PathFinder.h"

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
  void clear( void );

  void idleUpdate( bool idleUpdate_ = true );

  nsol::DataSet* dataset( void );

public slots:

  void timerUpdate( void );

  void changeClearColor( void );
  void toggleUpdateOnIdle( void );
  void toggleShowFPS( void );

  void selectPresynapticNeuron( unsigned int gid );
  void selectPostsynapticNeuron( const std::vector< unsigned int >& gid );

  const std::vector< nsol::MorphologySynapsePtr >& currentSynapses( void );

protected:

  virtual void initializeGL( void );
  virtual void paintGL( void );
  virtual void resizeGL( int w, int h );

  virtual void mousePressEvent( QMouseEvent* event );
  virtual void mouseReleaseEvent( QMouseEvent* event );
  virtual void wheelEvent( QWheelEvent* event );
  virtual void mouseMoveEvent( QMouseEvent* event );
  virtual void keyPressEvent( QKeyEvent* event );

  void setupNeuronMorphologies( void );

  void setupSynapses( const std::set< unsigned int >& gidsPre,
                      const std::set< unsigned int >& gidsPost = std::set< unsigned int >( ));
  void setupPaths( const std::set< unsigned int >& gidsPre,
                   const std::set< unsigned int >& gidsPost );

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
  synvis::NeuronScene* _neuronScene;
  synvis::PSManager* _psManager;
  synvis::PathFinder* _pathFinder;

  float _particleSizeThreshold;

  QTimer* _cameraTimer;

  synvis::TRenderMorpho _neuronsSelectedPre;
  synvis::TRenderMorpho _neuronsSelectedPost;
  synvis::TRenderMorpho _neuronsRelated;
  synvis::TRenderMorpho _neuronsContext;

  float _renderSpeed;
  float _maxFPS;
  float _renderPeriod;
  float _elapsedTimeRenderAcc;
  float _renderPeriodMicroseconds;
  bool _alphaBlendingAccumulative;

  std::set< unsigned int > _gidsAll;
  std::set< unsigned int > _gidsSelectedPre;
  std::set< unsigned int > _gidsSelectedPost;
  std::set< unsigned int > _gidsRelated;
  std::set< unsigned int > _gidsOther;

  synvis::vec3 _colorSelectedPre;
  synvis::vec3 _colorSelectedPost;
  synvis::vec3 _colorRelated;
  synvis::vec3 _colorContext;

  std::vector< nsol::MorphologySynapsePtr > _currentSynapses;
};

#endif // __VISIMPL__OPENGLWIDGET__
