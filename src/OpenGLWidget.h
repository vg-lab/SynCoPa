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

#define PLAB_SKIP_GLEW_INCLUDE 1

#define NEUROLOTS_SKIP_GLEW_INCLUDE 1

#define SIM_SLIDER_UPDATE_PERIOD 0.5f

#include <nlrender/nlrender.h>
#include <reto/reto.h>
#include <nsol/nsol.h>

#include "NeuronScene.h"
#include "ParticleManager.h"
#include "PathFinder.h"
#include "DomainManager.h"
#include "NeuronClusterManager.h"
#include "DynamicPathGenerator.h"

#include <plab/reto/RetoCamera.h>
#include <QOpenGLDebugMessage>

    class ShaderProgram;

class OpenGLWidget
  : public QOpenGLWidget , public QOpenGLFunctions
{

Q_OBJECT;

public:

  OpenGLWidget( QWidget* parent = nullptr ,
                Qt::WindowFlags windowFlags = Qt::WindowFlags( )
              );

  ~OpenGLWidget( );

  void createParticleSystem( );

  void loadBlueConfig( const std::string& blueConfigFilePath ,
                       const std::string& target );

  /** \brief Performs data processing after a successful data loading.
   *
   */
  void loadPostprocess( );

  void home( bool animate = true );

  void idleUpdate( bool idleUpdate_ = true );

  void showFps( bool showFps_ = true );

  nsol::DataSet* dataset( );

  void alphaMode( bool alphaAccumulative = false );

  void mode( syncopa::TMode mode_ );

  syncopa::TMode mode( ) const;

  bool dynamicActive( ) const;

  syncopa::DomainManager* getDomainManager( ) const;

  const syncopa::gidUSet& getGidsAll( ) const;

signals:

  void progress( const QString& , const unsigned int );

public slots:

  void changeClearColor( );

  void toggleUpdateOnIdle( );

  void toggleShowFPS( );

  const std::vector< nsol::MorphologySynapsePtr >& currentSynapses( );

  void colorSynapsesPre( const syncopa::vec3& color );

  void colorSynapsesPost( const syncopa::vec3& color );

  void colorPathsPre( const syncopa::vec3& color );

  void colorPathsPost( const syncopa::vec3& color );

  void colorDynamicPre( const syncopa::vec3& color );

  void colorDynamicPost( const syncopa::vec3& color );

  void colorSynapseMap( const tQColorVec& colors );

  const syncopa::vec3 colorSynapsesPre( ) const;

  const syncopa::vec3 colorSynapsesPost( ) const;

  const syncopa::vec3 colorPathsPre( ) const;

  const syncopa::vec3 colorPathsPost( ) const;

  const syncopa::vec3 colorDynamicPre( ) const;

  const syncopa::vec3 colorDynamicPost( ) const;

  std::vector< std::pair< float , QColor >> colorSynapseMap( ) const;

  void alphaSynapsesPre( float transparency );

  void alphaSynapsesPost( float transparency );

  void alphaPathsPre( float transparency );

  void alphaPathsPost( float transparency );

  void alphaSynapseMap( const tQColorVec& colors , float transparency );

  float alphaSynapsesPre( ) const;

  float alphaSynapsesPost( ) const;

  float alphaPathsPre( ) const;

  float alphaPathsPost( ) const;

  float alphaSynapsesMap( ) const;

  void sizeSynapsesPre( float );

  void sizeSynapsesPost( float );

  void sizePathsPre( float );

  void sizePathsPost( float );

  void sizeSynapseMap( float );

  void sizeDynamic( float );

  float sizeSynapsesPre( ) const;

  float sizeSynapsesPost( ) const;

  float sizePathsPre( ) const;

  float sizePathsPost( ) const;

  float sizeSynapseMap( ) const;

  float sizeDynamic( ) const;

  void showSynapsesPre( bool state );

  void showSynapsesPost( bool state );

  void showPathsPre( bool state );

  void showPathsPost( bool state );

//  void showMorphologies( bool show );

  void setSynapseMapping( int attrib = ( int ) syncopa::TBSA_SYNAPSE_OTHER );

  void setSynapseMappingState( bool state );

  void startDynamic( );

  bool toggleDynamicMovement( );

  void stopDynamic( );

  const QPolygonF& getSynapseMappingPlot( ) const;

  void filteringState( bool state );

  void filteringBounds( float min , float max );

  std::pair< float , float > rangeBounds( ) const;

  void updateMorphologyModel(
    std::shared_ptr< syncopa::NeuronClusterManager > manager );

  void updateSynapsesModel(
    std::shared_ptr< syncopa::NeuronClusterManager > manager );

  void updatePathsModel(
    std::shared_ptr< syncopa::NeuronClusterManager > manager );

protected:

  virtual void initializeGL( );

  virtual void paintGL( );

  virtual void resizeGL( int w , int h );

//  bool showDialog( QColor& current, const std::string& message = "" );

  virtual void mousePressEvent( QMouseEvent* event );

  virtual void mouseReleaseEvent( QMouseEvent* event );

  virtual void wheelEvent( QWheelEvent* event );

  virtual void mouseMoveEvent( QMouseEvent* event );

  virtual void keyPressEvent( QKeyEvent* event );

  void setupSynapses( );

  void setupPaths( );

  void paintParticles( );

  void paintMorphologies( );

  void initRenderToTexture( );

  void recalculateTextureDimensions( int width_ , int height_ );

  QLabel _fpsLabel;
  bool _showFps;

  std::shared_ptr< plab::RetoCamera > _camera;
  reto::CameraAnimation* _animation;
  glm::vec3 _lastCameraPosition;

  unsigned int _frameCount;
  float _deltaTime;

  int _mouseX , _mouseY;
  bool _rotation , _translation;

  bool _idleUpdate;
  bool _paint;

  QColor _currentClearColor;

  std::chrono::time_point< std::chrono::system_clock > _then;
  std::chrono::time_point< std::chrono::system_clock > _lastFrame;

  nlrender::Renderer* _nlrenderer;

  nsol::DataSet* _dataset;
  syncopa::ParticleManager _particleManager;
  syncopa::PathFinder _pathFinder;
  syncopa::NeuronScene* _neuronScene;
  syncopa::DomainManager* _domainManager;

  syncopa::TMode _mode;

  float _particleSizeThreshold;

  std::vector< syncopa::TRenderMorpho > _neuronModel;

  float _renderSpeed;
  float _maxFPS;
  float _renderPeriod;
  float _elapsedTimeRenderAcc;
  float _renderPeriodMicroseconds;

  syncopa::gidUSet _gidsAll;

  tQColorVec _colorSynMap;
  float _alphaSynapsesMap;

  bool _dynamicActive;
  bool _dynamicMovement;

  std::vector< nsol::MorphologySynapsePtr > _currentSynapses;

  // Render to texture
  QOpenGLFunctions_4_0_Core* _oglFunctions;

  reto::ShaderProgram* _screenPlaneShader;

  unsigned int _quadVAO;
  unsigned int _opaqueFrameBuffer;
  unsigned int _weightFrameBuffer;

  unsigned int _opaqueColorTexture;
  unsigned int _opaqueDepthTexture;
  unsigned int _accumulationTexture;
  unsigned int _revealTexture;

  int _currentWidth;
  int _currentHeight;

  Eigen::Vector2f _inverseResolution;

  bool _mapSynapseValues;
  syncopa::TBrainSynapseAttribs _currentSynapseAttrib;

};

#endif // __VISIMPL__OPENGLWIDGET__
