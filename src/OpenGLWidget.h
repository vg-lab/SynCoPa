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
#include <QOpenGLFunctions_4_0_Core>
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

  void colorSelectedPre( const syncopa::vec3& color );
  void colorSelectedPost( const syncopa::vec3& color );
  void colorRelated( const syncopa::vec3& color );
  void colorContext( const syncopa::vec3& color );
  void colorSynapsesPre( const syncopa::vec3& color );
  void colorSynapsesPost( const syncopa::vec3& color );
  void colorPathsPre( const syncopa::vec3& color );
  void colorPathsPost( const syncopa::vec3& color );


  const syncopa::vec3& colorSelectedPre( void ) const;
  const syncopa::vec3& colorSelectedPost( void ) const;
  const syncopa::vec3& colorRelated( void ) const;
  const syncopa::vec3& colorContext( void ) const;
  const syncopa::vec3& colorSynapsesPre( void ) const;
  const syncopa::vec3& colorSynapsesPost( void ) const;
  const syncopa::vec3& colorPathsPre( void ) const;
  const syncopa::vec3& colorPathsPost( void ) const;

  void showSelectedPre( int state );
  void showSelectedPost( int state );
  void showRelated( int state );
  void showContext( int state );
  void showSynapsesPre( int state );
  void showSynapsesPost( int state );
  void showPathsPre( int state );
  void showPathsPost( int state );

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

  void setupNeuronMorphologies( void );

  void setupSynapses( const std::set< unsigned int >& gidsPre,
                      const std::set< unsigned int >& gidsPost = std::set< unsigned int >( ));
  void setupPaths( const std::set< unsigned int >& gidsPre,
                   const std::set< unsigned int >& gidsPost );

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

  reto::ShaderProgram* _particlesShader;
//  prefr::ParticleSystem* _particleSystem;
  nlrender::Renderer* _nlrenderer;

  nsol::DataSet* _dataset;
  syncopa::NeuronScene* _neuronScene;
  syncopa::PSManager* _psManager;
  syncopa::PathFinder* _pathFinder;

  float _particleSizeThreshold;

//  QTimer* _cameraTimer;

  syncopa::TRenderMorpho _neuronsSelectedPre;
  syncopa::TRenderMorpho _neuronsSelectedPost;
  syncopa::TRenderMorpho _neuronsRelated;
  syncopa::TRenderMorpho _neuronsContext;

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

  syncopa::vec3 _colorSelectedPre;
  syncopa::vec3 _colorSelectedPost;
  syncopa::vec3 _colorRelated;
  syncopa::vec3 _colorContext;
  syncopa::vec3 _colorSynapsesPre;
  syncopa::vec3 _colorSynapsesPost;
  syncopa::vec3 _colorPathsPre;
  syncopa::vec3 _colorPathsPost;

  float _alphaSynapsesPre;
  float _alphaSynapsesPost;
  float _alphaPathsPre;
  float _alphaPathsPost;

  bool _showSelectedPre;
  bool _showSelectedPost;
  bool _showRelated;
  bool _showContext;
  bool _showSynapsesPre;
  bool _showSynapsesPost;
  bool _showPathsPre;
  bool _showPathsPost;

  std::vector< nsol::MorphologySynapsePtr > _currentSynapses;

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
