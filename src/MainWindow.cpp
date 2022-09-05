/*
 * @file  MainWindow.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "MainWindow.h"
#include "NeuronClusterView.h"
#include "DataExport.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QGridLayout>
#include <QScrollArea>
#include <QColorDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QProgressBar>
#include <QDateTime>
#include <QSurface>
#include <QUrl>
#include <QMenu>

#include <syncopa/version.h>

#ifdef SYNCOPA_USE_GMRVLEX
#include <gmrvlex/gmrvlex.h>
#endif

#include <thread>

#define INITIAL_EGO_NETWORK_DISTANCES 5

using namespace syncopa;

syncopa::vec3 colorQtToEigen( const QColor& color_ )
{
  constexpr float invRGB = 1.0 / 255;
  return syncopa::vec3(
    std::min( 1.0f , std::max( 0.0f , color_.red( ) * invRGB )) ,
    std::min( 1.0f , std::max( 0.0f , color_.green( ) * invRGB )) ,
    std::min( 1.0f , std::max( 0.0f , color_.blue( ) * invRGB )));
}

QColor colorEigenToQt( const syncopa::vec3& color_ )
{
  return QColor( std::min( 255 , std::max( 0 , int( color_.x( ) * 255 ))) ,
                 std::min( 255 , std::max( 0 , int( color_.y( ) * 255 ))) ,
                 std::min( 255 , std::max( 0 , int( color_.z( ) * 255 ))));
}

constexpr int SLIDER_MIN = 0;
constexpr int SLIDER_MAX = 100;
constexpr float INV_RANGE_SLIDERS = 1.0 / ( SLIDER_MAX - SLIDER_MIN );

MainWindow::MainWindow(
  QWidget* parent_ ,
  bool updateOnIdle ,
  bool fps )
  : QMainWindow( parent_ )
  , _ui( new Ui::MainWindow )
  , _openGLWidget( nullptr )
  , _dockList( nullptr )
  , _listPresynaptic( nullptr )
  , _modelListPre( nullptr )
  , _listPostsynaptic( nullptr )
  , _modelListPost( nullptr )
  , _radioModeSynapses( nullptr )
  , _radioModePaths( nullptr )
  , _radioAlphaModeNormal( nullptr )
  , _radioAlphaModeAccumulative( nullptr )
  , _dockInfo( nullptr )
  , _layoutInfo( nullptr )
  , _widgetInfoPre( nullptr )
  , _widgetInfoPost( nullptr )
  , _dockColor( nullptr )
  , _frameColorSynapsesPre( nullptr )
  , _frameColorSynapsesPost( nullptr )
  , _checkSynapsesPre( nullptr )
  , _checkSynapsesPost( nullptr )
  , _checkPathsPre( nullptr )
  , _checkPathsPost( nullptr )
  , _colorMapWidget( nullptr )
  , _sliderAlphaSynapsesPre( nullptr )
  , _sliderAlphaSynapsesPost( nullptr )
  , _sliderAlphaPathsPre( nullptr )
  , _sliderAlphaPathsPost( nullptr )
  , _sliderAlphaSynapsesMap( nullptr )
  , _spinBoxSizeSynapsesPre( nullptr )
  , _spinBoxSizeSynapsesPost( nullptr )
  , _spinBoxSizePathsPre( nullptr )
  , _spinBoxSizePathsPost( nullptr )
  , _spinBoxSizeSynapsesMap( nullptr )
  , _buttonDynamicStart( nullptr )
  , _buttonDynamicStop( nullptr )
  , _comboSynapseMapAttrib( nullptr )
  , _sceneLayout( nullptr )
  , _groupBoxGeneral( nullptr )
  , _groupBoxSynapses( nullptr )
  , _groupBoxPaths( nullptr )
  , _groupBoxDynamic( nullptr )
  , m_thread{ nullptr }
  , _web_api( this , this )
  , _web_socket( nullptr )
  , _neuronClusterManager( std::make_shared< NeuronClusterManager >( ))
{
  _ui->setupUi( this );

  _ui->actionUpdateOnIdle->setChecked( updateOnIdle );
  _ui->actionShowFPSOnIdleUpdate->setChecked( fps );

  connect( _ui->actionQuit , SIGNAL( triggered( )) ,
           QApplication::instance( ) , SLOT( quit( )) );

  connect( _ui->actionOpenBlueConfig , SIGNAL( triggered( void )) ,
           this , SLOT( openBlueConfigThroughDialog( void )) );

  connect( _ui->actionExport , SIGNAL( triggered( void )) ,
           this , SLOT( exportDataDialog( void )) );

  connect( _ui->actionSyncScene , SIGNAL( triggered( void )) ,
           this , SLOT( syncScene( void )) );

  connect( _ui->actionNetworkConnection , SIGNAL( triggered( bool )) ,
           this , SLOT( onConnectionButtonTriggered( bool )) );

  connect( _ui->actionNetworkSynchronization , SIGNAL( triggered( bool )) ,
           this , SLOT( onConnectionSynchronizationTriggered( bool )) );

  _openGLWidget = new OpenGLWidget( nullptr , Qt::WindowFlags( ));
  setCentralWidget( _openGLWidget );

  if ( _openGLWidget->format( ).version( ).first < 4 )
  {
    std::cerr << "This application requires at least OpenGL 4.0" << std::endl;
    exit( -1 );
  }
}

MainWindow::~MainWindow( void )
{
  onConnectionThreadTerminated( );

  delete _ui;
}

void MainWindow::init( )
{
  _openGLWidget->createParticleSystem( );

  initSceneDock( );
  initListDock( );
  initColorDock( );
  initInfoDock( );

  tabifyDockWidget( _dockList , _dockColor );
  tabifyDockWidget( _dockColor , _dockInfo );

  _openGLWidget->idleUpdate( _ui->actionUpdateOnIdle->isChecked( ));
  _openGLWidget->showFps( _ui->actionShowFPSOnIdleUpdate->isChecked( ));
  _ui->actionSyncScene->setEnabled( false );

  connect( _ui->actionUpdateOnIdle , SIGNAL( triggered( )) ,
           _openGLWidget , SLOT( toggleUpdateOnIdle( )) );

  connect( _ui->actionBackgroundColor , SIGNAL( triggered( )) ,
           _openGLWidget , SLOT( changeClearColor( )) );

  connect( _ui->actionShowFPSOnIdleUpdate , SIGNAL( triggered( )) ,
           _openGLWidget , SLOT( toggleShowFPS( )) );

  connect( _ui->actionAbout , SIGNAL( triggered( )) ,
           this , SLOT( aboutDialog( )) );

  _radioModeSynapses->setChecked( true );

  _loadDefaultValues( );
}

void MainWindow::initListDock( void )
{
  _radioModeSynapses = new QRadioButton( tr( "Synapses" ));
  _radioModePaths = new QRadioButton( tr( "Paths" ));

  auto groupSelection = new QGroupBox( "Selection mode" );
  auto selectionLayout = new QVBoxLayout( );

  selectionLayout->addWidget( _radioModeSynapses );
  selectionLayout->addWidget( _radioModePaths );

  groupSelection->setLayout( selectionLayout );

  _listPresynaptic = new QListView( );
  _listPresynaptic->setSelectionMode( QAbstractItemView::SingleSelection );
  _listPresynaptic->setEditTriggers( QAbstractItemView::NoEditTriggers );

  _listPostsynaptic = new QListView( );
  _listPostsynaptic->setSelectionMode( QAbstractItemView::ExtendedSelection );
  _listPostsynaptic->setEditTriggers( QAbstractItemView::NoEditTriggers );

  _dockList = new QDockWidget( tr( "Selection" ));
  _dockList->setMinimumHeight( 400 );

  auto dockLayout = new QVBoxLayout( );
  dockLayout->setAlignment( Qt::AlignTop );

  auto container = new QWidget( );
  container->setLayout( dockLayout );

  auto clearButton = new QPushButton( "Clear selection" );
  connect( clearButton , SIGNAL( clicked( void )) ,
           this , SLOT( clear( void )) );

  dockLayout->addWidget( groupSelection , 0 );
  dockLayout->addWidget( new QLabel( "Presynaptic:" ) , 0 );
  dockLayout->addWidget( _listPresynaptic , 0 );
  dockLayout->addWidget( new QLabel( "Postsynaptic:" ) , 0 );
  dockLayout->addWidget( _listPostsynaptic , 0 );
  dockLayout->addWidget( clearButton );

  _dockList->setWidget( container );

  connect( _radioModeSynapses , SIGNAL( toggled( bool )) ,
           this , SLOT( modeChanged( bool )) );

  _modelListPre = new QStandardItemModel( );
  _listPresynaptic->setModel( _modelListPre );

  connect( _listPresynaptic->selectionModel( ) , SIGNAL(
                                                   selectionChanged(
                                                   const QItemSelection & , const QItemSelection &)) ,
           this , SLOT( presynapticNeuronClicked( )) );

  _modelListPost = new QStandardItemModel( );
  _listPostsynaptic->setModel( _modelListPost );

  connect( _listPostsynaptic->selectionModel( ) , SIGNAL(
                                                    selectionChanged(
                                                    const QItemSelection & , const QItemSelection &)) ,
           this , SLOT( postsynapticNeuronClicked( )) );

  addDockWidget( Qt::RightDockWidgetArea , _dockList );

  auto action = _dockList->toggleViewAction( );
  action->setIcon( QIcon( ":/icons/neuron_all.png" ));
  action->setToolTip( tr( "Toggle neuron lists panel visibility" ));
  _ui->menuPanels->addAction( action );
  _ui->toolBar->addAction( action );
}

void MainWindow::initSceneDock( )
{
  auto sceneWidget = new QWidget( );
  _sceneLayout = new QVBoxLayout( );
  _sceneLayout->setAlignment( Qt::AlignTop | Qt::AlignLeft );
  _sceneLayout->setMargin( 0 );
  _sceneLayout->setSpacing( 0 );
  sceneWidget->setLayout( _sceneLayout );

  auto scrollMorpho = new QScrollArea( );
  scrollMorpho->setWidgetResizable( true );
  scrollMorpho->setWidget( sceneWidget );

  auto dockScene = new QDockWidget( "Scene" );
  dockScene->setWidget( scrollMorpho );
  addDockWidget( Qt::LeftDockWidgetArea , dockScene );

  auto defaultSelections =
    std::map< QString , std::unordered_set< unsigned int >>( );
  defaultSelections[ "Presynaptic" ] = std::unordered_set< unsigned int >{ };
  defaultSelections[ "Postsynaptic" ] = std::unordered_set< unsigned int >{ };
  defaultSelections[ "Connected" ] = std::unordered_set< unsigned int >{ };
  defaultSelections[ "Other" ] = std::unordered_set< unsigned int >{ };
  auto selectionCluster = syncopa::NeuronCluster( "Main Group" ,
                                                  defaultSelections );

  _neuronClusterManager->addCluster( selectionCluster );
  neuronClusterManagerStructureRefresh( );

  connect(
    _neuronClusterManager.get( ) , SIGNAL( onStructureModification( )) ,
    this , SLOT( neuronClusterManagerStructureRefresh( ))
  );

  connect(
    _neuronClusterManager.get( ) , SIGNAL( onMetadataModification( )) ,
    this , SLOT( neuronClusterManagerMetadataRefresh( ))
  );

  auto action = dockScene->toggleViewAction( );
  action->setIcon( QIcon( ":/icons/eye_open.svg" ));
  action->setToolTip( tr( "Toggle scene configuration panel visibility" ));
  _ui->menuPanels->addAction( action );
  _ui->toolBar->addAction( action );
}

void MainWindow::initColorDock( void )
{
  _dockColor = new QDockWidget( "Visual configuration" );

  auto scrollArea = new QScrollArea( );
  auto layout = new QVBoxLayout( );
  auto container = new QWidget( );
  container->setLayout( layout );

  _groupBoxGeneral = new QGroupBox( "Alpha blending" );
  _groupBoxSynapses = new QGroupBox( "Synapses" );
  _groupBoxPaths = new QGroupBox( "Paths" );
  _groupBoxDynamic = new QGroupBox( "Dynamic" );

  _radioAlphaModeNormal = new QRadioButton( "Normal" );
  _radioAlphaModeAccumulative = new QRadioButton( "Accumulative" );

  _checkSynapsesPre = new QGroupBox( "Presynaptic" );
  _checkSynapsesPre->setCheckable( true );
  _checkSynapsesPost = new QGroupBox( "Postsynaptic" );
  _checkSynapsesPost->setCheckable( true );

  _checkPathsPre = new QGroupBox( "Presynaptic" );
  _checkPathsPre->setCheckable( true );
  _checkPathsPost = new QGroupBox( "Postsynaptic" );
  _checkPathsPost->setCheckable( true );

  _checkSynapsesPre->setChecked( true );
  _checkSynapsesPost->setChecked( true );
  _checkPathsPre->setChecked( true );
  _checkPathsPost->setChecked( true );

  auto layoutGeneral = new QVBoxLayout( );
  auto containerGeneral = new QWidget( );
  containerGeneral->setLayout( layoutGeneral );
  layoutGeneral->addWidget( _groupBoxGeneral );

  auto generalLayout = new QVBoxLayout( );
  generalLayout->setAlignment( Qt::AlignTop );
  _groupBoxGeneral->setLayout( generalLayout );

  auto layoutSynapses = new QVBoxLayout( );
  auto containerSynapses = new QWidget( );
  containerSynapses->setLayout( layoutSynapses );
  layoutSynapses->addWidget( _groupBoxSynapses );

  auto synLayout = new QGridLayout( );
  synLayout->setAlignment( Qt::AlignTop );
  _groupBoxSynapses->setLayout( synLayout );

  auto layoutPaths = new QVBoxLayout( );
  auto containerPaths = new QWidget( );
  containerPaths->setLayout( layoutPaths );
  layoutPaths->addWidget( _groupBoxPaths );

  layoutPaths->addWidget( _groupBoxDynamic );

  auto pathLayout = new QGridLayout( );
  pathLayout->setAlignment( Qt::AlignTop );
  _groupBoxPaths->setLayout( pathLayout );

  auto layoutDynamic = new QGridLayout( );
  layoutDynamic->setAlignment( Qt::AlignTop );
  _groupBoxDynamic->setLayout( layoutDynamic );

  QPalette palette;
  QColor color;

  _frameColorSynapsesPre = new QPushButton( );
  _frameColorSynapsesPre->setFixedSize( 20 , 20 );

  color = colorEigenToQt( _openGLWidget->colorSynapsesPre( ));
  _frameColorSynapsesPre->setStyleSheet( "background-color: " + color.name( ));

  _frameColorSynapsesPost = new QPushButton( );
  _frameColorSynapsesPost->setFixedSize( 20 , 20 );

  color = colorEigenToQt( _openGLWidget->colorSynapsesPost( ));
  _frameColorSynapsesPost->setStyleSheet( "background-color: " + color.name( ));

  _frameColorPathsPre = new QPushButton( );
  _frameColorPathsPre->setFixedSize( 20 , 20 );

  color = colorEigenToQt( _openGLWidget->colorPathsPre( ));
  _frameColorPathsPre->setStyleSheet( "background-color: " + color.name( ));

  _frameColorPathsPost = new QPushButton( );
  _frameColorPathsPost->setFixedSize( 20 , 20 );

  color = colorEigenToQt( _openGLWidget->colorPathsPost( ));
  _frameColorPathsPost->setStyleSheet( "background-color: " + color.name( ));

  QIcon neuronIcon;
  neuronIcon.addFile(QStringLiteral( ":/icons/neurotessmesh.png" ) , QSize( ) ,
                     QIcon::Normal , QIcon::On );
  neuronIcon.addFile(QStringLiteral( ":/icons/neurolots.png" ) , QSize( ) ,
                     QIcon::Normal , QIcon::Off );

  // General controls
  generalLayout->addWidget( _radioAlphaModeNormal );
  generalLayout->addWidget( _radioAlphaModeAccumulative );

  // Synapses
  _colorMapWidget = new PaletteColorWidget( );
  _colorMapWidget->init( false );

  connect( _colorMapWidget , SIGNAL( filterStateChanged( void )) ,
           this , SLOT( filteringStateChanged( void )) );

  connect( _colorMapWidget , SIGNAL( filterPaletteChanged( void )) ,
           this , SLOT( filteringPaletteChanged( void )) );

  connect( _colorMapWidget , SIGNAL( filterBoundsChanged( void )) ,
           this , SLOT( filteringBoundsChanged( void )) );

  _sliderAlphaSynapsesPre = new QSlider( Qt::Horizontal );
  _sliderAlphaSynapsesPre->setRange( SLIDER_MIN , SLIDER_MAX );

  _sliderAlphaSynapsesPost = new QSlider( Qt::Horizontal );
  _sliderAlphaSynapsesPost->setRange( SLIDER_MIN , SLIDER_MAX );

  _sliderAlphaPathsPre = new QSlider( Qt::Horizontal );
  _sliderAlphaPathsPre->setRange( SLIDER_MIN , SLIDER_MAX );

  _sliderAlphaPathsPost = new QSlider( Qt::Horizontal );
  _sliderAlphaPathsPost->setRange( SLIDER_MIN , SLIDER_MAX );

  _sliderAlphaSynapsesMap = new QSlider( Qt::Horizontal );
  _sliderAlphaSynapsesMap->setRange( SLIDER_MIN , SLIDER_MAX );

  _spinBoxSizeSynapsesPre = new QDoubleSpinBox( );
  _spinBoxSizeSynapsesPre->setValue( 3.0f );

  _spinBoxSizeSynapsesPost = new QDoubleSpinBox( );
  _spinBoxSizeSynapsesPost->setValue( 3.0f );

  _spinBoxSizePathsPre = new QDoubleSpinBox( );
  _spinBoxSizePathsPre->setValue( 3.0f );

  _spinBoxSizePathsPost = new QDoubleSpinBox( );
  _spinBoxSizePathsPost->setValue( 3.0f );

  _spinBoxSizeSynapsesMap = new QDoubleSpinBox( );
  _spinBoxSizePathsPost->setValue( 3.0f );

  auto checkMapSynapses = new QCheckBox( tr( "Map attribute to color" ));
  _comboSynapseMapAttrib = new QComboBox( );

  QStringList optionList = { "Delay" ,
                             "Conductance" ,
                             "Utilization" ,
                             "Depression" ,
                             "Facilitation" ,
                             "Decay" ,
                             "Efficacy" ,
                             "Synapse type" };

  _comboSynapseMapAttrib->addItems( optionList );

  auto line = new QFrame( );
  line->setFrameShape( QFrame::HLine );
  line->setFrameShadow( QFrame::Sunken );

  int row = 0;
  int col = 0;

  auto lay = new QGridLayout( );
  _checkSynapsesPre->setLayout( lay );
  lay->addWidget( _frameColorSynapsesPre , 0 , 0 , 1 , 1 );
  lay->addWidget( _spinBoxSizeSynapsesPre , 0 , 1 , 1 , 1 );
  lay->addWidget( _sliderAlphaSynapsesPre , 0 , 2 , 1 , 2 );

  lay = new QGridLayout( );
  _checkSynapsesPost->setLayout( lay );
  lay->addWidget( _frameColorSynapsesPost , 0 , 0 , 1 , 1 );
  lay->addWidget( _spinBoxSizeSynapsesPost , 0 , 1 , 1 , 1 );
  lay->addWidget( _sliderAlphaSynapsesPost , 0 , 2 , 1 , 2 );


  synLayout->addWidget( _checkSynapsesPre , 0 , 0 , 2 , 4 );
  synLayout->addWidget( _checkSynapsesPost , 2 , 0 , 2 , 4 );

  row = 4;
  col = 0;

  synLayout->addWidget( line , row , col , 1 , 4 );

  ++row;
  synLayout->addWidget( checkMapSynapses , row , 0 , 1 , 2 );
  synLayout->addWidget( _comboSynapseMapAttrib , row , 2 , 1 , 2 );

  ++row;
  col = 0;

  synLayout->addWidget( _spinBoxSizeSynapsesMap , row , col++ , 1 , 1 );
  synLayout->addWidget( _sliderAlphaSynapsesMap , row , col , 1 , 3 );

  ++row;
  col = 0;

  synLayout->addWidget( _colorMapWidget , row , col , 4 , 4 );

  // Paths
  row = 0;
  col = 0;

  lay = new QGridLayout( );
  lay->addWidget( _frameColorPathsPre , 0 , 0 , 1 , 1 );
  lay->addWidget( _spinBoxSizePathsPre , 0 , 1 , 1 , 1 );
  lay->addWidget( _sliderAlphaPathsPre , 0 , 2 , 1 , 2 );
  _checkPathsPre->setLayout( lay );

  lay = new QGridLayout( );
  lay->addWidget( _frameColorPathsPost , 0 , 0 , 1 , 1 );
  lay->addWidget( _spinBoxSizePathsPost , 0 , 1 , 1 , 1 );
  lay->addWidget( _sliderAlphaPathsPost , 0 , 2 , 1 , 2 );
  _checkPathsPost->setLayout( lay );

  pathLayout->addWidget( _checkPathsPre , 0 , 0 , 2 , 4 );
  pathLayout->addWidget( _checkPathsPost , 2 , 0 , 2 , 4 );

  _frameColorDynamicPre = new QPushButton( );
  _frameColorDynamicPre->setFixedSize( 20 , 20 );
  _frameColorDynamicPre->setStyleSheet( "background-color: #FFFFFFFF" );

  _frameColorDynamicPost = new QPushButton( );
  _frameColorDynamicPost->setFixedSize( 20 , 20 );
  _frameColorDynamicPost->setStyleSheet( "background-color: #FFFFFFFF" );

  _buttonDynamicStart = new QPushButton( "Start" );
  _buttonDynamicStop = new QPushButton( "Stop" );

  layoutDynamic->addWidget( _frameColorDynamicPre , 0 , 0 , 1 , 1 );
  layoutDynamic->addWidget( new QLabel( "Presynaptic" ) , 0 , 1 , 1 , 1 );

  layoutDynamic->addWidget( _frameColorDynamicPost , 1 , 0 , 1 , 1 );
  layoutDynamic->addWidget( new QLabel( "Postsynaptic" ) , 1 , 1 , 1 , 1 );

  layoutDynamic->addWidget( _buttonDynamicStart , 0 , 2 , 1 , 1 );
  layoutDynamic->addWidget( _buttonDynamicStop , 1 , 2 , 1 , 1 );

  auto tabsWidget = new QTabWidget( );
  tabsWidget->setTabPosition( QTabWidget::West );
  tabsWidget->addTab( containerGeneral , "General" );
  tabsWidget->addTab( containerSynapses , "Synapses" );
  tabsWidget->addTab( containerPaths , "Paths" );

  scrollArea->setWidget( container );
  scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

  _dockColor->setWidget( tabsWidget );
  addDockWidget( Qt::RightDockWidgetArea , _dockColor );

  // SLOTS
  connect( _radioAlphaModeNormal , SIGNAL( toggled( bool )) ,
           this , SLOT( alphaModeChanged( bool )) );

  connect( _buttonDynamicStart , SIGNAL( clicked( )) , this ,
           SLOT( dynamicStart( )) );
  connect( _buttonDynamicStop , SIGNAL( clicked( )) , this ,
           SLOT( dynamicStop( )) );

  connect( _frameColorSynapsesPre , SIGNAL( clicked( )) ,
           this , SLOT( colorSelectionClicked( )) );

  connect( _frameColorSynapsesPost , SIGNAL( clicked( )) ,
           this , SLOT( colorSelectionClicked( )) );

  connect( _frameColorPathsPre , SIGNAL( clicked( )) ,
           this , SLOT( colorSelectionClicked( )) );

  connect( _frameColorPathsPost , SIGNAL( clicked( )) ,
           this , SLOT( colorSelectionClicked( )) );

  connect( _frameColorDynamicPre , SIGNAL( clicked( )) ,
           this , SLOT( colorSelectionClicked( )) );

  connect( _frameColorDynamicPost , SIGNAL( clicked( )) ,
           this , SLOT( colorSelectionClicked( )) );


  connect( _checkSynapsesPre , SIGNAL( toggled( bool )) ,
           _openGLWidget , SLOT( showSynapsesPre( bool )) );
  connect( _checkSynapsesPost , SIGNAL( toggled( bool )) ,
           _openGLWidget , SLOT( showSynapsesPost( bool )) );
  connect( _checkPathsPre , SIGNAL( toggled( bool )) ,
           _openGLWidget , SLOT( showPathsPre( bool )) );
  connect( _checkPathsPost , SIGNAL( toggled( bool )) ,
           _openGLWidget , SLOT( showPathsPost( bool )) );

  connect( _sliderAlphaSynapsesPre , SIGNAL( valueChanged( int )) ,
           this , SLOT( transparencySliderMoved( int )) );
  connect( _sliderAlphaSynapsesPost , SIGNAL( valueChanged( int )) ,
           this , SLOT( transparencySliderMoved( int )) );
  connect( _sliderAlphaPathsPre , SIGNAL( valueChanged( int )) ,
           this , SLOT( transparencySliderMoved( int )) );
  connect( _sliderAlphaPathsPost , SIGNAL( valueChanged( int )) ,
           this , SLOT( transparencySliderMoved( int )) );
  connect( _sliderAlphaSynapsesMap , SIGNAL( valueChanged( int )) ,
           this , SLOT( transparencySliderMoved( int )) );

  connect( _spinBoxSizeSynapsesPre , SIGNAL( valueChanged( double )) ,
           this , SLOT( sizeSpinBoxChanged( double )) );
  connect( _spinBoxSizeSynapsesPost , SIGNAL( valueChanged( double )) ,
           this , SLOT( sizeSpinBoxChanged( double )) );
  connect( _spinBoxSizePathsPre , SIGNAL( valueChanged( double )) ,
           this , SLOT( sizeSpinBoxChanged( double )) );
  connect( _spinBoxSizePathsPost , SIGNAL( valueChanged( double )) ,
           this , SLOT( sizeSpinBoxChanged( double )) );
  connect( _spinBoxSizeSynapsesMap , SIGNAL( valueChanged( double )) ,
           this , SLOT( sizeSpinBoxChanged( double )) );

  checkMapSynapses->setChecked( true );
  connect( checkMapSynapses , SIGNAL( stateChanged( int )) ,
           this , SLOT( setSynapseMappingState( int )) );

  connect( _comboSynapseMapAttrib , SIGNAL( currentIndexChanged( int )) ,
           this , SLOT( setSynapseMappingAttribute( int )) );

  _radioAlphaModeNormal->setChecked( true );

  checkMapSynapses->setChecked( false );

  auto action = _dockColor->toggleViewAction( );
  action->setIcon( QIcon( ":/icons/colorpicker.svg" ));
  action->setToolTip( tr( "Toggle colors panel visibility" ));
  _ui->menuPanels->addAction( action );
  _ui->toolBar->addAction( action );
}

void MainWindow::initInfoDock( void )
{
  _dockInfo = new QDockWidget( tr( "Information" ));
  _dockInfo->setMinimumHeight( 400 );

  auto container = new QWidget( );

  _layoutInfo = new QVBoxLayout( );
  container->setLayout( _layoutInfo );

  auto scrollInfo = new QScrollArea( );
  scrollInfo->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  scrollInfo->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  scrollInfo->setWidget( container );
  scrollInfo->setWidgetResizable( true );

  _dockInfo->setWidget( scrollInfo );

  addDockWidget( Qt::RightDockWidgetArea , _dockInfo );

  auto action = _dockInfo->toggleViewAction( );
  action->setIcon( QIcon( ":/icons/information.svg" ));
  action->setToolTip( tr( "Toggle information panel visibility" ));
  _ui->menuPanels->addAction( action );
  _ui->toolBar->addAction( action );
}

void MainWindow::onDataLoaded( )
{
  _ui->statusbar->clearMessage( );

  if ( m_thread )
  {
    const auto errors = m_thread->errors( );
    if ( !errors.empty( ))
    {
      QApplication::restoreOverrideCursor( );
      _ui->toolBar->setEnabled( true );

      m_thread = nullptr;

      QMessageBox msgBox( this );
      msgBox.setWindowTitle( tr( "Error loading data" ));
      msgBox.setIcon( QMessageBox::Icon::Critical );
      msgBox.setText( QString::fromStdString( errors ));
      msgBox.exec( );

      return;
    }
  }

  _openGLWidget->loadPostprocess( );

  loadPresynapticList( );

  disableInterface( false );

  // Refresh view data clearing the selection.
  clear( );

  m_thread = nullptr;
}

void MainWindow::_loadDefaultValues( void )
{
  const int range = SLIDER_MAX - SLIDER_MIN;

  int value = _openGLWidget->alphaSynapsesPre( ) * range + SLIDER_MIN;
  value = std::min( SLIDER_MAX , std::max( SLIDER_MIN , value ));

  _sliderAlphaSynapsesPre->setSliderPosition( value );

  value = _openGLWidget->alphaSynapsesPost( ) * range + SLIDER_MIN;
  value = std::min( SLIDER_MAX , std::max( SLIDER_MIN , value ));

  _sliderAlphaSynapsesPost->setSliderPosition( value );

  value = _openGLWidget->alphaPathsPre( ) * range + SLIDER_MIN;
  value = std::min( SLIDER_MAX , std::max( SLIDER_MIN , value ));

  _sliderAlphaPathsPre->setSliderPosition( value );

  value = _openGLWidget->alphaPathsPost( ) * range + SLIDER_MIN;
  value = std::min( SLIDER_MAX , std::max( SLIDER_MIN , value ));

  _sliderAlphaPathsPost->setSliderPosition( value );

  value = _openGLWidget->alphaSynapsesMap( ) * range + SLIDER_MIN;
  value = std::min( SLIDER_MAX , std::max( SLIDER_MIN , value ));

  _sliderAlphaSynapsesMap->setSliderPosition( value );

  _openGLWidget->colorSynapseMap( _colorMapWidget->getColors( ));

  _spinBoxSizeSynapsesPre->setValue( _openGLWidget->sizeSynapsesPre( ));
  _spinBoxSizeSynapsesPost->setValue( _openGLWidget->sizeSynapsesPost( ));
  _spinBoxSizePathsPre->setValue( _openGLWidget->sizePathsPre( ));
  _spinBoxSizePathsPost->setValue( _openGLWidget->sizePathsPost( ));
  _spinBoxSizeSynapsesMap->setValue( _openGLWidget->sizeSynapseMap( ));

}

static bool sortAscending( const std::pair< unsigned int , unsigned int >& lhs ,
                           const std::pair< unsigned int , unsigned int >& rhs )
{
  return lhs.first < rhs.first;
}

void MainWindow::updateInfoDock( void )
{
  clearInfoDock( );

  const auto& synapses = _openGLWidget->currentSynapses( );

  std::unordered_map< unsigned int , unsigned int > presentSynapsesPre;
  std::unordered_map< unsigned int , unsigned int > presentSynapsesPost;

  std::vector< std::pair< unsigned int , unsigned int >> vecSynPre;
  std::vector< std::pair< unsigned int , unsigned int >> vecSynPost;

  for ( auto syn: synapses )
  {
    const unsigned int gidPre = syn->preSynapticNeuron( );
    const unsigned int gidPost = syn->postSynapticNeuron( );

    auto synapseNumberPre = presentSynapsesPre.find( gidPre );
    if ( synapseNumberPre == presentSynapsesPre.end( ))
    {
      synapseNumberPre =
        presentSynapsesPre.insert(
          std::make_pair( gidPre , vecSynPre.size( ))).first;

      vecSynPre.push_back( std::make_pair( gidPre , 0 ));
    }

    vecSynPre[ synapseNumberPre->second ].second++;

    auto synapseNumberPost = presentSynapsesPost.find( gidPost );
    if ( synapseNumberPost == presentSynapsesPost.end( ))
    {
      synapseNumberPost =
        presentSynapsesPost.insert(
          std::make_pair( gidPost , vecSynPost.size( ))).first;

      vecSynPost.push_back( std::make_pair( gidPost , 0 ));
    }

    vecSynPost[ synapseNumberPost->second ].second++;
  }

  std::sort( vecSynPre.begin( ) , vecSynPre.end( ) , sortAscending );
  std::sort( vecSynPost.begin( ) , vecSynPost.end( ) , sortAscending );

  for ( unsigned int i = 0; i < 2; ++i )
  {
    QWidget* widget = nullptr;
    std::vector< std::pair< unsigned int , unsigned int >>* presentSynapses = nullptr;

    if ( i == 0 )
    {
      _widgetInfoPre = new QGroupBox( tr( "Presynaptic neurons:" ));
      widget = _widgetInfoPre;
      presentSynapses = &vecSynPre;
    }
    else
    {
      _widgetInfoPost = new QGroupBox( tr( "Postsynaptic neurons:" ));
      widget = _widgetInfoPost;
      presentSynapses = &vecSynPost;
    }

    auto upperLayout = new QGridLayout( );
    widget->setLayout( upperLayout );

    auto container = new QWidget( );
    QGridLayout* layoutWidget = new QGridLayout( );
    container->setLayout( layoutWidget );

    auto scroll = new QScrollArea( );
    scroll->setWidgetResizable( true );
    scroll->setWidget( container );

    unsigned int currentRow = 0;
    unsigned int currentColumn = 0;

    constexpr unsigned int maxColumns = 3;

    auto line = new QFrame( );
    line->setFrameShape( QFrame::VLine );
    line->setFrameShadow( QFrame::Sunken );

    upperLayout->addWidget( new QLabel( "  GID" ) , 0 , currentColumn++ , 1 ,
                            1 );
    upperLayout->addWidget( line , 0 , currentColumn++ , 1 , 1 );
    upperLayout->addWidget( new QLabel( "# synapses" ) , 0 , currentColumn , 1 ,
                            1 );
    upperLayout->addWidget( scroll , 1 , 0 , 1 , maxColumns );

    currentColumn = 0;

    if ( presentSynapses != nullptr )
    {
      for ( const auto& syn: *presentSynapses )
      {
        currentColumn = 0;

        const auto gidLabel = new QLabel( QString::number( syn.first ));

        layoutWidget->addWidget( gidLabel , currentRow , currentColumn++ , 1 ,
                                 1 );

        auto vline = new QFrame( );
        vline->setFrameShape( QFrame::VLine );
        vline->setFrameShadow( QFrame::Sunken );

        layoutWidget->addWidget( vline , currentRow , currentColumn++ , 1 , 1 );

        const auto usedLabel = new QLabel( QString::number( syn.second ));

        layoutWidget->addWidget( usedLabel , currentRow , currentColumn , 1 ,
                                 1 );

        ++currentRow;
      }
    }

    _layoutInfo->addWidget( widget , widget == _widgetInfoPre ? 0 : 1 );
  }

  update( );
}

void MainWindow::clearInfoDock( void )
{
  std::function< void( QLayout* ) > clearLayout = [ &clearLayout ](
    QLayout* layout )
  {
    while ( layout->count( ) > 0 )
    {
      auto child = layout->takeAt( 0 );
      if ( child->layout( ) != 0 )
      {
        clearLayout( child->layout( ));
      }
      else if ( child->widget( ) != 0 )
      {
        delete child->widget( );
      }

      delete child;
    }
  };

  clearLayout( _layoutInfo );
  _widgetInfoPre = nullptr;
  _widgetInfoPost = nullptr;
}

void MainWindow::loadData( const std::string& dataset ,
                           const std::string& target )
{
  if ( m_thread )
  {
    QMessageBox msgBox{ this };
    msgBox.setWindowTitle( tr( "Data loading" ));
    msgBox.setIcon( QMessageBox::Icon::Information );
    msgBox.setText(
      tr( "A dataset is already loading. Cannot load: %1, target %2" )
        .arg( QString::fromStdString( dataset )
                .arg( QString::fromStdString( target ))));
    msgBox.exec( );
  }

  disableInterface( true );
  _ui->statusbar->showMessage(
    tr( "Loading %1" ).arg( QString::fromStdString( dataset )));

  m_thread = std::make_shared< LoadingThread >( dataset , target , this );
  auto dialog = new LoadingDialog{ this };

  connect( m_thread.get( ) , SIGNAL( finished( )) ,
           this , SLOT( onDataLoaded( )) , Qt::QueuedConnection );
  connect( m_thread.get( ) , SIGNAL( destroyed( QObject * )) ,
           dialog , SLOT( closeDialog( )) );
  connect( m_thread.get( ) ,
           SIGNAL( progress(
                     const QString & , const unsigned int)) ,
           dialog , SLOT( progress(
                            const QString & , const unsigned int)) );

  dialog->show( );
  _openGLWidget->doneCurrent( );

  m_thread->start( );
}

void MainWindow::loadPresynapticList( void )
{
  nsol::DataSet* nsolData = _openGLWidget->dataset( );

  _modelListPre->clear( );

  QList< QStandardItem* > items;
  for ( const auto& neuron: nsolData->neurons( ))
  {
    auto item = new QStandardItem( );
    item->setData( neuron.first , Qt::DisplayRole );

    items << item;
  }

  _modelListPre->appendColumn( items );
  _modelListPre->sort( 0 , Qt::AscendingOrder );

  update( );
}

void MainWindow::loadPostsynapticList( unsigned int gid )
{
  auto nsolData = _openGLWidget->dataset( );

  _modelListPost->clear( );

  std::set< unsigned int > selection = { gid };
  const auto synapses = nsolData->circuit( ).synapses( selection ,
                                                       nsol::Circuit::PRESYNAPTICCONNECTIONS );

  selection.clear( );

  QList< QStandardItem* > items;
  for ( const auto& syn: synapses )
  {
    unsigned int postGid = syn->postSynapticNeuron( );
    if ( selection.find( postGid ) != selection.end( ))
      continue;

    auto item = new QStandardItem( );
    item->setData( postGid , Qt::DisplayRole );

    items << item;

    selection.insert( postGid );
  }

  _modelListPost->appendColumn( items );
  _modelListPost->sort( 0 , Qt::SortOrder::AscendingOrder );

  update( );
}


void MainWindow::openBlueConfigThroughDialog( void )
{
  const QString filename = QFileDialog::getOpenFileName(
    this , tr( "Open BlueConfig" ) , _lastOpenedFileNamePath ,
    tr( "BlueConfig ( BlueConfig CircuitConfig);; All files (*)" ) ,
    nullptr , QFileDialog::DontUseNativeDialog );

  if ( !filename.isEmpty( ))
  {
    bool ok;
    const QString target = QInputDialog::getText(
      this , tr( "Select the target" ) ,
      tr( "Target:" ) , QLineEdit::Normal , QString( ) , &ok );

    if ( !ok ) return;

    _lastOpenedFileNamePath = QFileInfo( filename ).path( );
    loadData( filename.toStdString( ) , target.toStdString( ));
  }
}

void MainWindow::exportDataDialog( void )
{
  const QString xml = QFileDialog::getSaveFileName(
    this , tr( "Save scene..." ) , tr( "scene.xml" ) ,
    tr( "Extensible Markup Language (*.xml)" ) ,
    nullptr , QFileDialog::DontUseNativeDialog );

  if ( xml.isEmpty( )) return;

  const QString csv = QFileDialog::getSaveFileName(
    this , tr( "Save synapses..." ) , tr( "synapses.csv" ) ,
    tr( "Comma Separated Values (*.csv)" ) ,
    nullptr , QFileDialog::DontUseNativeDialog );

  if ( csv.isEmpty( )) return;

  const QStringList list{ "Aggregate" , "Compact" , "Matrix" };

  const QString option =
    csv.isEmpty( ) ?
    "" :
    QInputDialog::getItem( this , tr( "Select CSV type" ) ,
                           "CSV type:" , list , 0 , false );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  const auto dataset = _openGLWidget->dataset( );

  if ( !xml.isEmpty( ))
  {
    std::ofstream file;
    file.open( xml.toStdString( ) , std::ofstream::out );

    if ( file.good( ))
    {
      syncopa::toXML( dataset , file );
    }
    file.close( );
  }

  if ( !csv.isEmpty( ))
  {

    if ( !option.isEmpty( ))
    {
      std::ofstream file;
      file.open( csv.toStdString( ) , std::ofstream::out );
      if ( file.good( ))
      {
        if ( option == "Matrix" )
        {
          toMatrixCSV( dataset , file );
        }
        else if ( option == "Aggregate" )
        {
          toAggregateCSV( dataset , file );
        }
        else
        {
          toCompactCSV( dataset , file );
        }
      }
      file.close( );
    }
  }
  QApplication::restoreOverrideCursor( );
}

void MainWindow::syncScene( )
{
  std::stringstream xml;
  const auto dataset = _openGLWidget->dataset( );

  toXML( dataset , xml );
  auto xmlResult = QString::fromStdString( xml.str( ));

  std::stringstream csv;
  toAggregateCSV( dataset , csv );
  auto csvResult = QString::fromStdString( csv.str( ));

  _web_api.callSceneSyncEvent( xmlResult , csvResult );
}

void MainWindow::presynapticNeuronClicked( )
{
  const auto indexes = _listPresynaptic->selectionModel( )->selectedIndexes( );
  if ( indexes.empty( )) return;
  std::vector< unsigned int > selection;
  for ( const auto& item: _listPresynaptic->selectionModel( )->selectedIndexes( ))
  {
    selection.push_back( _modelListPre->itemFromIndex( item )->data(
      Qt::DisplayRole ).value< unsigned int >( ));
  }

  if ( _openGLWidget->mode( ) == syncopa::PATHS )
  {
    if ( selection.size( ) > 1 )
    {
      _modelListPost->clear( );
    }
    else
    {
      const auto gid = selection.front( );
      loadPostsynapticList( gid );
    }
  }

  // Generate cluster
  std::map< QString , std::unordered_set< unsigned int>> selections;

  std::unordered_set< unsigned int > pre( selection.begin( ) ,
                                          selection.end( ));
  auto connected = _openGLWidget->getDomainManager( )->connectedTo(
    selection.front( ));

  auto other = _openGLWidget->getGidsAll( );

  for ( const auto& item: pre )
  {
    connected.erase( item );
    other.erase( item );
  }

  for ( const auto& item: connected )
  {
    other.erase( item );
  }

  selections[ "Presynaptic" ] = pre;
  selections[ "Postsynaptic" ] = { };
  selections[ "Connected" ] = connected;
  selections[ "Other" ] = other;

  auto& metadata = _neuronClusterManager->getMetadata( "Main Group" );
  metadata.focusedSelection = "";

  auto& preMeta = metadata.selection[ "Presynaptic" ];
  preMeta.enabled = true;
  preMeta.synapsesVisibility = syncopa::SynapsesVisibility::ALL;
  preMeta.pathsVisibility = syncopa::PathsVisibility::ALL;
  preMeta.pathTypes = syncopa::PathTypes::ALL;

  auto& postMeta = metadata.selection[ "Postsynaptic" ];
  postMeta.enabled = true;
  postMeta.synapsesVisibility = syncopa::SynapsesVisibility::HIDDEN;
  postMeta.pathsVisibility = syncopa::PathsVisibility::HIDDEN;

  auto& connectedMeta = metadata.selection[ "Connected" ];
  connectedMeta.enabled = true;
  connectedMeta.synapsesVisibility = syncopa::SynapsesVisibility::HIDDEN;
  connectedMeta.pathsVisibility = syncopa::PathsVisibility::HIDDEN;

  auto& otherMeta = metadata.selection[ "Other" ];
  otherMeta.enabled = true;
  otherMeta.synapsesVisibility = syncopa::SynapsesVisibility::HIDDEN;
  otherMeta.pathsVisibility = syncopa::PathsVisibility::HIDDEN;

  _neuronClusterManager->addCluster(
    NeuronCluster( "Main Group" , selections ));

  //
  _groupBoxDynamic->setEnabled( _openGLWidget->mode( ) == syncopa::PATHS );
  dynamicStop( );

  updateInfoDock( );

  _openGLWidget->home( );

  if ( _web_socket )
  {
    if ( _openGLWidget->mode( ) == syncopa::SYNAPSES || selection.empty( ))
    {
      QJsonArray array;
      for ( const auto& item: selection )
      {
        array.push_back( QJsonValue( static_cast<int>(item)));
      }

      _web_api.callSynapsesModeSelectionEvent( array );
    }
    else
    {
      _web_api.callPathsModeSelectionEvent( selection.at( 0 ) , QJsonArray( ));
    }
  }
}

void MainWindow::postsynapticNeuronClicked( )
{
  std::vector< unsigned int > selection;
  for ( auto item: _listPostsynaptic->selectionModel( )->selectedIndexes( ))
  {
    selection.push_back( _modelListPost->itemFromIndex( item )->data(
      Qt::DisplayRole ).value< unsigned int >( ));
  }

  _groupBoxDynamic->setEnabled( true );
  dynamicStop( );
  updateInfoDock( );

  // Generate cluster

  std::map< QString , std::unordered_set< unsigned int>> selections;

  auto pre = _listPresynaptic->selectionModel( )->selection( )
    .first( ).indexes( ).first( ).data( ).value< unsigned int >( );

  std::unordered_set< unsigned int > post( selection.begin( ) ,
                                           selection.end( ));

  auto connected = selection.empty( )
                   ? std::unordered_set< unsigned int >( )
                   : _openGLWidget->getDomainManager( )->connectedTo(
      selection.front( ));

  auto other = _openGLWidget->getGidsAll( );

  post.erase( pre );
  connected.erase( pre );
  other.erase( pre );

  for ( const auto& item: post )
  {
    connected.erase( item );
    other.erase( item );
  }

  for ( const auto& item: connected )
  {
    other.erase( item );
  }

  selections[ "Presynaptic" ] = { pre };
  selections[ "Postsynaptic" ] = post;
  selections[ "Connected" ] = connected;
  selections[ "Other" ] = other;

  auto& metadata = _neuronClusterManager->getMetadata( "Main Group" );
  metadata.focusedSelection = "";

  auto& preMeta = metadata.selection[ "Presynaptic" ];
  preMeta.enabled = true;
  preMeta.synapsesVisibility = syncopa::SynapsesVisibility::CONNECTED_ONLY;
  preMeta.pathsVisibility = syncopa::PathsVisibility::CONNECTED_ONLY;
  preMeta.pathTypes = syncopa::PathTypes::PRE_ONLY;

  auto& postMeta = metadata.selection[ "Postsynaptic" ];
  postMeta.enabled = true;
  postMeta.synapsesVisibility = syncopa::SynapsesVisibility::CONNECTED_ONLY;
  postMeta.pathsVisibility = syncopa::PathsVisibility::CONNECTED_ONLY;
  postMeta.pathTypes = syncopa::PathTypes::POST_ONLY;

  auto& connectedMeta = metadata.selection[ "Connected" ];
  connectedMeta.enabled = false;
  connectedMeta.synapsesVisibility = syncopa::SynapsesVisibility::HIDDEN;
  connectedMeta.pathsVisibility = syncopa::PathsVisibility::HIDDEN;

  auto& otherMeta = metadata.selection[ "Other" ];
  otherMeta.enabled = false;
  otherMeta.synapsesVisibility = syncopa::SynapsesVisibility::HIDDEN;
  otherMeta.pathsVisibility = syncopa::PathsVisibility::HIDDEN;

  _neuronClusterManager->addCluster(
    NeuronCluster( "Main Group" , selections ));

  _openGLWidget->home( );

  if ( _web_socket )
  {
    QJsonArray array;
    for ( const auto& item: selection )
    {
      array.push_back( QJsonValue( static_cast<int>(item)));
    }
    _web_api.callPathsModeSelectionEvent( pre , array );
  }
}

void MainWindow::setSynapseMappingState( int state )
{
  _frameColorSynapsesPre->setEnabled( !state );
  _frameColorSynapsesPost->setEnabled( !state );
  _checkSynapsesPre->setEnabled( !state );
  _checkSynapsesPost->setEnabled( !state );
  _sliderAlphaSynapsesPre->setEnabled( !state );
  _sliderAlphaSynapsesPost->setEnabled( !state );
  _spinBoxSizeSynapsesPre->setEnabled( !state );
  _spinBoxSizeSynapsesPost->setEnabled( !state );

  _comboSynapseMapAttrib->setEnabled( state );
  _sliderAlphaSynapsesMap->setEnabled( state );
  _spinBoxSizeSynapsesMap->setEnabled( state );
  _colorMapWidget->setEnabled( state );

  _openGLWidget->setSynapseMappingState( state );

  auto rangeBounds = _openGLWidget->rangeBounds( );

  _colorMapWidget->setPlot( _openGLWidget->getSynapseMappingPlot( ) ,
                            rangeBounds.first ,
                            rangeBounds.second );

  if ( state ) colorSynapseMapAccepted( );
}

void MainWindow::setSynapseMappingAttribute( int attrib )
{
  _openGLWidget->setSynapseMapping( attrib );

  auto rangeBounds = _openGLWidget->rangeBounds( );

  _colorMapWidget->setPlot( _openGLWidget->getSynapseMappingPlot( ) ,
                            rangeBounds.first ,
                            rangeBounds.second );
}

void MainWindow::clear( void )
{
  std::map< QString , std::unordered_set< unsigned int>> selections;
  selections[ "Presynaptic" ] = { };
  selections[ "Postsynaptic" ] = { };
  selections[ "Connected" ] = { };
  selections[ "Other" ] = _openGLWidget->getGidsAll( );

  auto& metadata = _neuronClusterManager->getMetadata( "Main Group" );
  metadata.focusedSelection = "";

  auto& preMeta = metadata.selection[ "Presynaptic" ];
  preMeta.enabled = true;
  preMeta.synapsesVisibility = syncopa::SynapsesVisibility::HIDDEN;
  preMeta.pathsVisibility = syncopa::PathsVisibility::HIDDEN;

  auto& postMeta = metadata.selection[ "Postsynaptic" ];
  postMeta.enabled = true;
  postMeta.synapsesVisibility = syncopa::SynapsesVisibility::HIDDEN;
  postMeta.pathsVisibility = syncopa::PathsVisibility::HIDDEN;

  auto& connectedMeta = metadata.selection[ "Connected" ];
  connectedMeta.enabled = true;
  connectedMeta.synapsesVisibility = syncopa::SynapsesVisibility::HIDDEN;
  connectedMeta.pathsVisibility = syncopa::PathsVisibility::HIDDEN;

  auto& otherMeta = metadata.selection[ "Other" ];
  otherMeta.enabled = true;
  otherMeta.synapsesVisibility = syncopa::SynapsesVisibility::ALL;
  otherMeta.pathsVisibility = syncopa::PathsVisibility::HIDDEN;

  _listPresynaptic->blockSignals( true );
  _listPostsynaptic->blockSignals( true );

  _listPresynaptic->selectionModel( )->clearSelection( );
  if ( _modelListPost )
    _modelListPost->clear( );
  _listPresynaptic->blockSignals( false );
  _listPostsynaptic->blockSignals( false );

  _neuronClusterManager->addCluster(
    NeuronCluster( "Main Group" , selections ));

  updateInfoDock( );
  dynamicStop( );

  _openGLWidget->home( false );
}

bool MainWindow::showDialog( QColor& current , const QString& message )
{
  const QColor result = QColorDialog::getColor( current , this ,
                                                QString( message ) ,
                                                QColorDialog::DontUseNativeDialog );

  const auto valid = result.isValid( );
  if ( valid )
  {
    current = result;
  }

  return valid;
}

void MainWindow::colorSelectionClicked( void )
{
  auto frame = qobject_cast< QPushButton* >( sender( ));
  if ( frame )
  {
    QColor color;
    QString message;

    if ( frame == _frameColorSynapsesPre )
      color = colorEigenToQt( _openGLWidget->colorSynapsesPre( ));
    else if ( frame == _frameColorSynapsesPost )
      color = colorEigenToQt( _openGLWidget->colorSynapsesPost( ));
    else if ( frame == _frameColorPathsPre )
      color = colorEigenToQt( _openGLWidget->colorPathsPre( ));
    else if ( frame == _frameColorPathsPost )
      color = colorEigenToQt( _openGLWidget->colorPathsPost( ));
    else if ( frame == _frameColorDynamicPre )
      color = colorEigenToQt( _openGLWidget->colorDynamicPre( ));
    else if ( frame == _frameColorDynamicPost )
      color = colorEigenToQt( _openGLWidget->colorDynamicPost( ));
    else
    {
      std::cout << "Warning: Frame " << frame << " clicked not connected."
                << std::endl;
      return;
    }

    if ( showDialog( color , message ))
    {
      if ( frame == _frameColorSynapsesPre )
        _openGLWidget->colorSynapsesPre( colorQtToEigen( color ));
      else if ( frame == _frameColorSynapsesPost )
        _openGLWidget->colorSynapsesPost( colorQtToEigen( color ));
      else if ( frame == _frameColorPathsPre )
        _openGLWidget->colorPathsPre( colorQtToEigen( color ));
      else if ( frame == _frameColorPathsPost )
        _openGLWidget->colorPathsPost( colorQtToEigen( color ));
      else if ( frame == _frameColorDynamicPre )
        _openGLWidget->colorDynamicPre( colorQtToEigen( color ));
      else if ( frame == _frameColorDynamicPost )
        _openGLWidget->colorDynamicPost( colorQtToEigen( color ));
    }

    frame->setStyleSheet( "background-color: " + color.name( ));
  }
}

void MainWindow::transparencySliderMoved( int position )
{
  auto source = qobject_cast< QSlider* >( sender( ));

  const float normValue = ( position - SLIDER_MIN ) * INV_RANGE_SLIDERS;

  if ( source )
  {
    if ( source == _sliderAlphaSynapsesPre )
      _openGLWidget->alphaSynapsesPre( normValue );
    else if ( source == _sliderAlphaSynapsesPost )
      _openGLWidget->alphaSynapsesPost( normValue );
    else if ( source == _sliderAlphaPathsPre )
      _openGLWidget->alphaPathsPre( normValue );
    else if ( source == _sliderAlphaPathsPost )
      _openGLWidget->alphaPathsPost( normValue );
    else if ( source == _sliderAlphaSynapsesMap )
      _openGLWidget->alphaSynapseMap( _colorMapWidget->getColors( ) ,
                                      normValue );
  }
}

void MainWindow::sizeSpinBoxChanged( double value )
{
  auto source = qobject_cast< QDoubleSpinBox* >( sender( ));

  if ( source )
  {
    if ( source == _spinBoxSizeSynapsesPre )
      _openGLWidget->sizeSynapsesPre( value );
    else if ( source == _spinBoxSizeSynapsesPost )
      _openGLWidget->sizeSynapsesPost( value );
    else if ( source == _spinBoxSizePathsPre )
      _openGLWidget->sizePathsPre( value );
    else if ( source == _spinBoxSizePathsPost )
      _openGLWidget->sizePathsPost( value );
    else if ( source == _spinBoxSizeSynapsesMap )
      _openGLWidget->sizeSynapseMap( value );
  }
}

void MainWindow::colorSynapseMapAccepted( void )
{
  _openGLWidget->colorSynapseMap( _colorMapWidget->getColors( ));
}

void MainWindow::dynamicStart( void )
{
  if ( !_openGLWidget->dynamicActive( ))
  {
    _buttonDynamicStart->setText( "Pause" );
    _buttonDynamicStop->setEnabled( true );
    _openGLWidget->startDynamic( );
  }
  else
  {
    dynamicPause( );
  }
}

void MainWindow::dynamicPause( void )
{
  const auto state = _openGLWidget->toggleDynamicMovement( );

  const auto buttonText = state ? "Pause" : "Paused";
  _buttonDynamicStart->setText( buttonText );
}

void MainWindow::dynamicStop( void )
{
  if ( _openGLWidget->dynamicActive( ))
  {
    _buttonDynamicStop->setEnabled( false );
    _buttonDynamicStart->setText( "Start" );

    _openGLWidget->stopDynamic( );
  }
}

void MainWindow::filteringStateChanged( void )
{
  _openGLWidget->filteringState( _colorMapWidget->filter( ));
}

void MainWindow::filteringBoundsChanged( void )
{
  auto bounds = _colorMapWidget->filterBounds( );
  _openGLWidget->filteringBounds( bounds.first , bounds.second );
}

void MainWindow::modeChanged( bool selectedModeSynapses )
{
  const TMode mode = selectedModeSynapses ? syncopa::SYNAPSES : syncopa::PATHS;

  _openGLWidget->mode( mode );

  if ( _listPresynaptic->selectionModel( ))
  {
    const auto selectedPre = _listPresynaptic->selectionModel( )->selectedIndexes( );

    if ( selectedPre.size( ) != 1 )
    {
      _listPresynaptic->clearSelection( );
    }
    else if ( !selectedModeSynapses )
    {
      const auto item = _modelListPre->itemFromIndex( selectedPre.front( ));
      const unsigned int idx = item->data(
        Qt::DisplayRole ).value< unsigned int >( );

      loadPostsynapticList( idx );
    }
    else
    {
      _listPostsynaptic->clearSelection( );
    }
  }

  _groupBoxDynamic->setEnabled( !selectedModeSynapses );

  if ( selectedModeSynapses && _modelListPost )
    _modelListPost->clear( );

  _listPostsynaptic->setEnabled( !selectedModeSynapses );

  _groupBoxPaths->setEnabled( !selectedModeSynapses );

  _listPresynaptic->setSelectionMode( selectedModeSynapses ?
                                      QAbstractItemView::ExtendedSelection :
                                      QAbstractItemView::SingleSelection );

  updateInfoDock( );
}

void MainWindow::alphaModeChanged( bool state )
{
  _openGLWidget->alphaMode( !state );
}

void MainWindow::neuronClusterManagerStructureRefresh( )
{
  while ( auto* item = _sceneLayout->takeAt( 0 ))
  {
    if ( auto* widget = item->widget( ))
    {
      widget->deleteLater( );
    }
  }

  for ( const auto& item: _neuronClusterManager->getClusters( ))
  {
    auto& metadata = _neuronClusterManager->getMetadata( item.getName( ));
    _sceneLayout->addWidget(
      new NeuronClusterView( _neuronClusterManager , item , metadata ));
  }

  _openGLWidget->updateMorphologyModel( _neuronClusterManager );
  _openGLWidget->updateSynapsesModel( _neuronClusterManager );
  _openGLWidget->updatePathsModel( _neuronClusterManager );
}

void MainWindow::neuronClusterManagerMetadataRefresh( )
{
  _openGLWidget->updateMorphologyModel( _neuronClusterManager );
  _openGLWidget->updateSynapsesModel( _neuronClusterManager );
  _openGLWidget->updatePathsModel( _neuronClusterManager );
}

void MainWindow::aboutDialog( )
{
  QString message =
    QString( "<h2>Syncopa" ) + "<br>" +
    tr( "Version " ) + syncopa::Version::getString( ).c_str( ) +
    tr( " rev (%1)<br>" ).arg( syncopa::Version::getRevision( )) + "</h2><br>"
                                                                   "<a href='https://vg-lab.es/'>https://vg-lab.es/</a>" +
    "<h4>" + tr( "Build info:" ) + "</h4>" +
    "<ul><li>Qt " + QT_VERSION_STR +

    #ifdef SYNCOPA_USE_RETO
    "</li><li>ReTo " + RETO_REV_STRING +
    #else
    "</li><li>ReTo " + tr( "support not built." ) +
    #endif

    #ifdef SYNCOPA_USE_GMRVLEX
    "</li><li>GmrvLex " + GMRVLEX_REV_STRING +
    #else
    "</li><li>GmrvLex " + tr( "support not built." ) +
    #endif

    #ifdef SYNCOPA_USE_PREFR
    "</li><li>prefr " + PREFR_REV_STRING +
    #else
    "</li><li>prefr " + tr( "support not built." ) +
    #endif

    #ifdef SYNCOPA_USE_SCOOP
    "</li><li>Scoop " + SCOOP_REV_STRING +
    #else
    "</li><li>Scoop " + tr( "support not built." ) +
    #endif

    #ifdef SYNCOPA_USE_NEUROLOTS
    "</li><li>NeuroLOTs " + NLGENERATOR_REV_STRING +
    #else
    "</li><li>NeuroLOTs " + tr( "support not built." ) +
    #endif

    #ifdef SYNCOPA_USE_NSOL
    "</li><li>nsol " + NSOL_REV_STRING +
    #else
    "</li><li>nsol " + tr( "support not built." ) +
    #endif

    #ifdef SYNCOPA_USE_BRION
    "</li><li>Brion " + BRION_REV_STRING +
    #else
    "</li><li>Brion " + tr( "support not built." ) +
    #endif

    "</li></ul>" + "<h4>" + tr( "Developed by:" ) + "</h4>" +
    "VG-Lab / URJC / UPM"
    "<br><a href='https://vg-lab.es/'>https://vg-lab.es/</a>"
    "<br>(C) 2015-" +
    QString::number( QDateTime::currentDateTime( ).date( ).year( )) + "<br><br>"
                                                                      "<a href='https://vg-lab.es'><img src=':/icons/logoVGLab.png'/></a>"
                                                                      "&nbsp;&nbsp;&nbsp;&nbsp;"
                                                                      "<a href='https://www.urjc.es'><img src=':/icons/logoURJC.png' /></a>"
                                                                      "&nbsp;&nbsp;&nbsp;&nbsp;"
                                                                      "<a href='https://www.upm.es'><img src=':/icons/logoUPM.png' /></a>";

  QMessageBox::about( this , tr( "About Syncopa" ) , message );
}

void MainWindow::onConnectionButtonTriggered( bool value )
{
  auto action = qobject_cast< QAction* >( sender( ));

  if ( value )
  {
    if ( _web_socket ) return;

    bool ok = false;
    const auto title = tr( "Network connection" );
    const auto label = tr( "Port:" );
    const auto port = QInputDialog::getInt( this , title , label , 3000 , 0 ,
                                            65535 ,
                                            1 , &ok );
    if ( ok )
    {
      action->setIcon( QIcon( ":/icons/disconnect.svg" ));

      _web_socket = std::make_shared< wcw::WebClientManager >( port );
      _web_socket->registerAPI( "syncopa" , &_web_api );

      connect( this , &QObject::destroyed , [ this ]( )
      {
        _web_socket = nullptr;
      } );

      _web_socket->start( );
      _ui->actionSyncScene->setEnabled( true );
      _ui->actionNetworkSynchronization->setVisible( true );
      _ui->actionNetworkSynchronization->setChecked( false );
    }
    else
    {
      action->blockSignals( true );
      action->setChecked( false );
      action->blockSignals( false );
    }
  }
  else
  {
    if ( _web_socket )
    {
      onConnectionThreadTerminated( );
    }
  }
}

void MainWindow::onConnectionSynchronizationTriggered( bool value )
{
  _web_api.setSynchronizedMode( value );
  auto icon = QIcon( value ? ":/icons/sync.svg" : ":/icons/sync_off.svg" );
  _ui->actionNetworkSynchronization->setIcon( icon );
}

void MainWindow::onConnectionThreadTerminated( )
{
  _web_socket = nullptr;
  _ui->actionNetworkConnection->blockSignals( true );
  _ui->actionNetworkConnection->setChecked( false );
  _ui->actionNetworkConnection->setEnabled( true );
  _ui->actionNetworkConnection->setIcon( QIcon( ":/icons/connect.svg" ));
  _ui->actionNetworkConnection->blockSignals( false );
  _ui->actionSyncScene->setEnabled( false );
  _ui->actionNetworkSynchronization->setVisible( false );
}

void MainWindow::disableInterface( bool value )
{
  static bool current = false;

  if ( value == current ) return;

  current = value;

  _dockColor->setEnabled( !value );
  _dockInfo->setEnabled( !value );
  _dockList->setEnabled( !value );
  _ui->toolBar->setEnabled( !value );
  _ui->menubar->setEnabled( !value );

  if ( value )
    QApplication::setOverrideCursor( Qt::WaitCursor );
  else
    QApplication::restoreOverrideCursor( );
}

void LoadingThread::run( )
{
  try
  {
    if ( m_parent )
    {
      emit progress( tr( "Loading data" ) , 0 );

      connect( m_parent->_openGLWidget ,
               SIGNAL( progress(
                         const QString & , const unsigned int)) ,
               this , SIGNAL( progress(
                                const QString & , const unsigned int)) );

      m_parent->_openGLWidget->loadBlueConfig( m_blueconfig , m_target );
    }
  }
  catch ( const std::exception& e )
  {
    m_errors = std::string( e.what( ));
    m_errors +=
      std::string( "\n" ) + __FILE__ + " " + std::to_string( __LINE__ );
  }
  catch ( ... )
  {
    m_errors = std::string( "Unspecified exception." );
    m_errors +=
      std::string( "\n" ) + __FILE__ + " " + std::to_string( __LINE__ );
  }

  emit progress( tr( "Finished" ) , 100 );
}

LoadingDialog::LoadingDialog( QWidget* p )
  : QDialog( p , Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint )
{
  setWindowIcon( QIcon{ ":/neurotessmessh.png" } );

  auto layout = new QVBoxLayout( );
  m_progress = new QProgressBar( this );
  m_progress->setMinimumWidth( 590 );
  layout->addWidget( m_progress , 1 , Qt::AlignHCenter | Qt::AlignVCenter );
  layout->setMargin( 4 );
  setLayout( layout );

  setSizePolicy( QSizePolicy::MinimumExpanding ,
                 QSizePolicy::MinimumExpanding );
  setFixedSize( 600 , sizeHint( ).height( ));
}

void
LoadingDialog::progress( const QString& message , const unsigned int value )
{
  // NOTE: adjusted to the number of loading steps.
  constexpr float STEP_WEIGHT = 33.33;

  static float steps = 0;
  static unsigned int lastValue = 0;

  if ( lastValue > value ) ++steps;
  lastValue = value;

  const int progressValue = std::nearbyint(
    ( steps + static_cast<float>(value) / 100 ) * STEP_WEIGHT );

  m_progress->setValue( progressValue );
  if ( !message.isEmpty( ))
    m_progress->setFormat( tr( "%1 - %p%" ).arg( message ));
}

void LoadingDialog::closeDialog( )
{
  close( );
  deleteLater( );
}

void MainWindow::onConnectionError( )
{
  if ( _web_socket )
  {
    const auto title = tr( "Network error" );
    QMessageBox::critical( this , title , title , QMessageBox::Ok );

    _web_socket = nullptr;
  }
}

void MainWindow::manageSelectionEvent(
  const std::vector< unsigned int >& selection )
{
  std::vector< unsigned int > finalSelection;
  const int rows = _modelListPre->rowCount( );
  QItemSelection selectionSet;

  auto start = selection.cbegin( );
  auto end = selection.cend( );

  for ( int i = 0; i < rows; i++ )
  {
    const QModelIndex index = _modelListPre->index( i , 0 );
    const auto value = index.data( ).value< unsigned int >( );

    auto it = std::find( start , end , value );
    if ( it != end )
    {
      selectionSet.append( QItemSelectionRange( index ));
      finalSelection.push_back( value );
    }
  }

  _listPresynaptic->blockSignals( true );
  _listPresynaptic->selectionModel( )->select(
    selectionSet ,
    QItemSelectionModel::ClearAndSelect
  );
  _listPresynaptic->blockSignals( false );

  _groupBoxDynamic->setEnabled( false );
  dynamicStop( );
  updateInfoDock( );
}

void MainWindow::manageSynapsesSelectionEvent(
  const std::vector< unsigned int >& selection )
{
  if ( _openGLWidget->dataset( ) == nullptr ) return;
  if ( _openGLWidget->mode( ) != syncopa::SYNAPSES )
  {
    _radioModeSynapses->setChecked( true );
  }

  manageSelectionEvent( selection );
}

void MainWindow::managePathsSelectionEvent(
  unsigned int preSelection ,
  const std::vector< unsigned int >& postSelection )
{
  if ( _openGLWidget->dataset( ) == nullptr ) return;
  if ( _openGLWidget->mode( ) != syncopa::PATHS )
  {
    _radioModePaths->setChecked( true );
  }

  std::vector< unsigned int > finalSelection;
  const int rows = _modelListPre->rowCount( );
  QItemSelection pre;
  auto start = postSelection.cbegin( );
  auto end = postSelection.cend( );

  for ( int i = 0; i < rows; i++ )
  {
    const QModelIndex index = _modelListPre->index( i , 0 );
    const auto value = index.data( ).value< unsigned int >( );
    if ( value == preSelection )
    {
      pre.append( QItemSelectionRange( index ));
    }
  }

  _listPresynaptic->selectionModel( )->blockSignals( true );
  _listPostsynaptic->selectionModel( )->blockSignals( true );
  _listPresynaptic->selectionModel( )->select(
    pre ,
    QItemSelectionModel::ClearAndSelect
  );

  loadPostsynapticList( preSelection );

  QItemSelection post;
  for ( int i = 0; i < rows; i++ )
  {
    const QModelIndex index = _modelListPost->index( i , 0 );
    const auto value = index.data( ).value< unsigned int >( );

    auto it = std::find( start , end , value );
    if ( it != end )
    {
      post.append( QItemSelectionRange( index ));
      finalSelection.push_back( value );
    }
  }

  _listPostsynaptic->selectionModel( )->select(
    post ,
    QItemSelectionModel::ClearAndSelect
  );

  _listPresynaptic->selectionModel( )->blockSignals( false );
  _listPostsynaptic->selectionModel( )->blockSignals( false );

  _groupBoxDynamic->setEnabled( true );

  dynamicStop( );
  updateInfoDock( );
}

const std::shared_ptr< syncopa::NeuronClusterManager >&
MainWindow::getNeuronClusterManager( ) const
{
  return _neuronClusterManager;
}

void MainWindow::filteringPaletteChanged( void )
{
  const float normValue =
    ( _sliderAlphaSynapsesMap->value( ) - SLIDER_MIN ) * INV_RANGE_SLIDERS;
  _openGLWidget->alphaSynapseMap( _colorMapWidget->getColors( ) ,
                                  normValue );
}
