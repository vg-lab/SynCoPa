/*
 * @file  MainWindow.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#ifndef SYNCOPA_MAINWINDOW_H
#define SYNCOPA_MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QRadioButton>
#include <QListView>
#include <QTableView>
#include <QStandardItemModel>
#include <QGridLayout>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QPolygonF>
#include <QThread>
#include <QDialog>
#include <wcw/WebClientManager.h>

#include "GradientWidget.h"

#include "OpenGLWidget.h"
#include "PaletteColorWidget.h"
#include "SynCoPaWebAPI.h"

#include "ui_syncopa.h"
#include "NeuronClusterManager.h"
#include <memory>


class QProgressBar;

namespace Ui
{
  class MainWindow;
}

class MainWindow;

/** \class LoadingThread
 * \brief Loads the data into the application using a thread.
 *
 */
class LoadingThread
  : public QThread
{
Q_OBJECT
public:
  /** \brief LoadingThread class constructor.
   * \param[in] blueconfig Blueconfig file path.
   * \param[in] target Blueconfig target.
   * \param[in] mainW Application main window.
   */
  explicit LoadingThread( const std::string& blueconfig ,
                          const std::string& target , MainWindow* mainW )
    : m_blueconfig{ blueconfig }
    , m_target{ target }
    , m_parent{ mainW }
  { }

  virtual ~LoadingThread( )
  { }

  std::string errors( ) const
  { return m_errors; }

signals:

  void progress( const QString& message , const unsigned int value );

protected:
  virtual void run( );

private:
  const std::string m_blueconfig; /** blueconfig file path.              */
  const std::string m_target;     /** blueconfig target.                 */
  MainWindow* m_parent;     /** parent application main window.    */
  std::string m_errors;     /** error message or empty if success. */
};

class LoadingDialog
  : public QDialog
{
Q_OBJECT
public:
  /** \brief LoadingDialog class constructor.
   * \param[in] p Raw pointer of the widget parent of this one.
   * \param[in] f QDialog flags.
   *
   */
  explicit LoadingDialog( QWidget* p = nullptr );

  /** \brief LoadingDialog class virtual destructor.
   *
   */
  virtual ~LoadingDialog( )
  { };

public slots:

  /** \brief Updates the dialog with the message and progress value
   * \param[in] message Progress message.
   * \param[in] value Progress value in [0,100].
   *
   */
  void progress( const QString& message , const unsigned int value );

  /** \brief Closes and deletes the dialog.
   *
   */
  void closeDialog( );

private:
  QProgressBar* m_progress; /** progress bar. */
};

class MainWindow
  : public QMainWindow
{
Q_OBJECT

public:

  /** \brief MainWindow class constructor.
   * \param[in] parent Raw pointer of the widget parent of this one.
   * \param[in] updateOnIdle True to refresh Gl widget on idle and false
   *                         otherwise.
   * \param[in] fps True to show fps and false otherwise.
   *
   */
  explicit MainWindow(
    QWidget* parent = 0 ,
    bool updateOnIdle = true ,
    bool fps = false );

  virtual ~MainWindow( void );

  void init( );

  const std::shared_ptr< syncopa::NeuronClusterManager >&
  getNeuronClusterManager( ) const;

  void loadData( const std::string& dataset , const std::string& target );


  // Web events
  void manageSelectionEvent( const std::vector< unsigned int >& selection );

  void
  manageSynapsesSelectionEvent( const std::vector< unsigned int >& selection );

  void managePathsSelectionEvent(
    unsigned int preSelection ,
    const std::vector< unsigned int >& postSelection );


protected slots:

  void openBlueConfigThroughDialog( void );

  void exportDataDialog( void );

  void syncScene( void );

  void presynapticNeuronClicked( );

  void postsynapticNeuronClicked( );

  void setSynapseMappingState( int state );

  void setSynapseMappingAttribute( int attrib );

  void colorSelectionClicked( void );

  void colorSynapseMapAccepted( void );

  void transparencySliderMoved( int );

  void sizeSpinBoxChanged( double );

  void filteringStateChanged( void );

  void filteringBoundsChanged( void );

  void filteringPaletteChanged( void );

  void modeChanged( bool state );

  void alphaModeChanged( bool state );

  void clear( void );

  void dynamicStart( void );

  void dynamicPause( void );

  void dynamicStop( void );

  void neuronClusterManagerStructureRefresh( void );

  void neuronClusterManagerMetadataRefresh( void );

  /** \brief Helper method called after loading data.
   *
   */
  void onDataLoaded( );

  /** \brief Shows the about dialog.
   *
   */
  void aboutDialog( );

  /** \brief Starts/Stops the network connection.
   * \param[in] value True if button is checked and false otherwise.
   *
   */
  void onConnectionButtonTriggered( bool value );

  void onConnectionSynchronizationTriggered( bool value );

  void onConnectionError( );

  void onConnectionThreadTerminated( );

protected:

  bool showDialog( QColor& current , const QString& message = "" );

  void initListDock( void );

  void initSceneDock( void );

  void initColorDock( void );

  void initInfoDock( void );

  void updateInfoDock( void );

  void clearInfoDock( void );

  void loadPresynapticList( void );

  void loadPostsynapticList( unsigned int gid );

  void _loadDefaultValues( void );

  /** \brief Helper method to enable/disable the interface.
   *
   */
  void disableInterface( bool value );

  Ui::MainWindow* _ui;

  QString _lastOpenedFileNamePath;

  OpenGLWidget* _openGLWidget;

  QDockWidget* _dockList;
  QListView* _listPresynaptic;
  QStandardItemModel* _modelListPre;

  QListView* _listPostsynaptic;
  QStandardItemModel* _modelListPost;

  QRadioButton* _radioModeSynapses;
  QRadioButton* _radioModePaths;

  QRadioButton* _radioAlphaModeNormal;
  QRadioButton* _radioAlphaModeAccumulative;

  QDockWidget* _dockInfo;
  QVBoxLayout* _layoutInfo;
  QWidget* _widgetInfoPre;
  QWidget* _widgetInfoPost;

  QDockWidget* _dockColor;
  QPushButton* _frameColorSynapsesPre;
  QPushButton* _frameColorSynapsesPost;
  QPushButton* _frameColorPathsPre;
  QPushButton* _frameColorPathsPost;

  QGroupBox* _checkSynapsesPre;
  QGroupBox* _checkSynapsesPost;
  QGroupBox* _checkPathsPre;
  QGroupBox* _checkPathsPost;

  std::vector< std::tuple< QPushButton* , QPushButton* , QCheckBox*>>
    _egoNetworkButtons;

  std::vector< std::tuple< QColor , bool , bool>>
    _egoNetworkParameters;

  PaletteColorWidget* _colorMapWidget;

  QSlider* _sliderAlphaSynapsesPre;
  QSlider* _sliderAlphaSynapsesPost;
  QSlider* _sliderAlphaPathsPre;
  QSlider* _sliderAlphaPathsPost;

  QSlider* _sliderAlphaSynapsesMap;

  QDoubleSpinBox* _spinBoxSizeSynapsesPre;
  QDoubleSpinBox* _spinBoxSizeSynapsesPost;
  QDoubleSpinBox* _spinBoxSizePathsPre;
  QDoubleSpinBox* _spinBoxSizePathsPost;

  QDoubleSpinBox* _spinBoxSizeSynapsesMap;

  QPushButton* _frameColorDynamicPre;
  QPushButton* _frameColorDynamicPost;
  QPushButton* _buttonDynamicStart;
  QPushButton* _buttonDynamicStop;

  QComboBox* _comboSynapseMapAttrib;

  QLayout* _sceneLayout;

  QGroupBox* _groupBoxGeneral;
  QGroupBox* _groupBoxSynapses;
  QGroupBox* _groupBoxPaths;
  QGroupBox* _groupBoxDynamic;

  std::shared_ptr< LoadingThread > m_thread;

  SynCoPaWebAPI _web_api;
  std::shared_ptr< wcw::WebClientManager > _web_socket;

  std::shared_ptr< syncopa::NeuronClusterManager > _neuronClusterManager;

  friend class LoadingThread;
};

#endif //SYNCOPA_MAINWINDOW_H
