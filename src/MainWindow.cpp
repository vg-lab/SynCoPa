/*
 * @file  MainWindow.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "MainWindow.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QGridLayout>
#include <QScrollArea>
#include <QColorDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QDateTime>
#include <QSurface>
#include <QUrl>

#include <syncopa/version.h>

#ifdef SYNCOPA_USE_GMRVLEX
#include <gmrvlex/gmrvlex.h>
#endif

#include <thread>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

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
  return QColor( std::min( 255, std::max( 0, int( color_.x( ) * 255 ))),
                 std::min( 255, std::max( 0, int( color_.y( ) * 255 ))),
                 std::min( 255, std::max( 0, int( color_.z( ) * 255 ))));
}

MainWindow::MainWindow(
                        QWidget* parent_,
                        bool updateOnIdle,
                        bool fps)
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
, _frameColorMorphoPre( nullptr )
, _frameColorMorphoPost( nullptr )
, _frameColorMorphoContext( nullptr )
, _frameColorMorphoOther( nullptr )
, _frameColorSynapsesPre( nullptr )
, _frameColorSynapsesPost( nullptr )
, _frameColorPathsPre( nullptr )
, _frameColorPathsPost( nullptr )
, _buttonShowFullMorphoPre( nullptr )
, _buttonShowFullMorphoPost( nullptr )
, _buttonShowFullMorphoContext( nullptr )
, _buttonShowFullMorphoOther( nullptr )
, _checkShowMorphoPre( nullptr )
, _checkShowMorphoPost( nullptr )
, _checkShowMorphoContext( nullptr )
, _checkShowMorphoOther( nullptr )
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
, _invRangeSliders( 0.0f )
, _sliderMin( 0 )
, _sliderMax( 100 )
, _spinBoxSizeSynapsesPre( nullptr )
, _spinBoxSizeSynapsesPost( nullptr )
, _spinBoxSizePathsPre( nullptr )
, _spinBoxSizePathsPost( nullptr )
, _spinBoxSizeSynapsesMap( nullptr )
, _labelTransSynPre( nullptr )
, _labelTransSynPost( nullptr )
, _labelTransPathPre( nullptr )
, _labelTransPathPost( nullptr )
, _labelTransSynMap( nullptr )
, _buttonDynamicStart( nullptr )
, _buttonDynamicStop( nullptr )
, _comboSynapseMapAttrib( nullptr )
, _groupBoxGeneral( nullptr )
, _groupBoxMorphologies( nullptr )
, _groupBoxSynapses( nullptr )
, _groupBoxPaths( nullptr )
, _groupBoxDynamic( nullptr )
, m_thread{ nullptr }
, m_socket(nullptr)
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

  connect(_ui->actionNetworkConnection, SIGNAL(triggered(bool)),
          this, SLOT(onConnectionButtonTriggered(bool)));

  _invRangeSliders = 1.0 / ( _sliderMax - _sliderMin );

  _openGLWidget = new OpenGLWidget( nullptr, Qt::WindowFlags() );
  setCentralWidget( _openGLWidget );

  if ( _openGLWidget->format( ).version( ).first < 4 )
  {
    std::cerr << "This application requires at least OpenGL 4.0" << std::endl;
    exit( -1 );
  }
}

MainWindow::~MainWindow( void )
{
  delete _ui;
}

void MainWindow::init()
{
  _openGLWidget->createParticleSystem( );

  initListDock( );
  initColorDock( );
  initInfoDock( );

  tabifyDockWidget(_dockList, _dockColor);
  tabifyDockWidget(_dockColor, _dockInfo);

  _openGLWidget->idleUpdate( _ui->actionUpdateOnIdle->isChecked( ));
  _openGLWidget->showFps( _ui->actionShowFPSOnIdleUpdate->isChecked( ));

  connect( _ui->actionUpdateOnIdle , SIGNAL( triggered( )) ,
           _openGLWidget , SLOT( toggleUpdateOnIdle( )) );

  connect( _ui->actionBackgroundColor , SIGNAL( triggered( )) ,
           _openGLWidget , SLOT( changeClearColor( )) );

  connect( _ui->actionShowFPSOnIdleUpdate , SIGNAL( triggered( )) ,
           _openGLWidget , SLOT( toggleShowFPS( )) );

  connect( _ui->actionAbout , SIGNAL( triggered( )),
           this, SLOT( aboutDialog( )) );

  _radioModeSynapses->setChecked( true );

  _loadDefaultValues( );
}

void MainWindow::initListDock( void )
{
  _radioModeSynapses = new QRadioButton( tr( "Synapses" ));
  _radioModePaths = new QRadioButton( tr( "Paths" ));

  QGroupBox* groupSelection = new QGroupBox( "Selection mode" );
  QHBoxLayout* selectionLayout = new QHBoxLayout( );

  selectionLayout->addWidget( _radioModeSynapses );
  selectionLayout->addWidget( _radioModePaths );

  groupSelection->setLayout( selectionLayout );

  _listPresynaptic = new QListView( );
  _listPresynaptic->setMaximumWidth( 150 );
  _listPresynaptic->setEditTriggers( QAbstractItemView::NoEditTriggers );

  _listPostsynaptic = new QListView( );
  _listPostsynaptic->setMaximumWidth( 150 );
  _listPostsynaptic->setSelectionMode( QAbstractItemView::ExtendedSelection );
  _listPostsynaptic->setEditTriggers( QAbstractItemView::NoEditTriggers );

  _dockList = new QDockWidget( tr( "Selection" ));
  _dockList->setMaximumHeight( 500 );

  auto dockLayout = new QGridLayout( );
  dockLayout->setAlignment( Qt::AlignTop );

  auto container = new QWidget( );
  container->setLayout( dockLayout );

  auto clearButton = new QPushButton( "Clear selection" );
  connect( clearButton, SIGNAL( clicked( void )),
           this, SLOT( clear( void )));

  dockLayout->addWidget( groupSelection, 0, 0, 1, 4 );
  dockLayout->addWidget( new QLabel( "Presynaptic:" ), 1, 0, 1, 2 );
  dockLayout->addWidget( new QLabel( "Postsynaptic:" ), 1, 2, 1, 2 );
  dockLayout->addWidget( _listPresynaptic, 2, 0, 6, 2 );
  dockLayout->addWidget( _listPostsynaptic, 2, 2, 6, 2 );
  dockLayout->addWidget( clearButton );

  _dockList->setWidget( container );

  connect( _radioModeSynapses, SIGNAL( toggled( bool )),
           this, SLOT( modeChanged( bool )));

  _modelListPre = new QStandardItemModel();
  _listPresynaptic->setModel(_modelListPre);

  connect( _listPresynaptic->selectionModel(), SIGNAL( selectionChanged( const QItemSelection &, const QItemSelection &)),
           this, SLOT( presynapticNeuronClicked( )));

  _modelListPost = new QStandardItemModel();
  _listPostsynaptic->setModel(_modelListPost);

  connect( _listPostsynaptic->selectionModel(), SIGNAL( selectionChanged( const QItemSelection &, const QItemSelection &)),
           this, SLOT( postsynapticNeuronClicked( )));

  addDockWidget( Qt::RightDockWidgetArea, _dockList );
}

void MainWindow::initColorDock( void )
{
  _dockColor = new QDockWidget( "Visual configuration" );

  auto scrollArea = new QScrollArea( );
  auto layout = new QVBoxLayout( );
  auto container = new QWidget( );
  container->setLayout( layout );

  _groupBoxGeneral = new QGroupBox( "Alpha blending" );
  _groupBoxMorphologies = new QGroupBox( "Morphologies" );
  _groupBoxSynapses = new QGroupBox( "Synapses" );
  _groupBoxPaths = new QGroupBox( "Paths" );
  _groupBoxDynamic = new QGroupBox( "Dynamic" );

  _radioAlphaModeNormal = new QRadioButton( "Normal" );
  _radioAlphaModeAccumulative = new QRadioButton( "Accumulative" );

  _checkShowMorphoPre = new QCheckBox( "Presynaptic" );
  _checkShowMorphoPost = new QCheckBox( "Postsynaptic" );
  _checkShowMorphoContext = new QCheckBox( "Connected" );
  _checkShowMorphoOther = new QCheckBox( "Other" );
  _checkSynapsesPre = new QCheckBox( "Presynaptic" );
  _checkSynapsesPost = new QCheckBox( "Postsynaptic" );
  _checkPathsPre = new QCheckBox( "Presynaptic" );
  _checkPathsPost = new QCheckBox( "Postsynaptic" );

  _checkShowMorphoPre->setChecked( true );
  _checkShowMorphoPost->setChecked( true );
  _checkShowMorphoContext->setChecked( true );
  _checkShowMorphoOther->setChecked( true );
  _checkSynapsesPre->setChecked( true );
  _checkSynapsesPost->setChecked( true );
  _checkPathsPre->setChecked( true );
  _checkPathsPost->setChecked( true );

  auto layoutGeneral = new QVBoxLayout( );
  auto containerGeneral = new QWidget( );
  containerGeneral->setLayout( layoutGeneral );
  layoutGeneral->addWidget( _groupBoxGeneral );

  auto generalLayout = new QHBoxLayout( );
  generalLayout->setAlignment(  Qt::AlignTop );
  _groupBoxGeneral->setLayout( generalLayout );

  auto containerMorpho = new QWidget( );
  auto layoutMorpho = new QVBoxLayout( );
  containerMorpho->setLayout( layoutMorpho );
  layoutMorpho->addWidget( _groupBoxMorphologies );

  auto morphoLayout = new QGridLayout( );
  morphoLayout->setAlignment(  Qt::AlignTop );
  _groupBoxMorphologies->setLayout( morphoLayout );

  auto layoutSynapses = new QVBoxLayout( );
  auto containerSynapses = new QWidget( );
  containerSynapses->setLayout( layoutSynapses );
  layoutSynapses->addWidget( _groupBoxSynapses );

  auto synLayout = new QGridLayout( );
  synLayout->setAlignment(  Qt::AlignTop );
  _groupBoxSynapses->setLayout( synLayout );

  auto layoutPaths = new QVBoxLayout( );
  auto containerPaths = new QWidget( );
  containerPaths->setLayout( layoutPaths );

  layoutPaths->addWidget( _groupBoxPaths );
  layoutPaths->addWidget( _groupBoxDynamic );

  auto pathLayout = new QGridLayout( );
  pathLayout->setAlignment(  Qt::AlignTop );
  _groupBoxPaths->setLayout( pathLayout );

  auto layoutDynamic = new QGridLayout( );
  layoutDynamic->setAlignment( Qt::AlignTop );
  _groupBoxDynamic->setLayout( layoutDynamic );

  QPalette palette;
  QColor color;

  _frameColorMorphoPre = new QPushButton( );
  _frameColorMorphoPre->setFixedSize( 20, 20 );

  color = colorEigenToQt( _openGLWidget->colorSelectedPre( ));
  _frameColorMorphoPre->setStyleSheet( "background-color: " + color.name( ));

  _frameColorMorphoPost = new QPushButton( );
  _frameColorMorphoPost->setFixedSize( 20, 20 );

  color = colorEigenToQt( _openGLWidget->colorSelectedPost( ));
  _frameColorMorphoPost->setStyleSheet( "background-color: " + color.name( ));

  _frameColorMorphoContext = new QPushButton( );
  _frameColorMorphoContext->setFixedSize( 20, 20 );

  color = colorEigenToQt(_openGLWidget->colorRelated( ));
  _frameColorMorphoContext->setStyleSheet( "background-color: " + color.name( ));

  _frameColorMorphoOther = new QPushButton( );
  _frameColorMorphoOther->setFixedSize( 20, 20 );

  color = colorEigenToQt( _openGLWidget->colorContext( ));
  _frameColorMorphoOther->setStyleSheet( "background-color: " + color.name( ));

  _frameColorSynapsesPre = new QPushButton( );
  _frameColorSynapsesPre->setFixedSize( 20, 20 );

  color = colorEigenToQt( _openGLWidget->colorSynapsesPre( ));
  _frameColorSynapsesPre->setStyleSheet( "background-color: " + color.name( ));

  _frameColorSynapsesPost = new QPushButton( );
  _frameColorSynapsesPost->setFixedSize( 20, 20 );

  color = colorEigenToQt( _openGLWidget->colorSynapsesPost( ));
  _frameColorSynapsesPost->setStyleSheet( "background-color: " + color.name( ));

  _frameColorPathsPre = new QPushButton( );
  _frameColorPathsPre->setFixedSize( 20, 20 );

  color = colorEigenToQt( _openGLWidget->colorPathsPre( ));
  _frameColorPathsPre->setStyleSheet( "background-color: " + color.name( ));

  _frameColorPathsPost = new QPushButton( );
  _frameColorPathsPost->setFixedSize( 20, 20 );

  color = colorEigenToQt( _openGLWidget->colorPathsPost( ));
  _frameColorPathsPost->setStyleSheet( "background-color: " + color.name( ));

  QIcon neuronIcon;
  neuronIcon.addFile(QStringLiteral(":/icons/neurotessmesh.png"), QSize(), QIcon::Normal, QIcon::On);
  neuronIcon.addFile(QStringLiteral(":/icons/neurolots.png"), QSize(), QIcon::Normal, QIcon::Off);

  _buttonShowFullMorphoPre = new QPushButton( neuronIcon, "" );
  _buttonShowFullMorphoPre->setFixedSize( 20, 20 );
  _buttonShowFullMorphoPre->setCheckable( true );

  _buttonShowFullMorphoPost = new QPushButton( neuronIcon, "" );
  _buttonShowFullMorphoPost->setFixedSize( 20, 20 );
  _buttonShowFullMorphoPost->setCheckable( true );

  _buttonShowFullMorphoContext = new QPushButton( neuronIcon, "" );
  _buttonShowFullMorphoContext->setFixedSize( 20, 20 );
  _buttonShowFullMorphoContext->setCheckable( true );

  _buttonShowFullMorphoOther = new QPushButton( neuronIcon, "" );
  _buttonShowFullMorphoOther->setFixedSize( 20, 20 );
  _buttonShowFullMorphoOther->setCheckable( true );

  // General controls
  generalLayout->addWidget( _radioAlphaModeNormal );
  generalLayout->addWidget( _radioAlphaModeAccumulative );

  // Morphologies
  unsigned int row = 0;
  unsigned int col = 0;

  morphoLayout->addWidget( _frameColorMorphoPre, row, col++, 1, 1 );
  morphoLayout->addWidget( _buttonShowFullMorphoPre, row, col++, 1, 1 );
  morphoLayout->addWidget( _checkShowMorphoPre, row, col++, 1, 2 );

  ++col;

  morphoLayout->addWidget( _frameColorMorphoPost, row, col++, 1, 1 );
  morphoLayout->addWidget( _buttonShowFullMorphoPost, row, col++, 1, 1 );
  morphoLayout->addWidget( _checkShowMorphoPost, row, col++, 1, 2 );


  ++row;
  col = 0;

  morphoLayout->addWidget( _frameColorMorphoContext, row, col++, 1, 1 );
  morphoLayout->addWidget( _buttonShowFullMorphoContext, row, col++, 1, 1 );
  morphoLayout->addWidget( _checkShowMorphoContext, row, col++, 1, 2 );

  ++col;

  morphoLayout->addWidget( _frameColorMorphoOther, row, col++, 1, 1 );
  morphoLayout->addWidget( _buttonShowFullMorphoOther, row, col++, 1, 1 );
  morphoLayout->addWidget( _checkShowMorphoOther, row, col++, 1, 2 );

  // Synapses
  _colorMapWidget = new PaletteColorWidget( );
  _colorMapWidget->init( false );

  connect( _colorMapWidget, SIGNAL( acceptClicked( void )),
           this, SLOT( colorSynapseMapAccepted( void )));

  connect( _colorMapWidget, SIGNAL( filterStateChanged( void )),
           this, SLOT( filteringStateChanged( void )));

  connect( _colorMapWidget, SIGNAL( filterBoundsChanged( void )),
             this, SLOT( filteringBoundsChanged( void )));

  _sliderAlphaSynapsesPre = new QSlider( Qt::Horizontal );
  _sliderAlphaSynapsesPre->setRange(_sliderMin , _sliderMax);

  _sliderAlphaSynapsesPost = new QSlider( Qt::Horizontal );
  _sliderAlphaSynapsesPost->setRange(_sliderMin , _sliderMax);

  _sliderAlphaPathsPre = new QSlider( Qt::Horizontal );
  _sliderAlphaPathsPre->setRange(_sliderMin , _sliderMax);

  _sliderAlphaPathsPost = new QSlider( Qt::Horizontal );
  _sliderAlphaPathsPost->setRange(_sliderMin , _sliderMax);

  _sliderAlphaSynapsesMap = new QSlider( Qt::Horizontal );
  _sliderAlphaSynapsesMap->setRange(_sliderMin , _sliderMax);

  _labelTransSynPre = new QLabel( );
  _labelTransSynPost = new QLabel( );
  _labelTransPathPre = new QLabel( );
  _labelTransPathPost = new QLabel( );
  _labelTransSynMap = new QLabel( );

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

  QStringList optionList = { "Delay",
                             "Conductance",
                             "Utilization",
                             "Depression",
                             "Facilitation",
                             "Decay",
                             "Efficacy",
                             "Synapse type" };

  _comboSynapseMapAttrib->addItems( optionList );

  auto line = new QFrame( );
  line->setFrameShape( QFrame::HLine );
  line->setFrameShadow( QFrame::Sunken );

  row = 0;
  col = 0;

  synLayout->addWidget( _frameColorSynapsesPre, row, col++, 1, 1 );
  synLayout->addWidget( _checkSynapsesPre, row, col++, 1, 2 );

  ++col;
  synLayout->addWidget( _spinBoxSizeSynapsesPre, row, col++, 1, 1 );
  synLayout->addWidget( _sliderAlphaSynapsesPre, row, col, 1, 2 );

  ++row;
  col = 0;

  synLayout->addWidget( _frameColorSynapsesPost, row, col++, 1, 1 );
  synLayout->addWidget( _checkSynapsesPost, row, col++, 1, 2 );

  ++col;
  synLayout->addWidget( _spinBoxSizeSynapsesPost, row, col++, 1, 1 );
  synLayout->addWidget( _sliderAlphaSynapsesPost, row, col, 1, 2 );

  ++row;
  col = 0;

  synLayout->addWidget( line, row, col, 1, 6 );

  ++row;
  synLayout->addWidget( checkMapSynapses, row, col++, 1, 4 );
  ++col;
  ++col;
  ++col;
  synLayout->addWidget( _comboSynapseMapAttrib, row, col, 1, 2 );

  ++row;
  col = 0;

  synLayout->addWidget( _spinBoxSizeSynapsesMap, row, col++, 1, 2 );
  ++col;
  synLayout->addWidget( _sliderAlphaSynapsesMap, row, col, 1, 2 );

  ++row;
  col = 0;

  synLayout->addWidget( _colorMapWidget, row, col, 4, 6 );

  // Paths
  row = 0;
  col = 0;

  pathLayout->addWidget( _frameColorPathsPre, row, col++, 1, 1 );
  pathLayout->addWidget( _checkPathsPre, row, col++, 1, 2 );

  ++col;
  pathLayout->addWidget( _spinBoxSizePathsPre, row, col++, 1, 1 );
  pathLayout->addWidget( _sliderAlphaPathsPre, row, col, 1, 2 );

  ++row;
  col = 0;

  pathLayout->addWidget( _frameColorPathsPost, row, col++, 1, 1 );
  pathLayout->addWidget( _checkPathsPost, row, col++, 1, 2 );

  ++col;
  pathLayout->addWidget( _spinBoxSizePathsPost, row, col++, 1, 1 );
  pathLayout->addWidget( _sliderAlphaPathsPost, row, col, 1, 2 );

  _buttonDynamicStart = new QPushButton( "Start" );
  _buttonDynamicStop = new QPushButton( "Stop" );
  layoutDynamic->addWidget( _buttonDynamicStart, 0, 0, 1, 1 );
  layoutDynamic->addWidget( _buttonDynamicStop, 0, 1, 1, 1 );

  auto tabsWidget = new QTabWidget( );
  tabsWidget->setTabPosition( QTabWidget::West );
  tabsWidget->addTab( containerGeneral, "General" );
  tabsWidget->addTab( containerMorpho, "Morphologies" );
  tabsWidget->addTab( containerSynapses, "Synapses" );
  tabsWidget->addTab( containerPaths, "Paths" );

  scrollArea->setWidget( container );
  scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

  _dockColor->setWidget( tabsWidget );
  addDockWidget( Qt::RightDockWidgetArea, _dockColor );

  // SLOTS
  connect( _radioAlphaModeNormal, SIGNAL( toggled( bool )),
           this, SLOT( alphaModeChanged( bool )));

  connect( _buttonDynamicStart, SIGNAL( clicked( )), this, SLOT( dynamicStart( )));
  connect( _buttonDynamicStop, SIGNAL( clicked( )), this, SLOT( dynamicStop( )));

  connect( _frameColorMorphoPre, SIGNAL( clicked( )),
           this, SLOT( colorSelectionClicked()));

  connect( _frameColorMorphoPost, SIGNAL( clicked( )),
           this, SLOT( colorSelectionClicked()));

  connect( _frameColorMorphoContext, SIGNAL( clicked( )),
           this, SLOT( colorSelectionClicked()));

  connect( _frameColorMorphoOther, SIGNAL( clicked( )),
           this, SLOT( colorSelectionClicked()));

  connect( _frameColorSynapsesPre, SIGNAL( clicked( )),
           this, SLOT( colorSelectionClicked()));

  connect( _frameColorSynapsesPost, SIGNAL( clicked( )),
           this, SLOT( colorSelectionClicked()));

  connect( _frameColorPathsPre, SIGNAL( clicked( )),
           this, SLOT( colorSelectionClicked()));

  connect( _frameColorPathsPost, SIGNAL( clicked( )),
           this, SLOT( colorSelectionClicked()));

  connect(  _checkShowMorphoPre, SIGNAL( stateChanged( int )),
           _openGLWidget, SLOT( showSelectedPre( int )));
  connect(  _checkShowMorphoPost, SIGNAL( stateChanged( int )),
           _openGLWidget, SLOT( showSelectedPost( int )));
  connect(  _checkShowMorphoContext, SIGNAL( stateChanged( int )),
           _openGLWidget, SLOT( showRelated( int )));
  connect(  _checkShowMorphoOther, SIGNAL( stateChanged( int )),
           _openGLWidget, SLOT( showContext( int )));
  connect(  _checkSynapsesPre, SIGNAL( stateChanged( int )),
           _openGLWidget, SLOT( showSynapsesPre( int )));
  connect(  _checkSynapsesPost, SIGNAL( stateChanged( int )),
           _openGLWidget, SLOT( showSynapsesPost( int )));
  connect(  _checkPathsPre, SIGNAL( stateChanged( int )),
           _openGLWidget, SLOT( showPathsPre( int )));
  connect(  _checkPathsPost, SIGNAL( stateChanged( int )),
           _openGLWidget, SLOT( showPathsPost( int )));

  connect( _buttonShowFullMorphoPre, SIGNAL( toggled( bool )),
           this, SLOT( showFullMorphologyChecked( bool )));
  connect( _buttonShowFullMorphoPost, SIGNAL( toggled( bool )),
             this, SLOT( showFullMorphologyChecked( bool )));
  connect( _buttonShowFullMorphoContext, SIGNAL( toggled( bool )),
             this, SLOT( showFullMorphologyChecked( bool )));
  connect( _buttonShowFullMorphoOther, SIGNAL( toggled( bool )),
             this, SLOT( showFullMorphologyChecked( bool )));

  connect( _sliderAlphaSynapsesPre, SIGNAL( valueChanged( int )),
           this, SLOT( transparencySliderMoved( int )));
  connect( _sliderAlphaSynapsesPost, SIGNAL( valueChanged( int )),
           this, SLOT( transparencySliderMoved( int )));
  connect( _sliderAlphaPathsPre, SIGNAL( valueChanged( int )),
           this, SLOT( transparencySliderMoved( int )));
  connect( _sliderAlphaPathsPost, SIGNAL( valueChanged( int )),
           this, SLOT( transparencySliderMoved( int )));
  connect( _sliderAlphaSynapsesMap, SIGNAL( valueChanged( int )),
           this, SLOT( transparencySliderMoved( int )));

  connect( _spinBoxSizeSynapsesPre, SIGNAL( valueChanged( double )),
           this, SLOT( sizeSpinBoxChanged( double )));
  connect( _spinBoxSizeSynapsesPost, SIGNAL( valueChanged( double )),
           this, SLOT( sizeSpinBoxChanged( double )));
  connect( _spinBoxSizePathsPre, SIGNAL( valueChanged( double )),
           this, SLOT( sizeSpinBoxChanged( double )));
  connect( _spinBoxSizePathsPost, SIGNAL( valueChanged( double )),
           this, SLOT( sizeSpinBoxChanged( double )));
  connect( _spinBoxSizeSynapsesMap, SIGNAL( valueChanged( double )),
           this, SLOT( sizeSpinBoxChanged( double )));

  checkMapSynapses->setChecked( true );
  connect( checkMapSynapses, SIGNAL( stateChanged( int )),
           this, SLOT( setSynapseMappingState( int )));

  connect( _comboSynapseMapAttrib, SIGNAL( currentIndexChanged( int )),
           this, SLOT( setSynapseMappingAttribute( int )));

  _radioAlphaModeNormal->setChecked( true );

  checkMapSynapses->setChecked( false );

  _buttonShowFullMorphoPre->setChecked( true );
  _buttonShowFullMorphoPost->setChecked( true );
  _buttonShowFullMorphoContext->setChecked( false );
  _buttonShowFullMorphoOther->setChecked( false );
}

void MainWindow::initInfoDock( void )
{
  _dockInfo = new QDockWidget( tr( "Information" ));
  _dockInfo->setMinimumHeight(100);

  auto container = new QWidget( );

  _layoutInfo = new QVBoxLayout( );
  container->setLayout( _layoutInfo );

  auto scrollInfo = new QScrollArea( );
  scrollInfo->setHorizontalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  scrollInfo->setVerticalScrollBarPolicy( Qt::ScrollBarAsNeeded );
  scrollInfo->setWidget( container );
  scrollInfo->setWidgetResizable( true );

  _dockInfo->setWidget( scrollInfo );

  addDockWidget( Qt::RightDockWidgetArea, _dockInfo );
}

void MainWindow::onDataLoaded()
{
  _ui->statusbar->clearMessage();

  if(m_thread)
  {
    const auto errors = m_thread->errors();
    if(!errors.empty())
    {
      QApplication::restoreOverrideCursor();
      _ui->toolBar->setEnabled(true);

      m_thread = nullptr;

      QMessageBox msgBox(this);
      msgBox.setWindowTitle(tr("Error loading data"));
      msgBox.setIcon(QMessageBox::Icon::Critical);
      msgBox.setText(QString::fromStdString(errors));
      msgBox.exec();

      return;
    }
  }

  _openGLWidget->loadPostprocess( );

  loadPresynapticList( );

  disableInterface( false );

  m_thread = nullptr;
}

void MainWindow::_loadDefaultValues( void )
{
  const int range = _sliderMax - _sliderMin;

  int value = _openGLWidget->alphaSynapsesPre( ) * range + _sliderMin;
  value = std::min( _sliderMax, std::max( _sliderMin, value ));

  _sliderAlphaSynapsesPre->setSliderPosition( value );

  value = _openGLWidget->alphaSynapsesPost( ) * range + _sliderMin;
  value = std::min( _sliderMax, std::max( _sliderMin, value ));

  _sliderAlphaSynapsesPost->setSliderPosition( value );

  value = _openGLWidget->alphaPathsPre( ) * range + _sliderMin;
  value = std::min( _sliderMax, std::max( _sliderMin, value ));

  _sliderAlphaPathsPre->setSliderPosition( value );

  value = _openGLWidget->alphaPathsPost( ) * range + _sliderMin;
  value = std::min( _sliderMax, std::max( _sliderMin, value ));

  _sliderAlphaPathsPost->setSliderPosition( value );

  value = _openGLWidget->alphaSynapsesMap( ) * range + _sliderMin;
  value = std::min( _sliderMax, std::max( _sliderMin, value ));

  _sliderAlphaSynapsesMap->setSliderPosition( value );

  _openGLWidget->colorSynapseMap( _colorMapWidget->getColors( ));

  _spinBoxSizeSynapsesPre->setValue( _openGLWidget->sizeSynapsesPre( ));
  _spinBoxSizeSynapsesPost->setValue( _openGLWidget->sizeSynapsesPost( ));
  _spinBoxSizePathsPre->setValue( _openGLWidget->sizePathsPre( ));
  _spinBoxSizePathsPost->setValue( _openGLWidget->sizePathsPost( ));
  _spinBoxSizeSynapsesMap->setValue( _openGLWidget->sizeSynapseMap( ));

}

static bool sortAscending ( const std::pair< unsigned int, unsigned int >& lhs,
                     const std::pair< unsigned int, unsigned int >& rhs)
{
  return lhs.first < rhs.first;
}

void MainWindow::updateInfoDock( void )
{
  clearInfoDock( );

  const auto& synapses = _openGLWidget->currentSynapses( );

  std::unordered_map< unsigned int, unsigned int > presentSynapsesPre;
  std::unordered_map< unsigned int, unsigned int > presentSynapsesPost;

  std::vector< std::pair< unsigned int, unsigned int >> vecSynPre;
  std::vector< std::pair< unsigned int, unsigned int >> vecSynPost;

  for( auto syn : synapses )
  {
    const unsigned int gidPre = syn->preSynapticNeuron( );
    const unsigned int gidPost = syn->postSynapticNeuron( );

    auto synapseNumberPre = presentSynapsesPre.find( gidPre );
    if( synapseNumberPre == presentSynapsesPre.end( ))
    {
      synapseNumberPre =
          presentSynapsesPre.insert(
              std::make_pair( gidPre, vecSynPre.size( ))).first;

      vecSynPre.push_back( std::make_pair( gidPre, 0 ));
    }

    vecSynPre[ synapseNumberPre->second ].second++;

    auto synapseNumberPost = presentSynapsesPost.find( gidPost );
    if( synapseNumberPost == presentSynapsesPost.end( ))
    {
      synapseNumberPost =
          presentSynapsesPost.insert(
              std::make_pair( gidPost, vecSynPost.size( ) )).first;

      vecSynPost.push_back( std::make_pair( gidPost, 0 ));
    }

    vecSynPost[ synapseNumberPost->second ].second++;
  }

  std::sort( vecSynPre.begin( ), vecSynPre.end( ), sortAscending );
  std::sort( vecSynPost.begin( ), vecSynPost.end( ), sortAscending );

  for( unsigned int i = 0; i < 2; ++i )
  {
    QWidget *widget = nullptr;
    std::vector< std::pair< unsigned int, unsigned int >>* presentSynapses = nullptr;

    if( i == 0 )
    {
      _widgetInfoPre = new QGroupBox( tr( "Presynaptic neurons:") );
      widget = _widgetInfoPre;
      presentSynapses = &vecSynPre;
    }
    else
    {
      _widgetInfoPost = new QGroupBox( tr( "Postsynaptic neurons:") );
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

    upperLayout->addWidget( new QLabel( "  GID" ), 0, currentColumn++, 1, 1 );
    upperLayout->addWidget( line, 0, currentColumn++, 1, 1 );
    upperLayout->addWidget( new QLabel( "# synapses" ), 0, currentColumn, 1, 1 );
    upperLayout->addWidget( scroll, 1, 0, 1, maxColumns );

    currentColumn = 0;

    if(presentSynapses != nullptr)
    {
      for( const auto &syn : *presentSynapses )
      {
        currentColumn = 0;

        const auto gidLabel = new QLabel(QString::number( syn.first ));

        layoutWidget->addWidget(gidLabel, currentRow, currentColumn++, 1, 1 );

        auto vline = new QFrame( );
        vline->setFrameShape( QFrame::VLine );
        vline->setFrameShadow( QFrame::Sunken );

        layoutWidget->addWidget(vline, currentRow, currentColumn++, 1, 1);

        const auto usedLabel = new QLabel(QString::number( syn.second ));

        layoutWidget->addWidget(usedLabel, currentRow, currentColumn, 1, 1 );

        ++currentRow;
      }
    }

    _layoutInfo->addWidget( widget, widget == _widgetInfoPre ? 0:1);
  }

  update( );
}

void MainWindow::clearInfoDock( void )
{
  std::function<void(QLayout*)> clearLayout = [&clearLayout](QLayout *layout)
  {
    while(layout->count() > 0)
    {
      auto child = layout->takeAt(0);
      if(child->layout() != 0)
      {
        clearLayout(child->layout());
      }
      else if(child->widget() != 0)
      {
        delete child->widget();
      }

      delete child;
    }
  };

  clearLayout(_layoutInfo);
  _widgetInfoPre = nullptr;
  _widgetInfoPost = nullptr;
}

void MainWindow::loadData( const std::string& dataset,
                           const std::string& target )
{
  if(m_thread)
  {
    QMessageBox msgBox{this};
    msgBox.setWindowTitle(tr("Data loading"));
    msgBox.setIcon(QMessageBox::Icon::Information);
    msgBox.setText(tr("A dataset is already loading. Cannot load: %1, target %2")
                         .arg(QString::fromStdString(dataset)
                         .arg(QString::fromStdString(target))));
    msgBox.exec();
  }

  disableInterface(true);
  _ui->statusbar->showMessage(tr("Loading %1").arg(QString::fromStdString(dataset)) );

  m_thread = std::make_shared<LoadingThread>(dataset, target, this);
  auto dialog = new LoadingDialog{this};

  connect(m_thread.get(), SIGNAL(finished()),
          this,           SLOT(onDataLoaded()), Qt::QueuedConnection);
  connect(m_thread.get(), SIGNAL(destroyed(QObject *)),
          dialog,         SLOT(closeDialog()));
  connect(m_thread.get(), SIGNAL(progress(const QString &, const unsigned int)),
          dialog,         SLOT(progress(const QString &, const unsigned int)));

  dialog->show();
  _openGLWidget->doneCurrent();

  m_thread->start();
}

void MainWindow::loadPresynapticList( void )
{
  nsol::DataSet* nsolData = _openGLWidget->dataset( );

  _modelListPre->clear( );

  QList<QStandardItem*> items;
  for( const auto &neuron : nsolData->neurons( ))
  {
    auto item = new QStandardItem( );
    item->setData( neuron.first, Qt::DisplayRole );

    items << item;
  }

  _modelListPre->appendColumn( items );
  _modelListPre->sort( 0, Qt::AscendingOrder );

  update( );
}

void MainWindow::loadPostsynapticList( unsigned int gid )
{
  auto nsolData = _openGLWidget->dataset( );

  _modelListPost->clear( );

  std::set< unsigned int > selection = { gid };
  const auto synapses = nsolData->circuit( ).synapses( selection, nsol::Circuit::PRESYNAPTICCONNECTIONS );

  selection.clear( );

  QList<QStandardItem*> items;
  for(const auto &syn : synapses )
  {
    unsigned int postGid = syn->postSynapticNeuron( );
    if( selection.find( postGid ) != selection.end( ))
      continue;

    auto item = new QStandardItem( );
    item->setData( postGid, Qt::DisplayRole );

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
    QString target = QInputDialog::getText(
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

  const QString csv = QFileDialog::getSaveFileName(
    this , tr( "Save synapses..." ) , tr( "synapses.csv" ) ,
    tr( "Comma Separated Values (*.csv)" ) ,
    nullptr , QFileDialog::DontUseNativeDialog );

  QStringList list{ "Compact" , "Matrix" };

  const QString option =
    csv.isEmpty( ) ?
    "" :
    QInputDialog::getItem( this , tr( "Select CSV type" ) ,
                           "CSV type:" , list , 0 , false );

  QApplication::setOverrideCursor( Qt::WaitCursor );
  if ( !xml.isEmpty( ))
  {
    exportXML( xml );
  }

  if ( !csv.isEmpty( ))
  {
    if ( !option.isEmpty( ))
    {
      if ( option == "Matrix" )
      {
        exportMatrixCSV( csv );
      }
      else
      {
        exportCompactCSV( csv );
      }
    }
  }
  QApplication::restoreOverrideCursor( );
}

void MainWindow::exportMatrixCSV( const QString& filename ) const
{
  const auto* nsolData = _openGLWidget->dataset( );
  const auto& neurons = nsolData->neurons( );

  const auto minmax = std::minmax_element(
    neurons.begin( ) , neurons.end( ) ,
    [ ]( std::pair< const unsigned int , nsol::Neuron* > a ,
         std::pair< const unsigned int , nsol::Neuron* > b )
    {
      return a.first < b.first;
    } );

  const auto min = minmax.first;
  const auto max = minmax.second;

  QFile file( filename );
  file.open( QFile::WriteOnly );

  file.write( "Pre gid \\ Post gid" );

  for ( unsigned int i = min->first; i <= max->first; i++ )
  {
    file.write( tr( ";%1" ).arg( i ).toUtf8( ));
  }

  std::map< unsigned int , unsigned int > synapsesMap;
  for ( const auto& pre: nsolData->neurons( ))
  {
    const auto synapses = nsolData->circuit( ).synapses(
      pre.first , nsol::Circuit::PRESYNAPTICCONNECTIONS
    );

    for ( const auto& syn: synapses )
    {
      const unsigned int post = syn->postSynapticNeuron( );
      synapsesMap[ post ] = synapsesMap[ post ] + 1;
    }

    file.write( tr( "\n%1" ).arg( pre.first ).toUtf8( ));
    for ( unsigned int i = min->first; i <= max->first; i++ )
    {
      file.write( tr( ";%1" ).arg( synapsesMap[ i ] ).toUtf8( ));
    }
    synapsesMap.clear( );
  }
  file.close( );
}

void MainWindow::exportCompactCSV( const QString& filename ) const
{
  const auto* nsolData = _openGLWidget->dataset( );

  QFile file( filename );
  file.open( QFile::WriteOnly );

  const brain::Circuit brainCircuit( *_openGLWidget->dataset( )->blueConfig( ));
  const brion::GIDSet gidSetBrain = brainCircuit.getGIDs(
    _openGLWidget->dataset( )->blueConfigTarget( ));

  const brain::Synapses& brainSynapses =
    brainCircuit.getAfferentSynapses( gidSetBrain ,
                                      brain::SynapsePrefetch::attributes );

  file.write( "SynId;PreNeuronId;PostNeuronId;Delay;Conductance;"
              "Utilization;Depression;Facilitation;Efficacy" );

  for ( const auto& pre: nsolData->neurons( ))
  {
    const auto synapses = nsolData->circuit( ).synapses(
      pre.first , nsol::Circuit::PRESYNAPTICCONNECTIONS
    );
    for ( const auto& syn: synapses )
    {
      auto morphSyn = dynamic_cast<nsol::MorphologySynapse*>(syn);
      if ( morphSyn )
      {
        auto brainSyn = brainSynapses[ morphSyn->gid( ) ];
        file.write( tr( "\n%1;%2;%3;%4;%5;%6;%7;%8;%9" )
                      .arg( morphSyn->gid( ))
                      .arg( morphSyn->preSynapticNeuron( ))
                      .arg( morphSyn->postSynapticNeuron( ))
                      .arg( brainSyn.getDelay( ))
                      .arg( brainSyn.getConductance( ))
                      .arg( brainSyn.getUtilization( ))
                      .arg( brainSyn.getDepression( ))
                      .arg( brainSyn.getFacilitation( ))
                      .arg( brainSyn.getEfficacy( ))
                      .toUtf8( ));
      }
    }
  }
  file.close( );
}

void MainWindow::exportXML( const QString& filename ) const
{
  const auto* nsolData = _openGLWidget->dataset( );

  QFile file( filename );
  file.open( QFile::WriteOnly );

  nsol::XmlSceneWriter::writeToXml(
    filename.toStdString( ) ,
    nsolData->columns( ) ,
    std::map< std::string , nsol::NeuronMorphologyPtr >( ) ,
    false
  );


  file.close( );
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

  if( _openGLWidget->mode( ) == syncopa::PATHS )
  {
    if(selection.size() > 1)
    {
      _openGLWidget->selectPresynapticNeuron( selection );
      _modelListPost->clear();
    }
    else
    {
      const auto gid = selection.front();
      _openGLWidget->selectPresynapticNeuron( gid );
      loadPostsynapticList( gid );
    }
  }
  else
  {
    _openGLWidget->selectPresynapticNeuron( selection );
  }

  _groupBoxDynamic->setEnabled( false );
  dynamicStop( );

  updateInfoDock( );

  if ( m_socket )
  {
    if ( _openGLWidget->mode( ) == syncopa::SYNAPSES || selection.empty( ))
    {
      m_socket->sendMessage( SynapsesModeSelection( selection ).toJSON( ));
    }
    else
    {
      throw std::runtime_error( "PATATA" );
      m_socket->sendMessage( PathsModeSelection(
        selection.at( 0 ) , std::vector< unsigned int >( ))
                               .toJSON( ));
    }
  }
}

void MainWindow::postsynapticNeuronClicked()
{
  std::vector< unsigned int > selection;
  for( auto item : _listPostsynaptic->selectionModel( )->selectedIndexes( ))
  {
    selection.push_back( _modelListPost->itemFromIndex( item )->data(
      Qt::DisplayRole ).value< unsigned int >( ));
  }

  _openGLWidget->selectPostsynapticNeuron( selection );

  _groupBoxDynamic->setEnabled( true );
  dynamicStop( );

//  std::cout << "Selected post: ";
//  std::for_each( selection.cbegin( ) , selection.cend( ) ,
//                 [ ]( unsigned int gid )
//                 { std::cout << gid << " "; } );
//  std::cout << std::endl;

  updateInfoDock( );

  if ( m_socket )
  {
    auto pre = _listPresynaptic->selectionModel( )->selection( )
      .first( ).indexes( ).first( ).data( ).value< unsigned int >( );
    auto message = PathsModeSelection( pre , selection );
    m_socket->sendMessage( message.toJSON( ));
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

  _colorMapWidget->setPlot( _openGLWidget->getSynapseMappingPlot( ),
                            rangeBounds.first,
                            rangeBounds.second );
}

void MainWindow::setSynapseMappingAttribute( int attrib )
{
  _openGLWidget->setSynapseMapping( attrib );

  auto rangeBounds = _openGLWidget->rangeBounds( );

  _colorMapWidget->setPlot( _openGLWidget->getSynapseMappingPlot( ),
                            rangeBounds.first,
                            rangeBounds.second );
}

void MainWindow::clear( void )
{
  _openGLWidget->clearSelection( );

  _listPresynaptic->selectionModel( )->clearSelection( );

  if( _modelListPost )
    _modelListPost->clear( );

  updateInfoDock( );
}

bool MainWindow::showDialog( QColor& current, const QString& message )
{
  const QColor result = QColorDialog::getColor( current, this,
                                                QString( message ),
                                                QColorDialog::DontUseNativeDialog);

  const auto valid = result.isValid();
  if( valid )
  {
    current = result;
  }

  return valid;
}

void MainWindow::colorSelectionClicked( void )
{
  auto frame = qobject_cast< QPushButton* >( sender( ));
  if( frame )
  {
    QColor color;
    QString message;

    if( frame == _frameColorMorphoPre )
      color = colorEigenToQt( _openGLWidget->colorSelectedPre( ));
    else if( frame == _frameColorMorphoPost )
      color = colorEigenToQt( _openGLWidget->colorSelectedPost( ));
    else if( frame == _frameColorMorphoContext )
      color = colorEigenToQt( _openGLWidget->colorRelated( ));
    else if( frame == _frameColorMorphoOther )
      color = colorEigenToQt( _openGLWidget->colorContext( ));
    else if( frame == _frameColorSynapsesPre )
      color = colorEigenToQt( _openGLWidget->colorSynapsesPre( ));
    else if( frame == _frameColorSynapsesPost )
      color = colorEigenToQt( _openGLWidget->colorSynapsesPost( ));
    else if( frame == _frameColorPathsPre )
      color = colorEigenToQt( _openGLWidget->colorPathsPre( ));
    else if( frame == _frameColorPathsPost )
      color = colorEigenToQt( _openGLWidget->colorPathsPost( ));
    else
    {
      std::cout << "Warning: Frame " << frame << " clicked not connected." << std::endl;
      return;
    }

    if( showDialog( color, message ))
    {
      if( frame == _frameColorMorphoPre )
        _openGLWidget->colorSelectedPre( colorQtToEigen( color ));
      else if( frame == _frameColorMorphoPost )
        _openGLWidget->colorSelectedPost( colorQtToEigen( color ));
      else if( frame == _frameColorMorphoContext )
        _openGLWidget->colorRelated( colorQtToEigen( color ));
      else if( frame == _frameColorMorphoOther )
        _openGLWidget->colorContext( colorQtToEigen( color ));
      else if( frame == _frameColorSynapsesPre )
        _openGLWidget->colorSynapsesPre( colorQtToEigen( color ));
      else if( frame == _frameColorSynapsesPost )
        _openGLWidget->colorSynapsesPost( colorQtToEigen( color ));
      else if( frame == _frameColorPathsPre )
        _openGLWidget->colorPathsPre ( colorQtToEigen( color ));
      else if( frame == _frameColorPathsPost )
        _openGLWidget->colorPathsPost( colorQtToEigen( color ));
    }

    frame->setStyleSheet( "background-color: " + color.name( ));
  }
}

void MainWindow::transparencySliderMoved( int position )
{
  auto source = qobject_cast< QSlider* >( sender( ));

  const float normValue = ( position - _sliderMin) * _invRangeSliders;

  if( source )
  {
    if( source == _sliderAlphaSynapsesPre )
      _openGLWidget->alphaSynapsesPre( normValue );
    else if( source == _sliderAlphaSynapsesPost )
      _openGLWidget->alphaSynapsesPost( normValue );
    else if( source == _sliderAlphaPathsPre )
      _openGLWidget->alphaPathsPre( normValue );
    else if( source == _sliderAlphaPathsPost )
      _openGLWidget->alphaPathsPost( normValue );
    else if( source == _sliderAlphaSynapsesMap )
      _openGLWidget->alphaSynapseMap( _colorMapWidget->getColors( ), normValue );
  }
}

void MainWindow::sizeSpinBoxChanged( double value )
{
  auto source = qobject_cast< QDoubleSpinBox* >( sender( ));

  if( source )
  {
    if( source == _spinBoxSizeSynapsesPre )
      _openGLWidget->sizeSynapsesPre( value );
    else if( source == _spinBoxSizeSynapsesPost )
      _openGLWidget->sizeSynapsesPost( value );
    else if( source == _spinBoxSizePathsPre )
      _openGLWidget->sizePathsPre( value );
    else if( source == _spinBoxSizePathsPost )
      _openGLWidget->sizePathsPost( value );
    else if( source == _spinBoxSizeSynapsesMap )
      _openGLWidget->sizeSynapseMap( value );
  }
}

void MainWindow::showFullMorphologyChecked( bool value )
{
  auto source = qobject_cast< QPushButton* >( sender( ));

  if( source )
  {
    if( source == _buttonShowFullMorphoPre )
      _openGLWidget->showFullMorphologiesPre( value );
    else if( source == _buttonShowFullMorphoPost )
      _openGLWidget->showFullMorphologiesPost( value );
    else if( source == _buttonShowFullMorphoContext )
      _openGLWidget->showFullMorphologiesContext( value );
    else if( source == _buttonShowFullMorphoOther )
      _openGLWidget->showFullMorphologiesOther( value );
  }
}

void MainWindow::colorSynapseMapAccepted( void )
{
  _openGLWidget->colorSynapseMap( _colorMapWidget->getColors( ));
}

void MainWindow::colorSynapseMapCancelled( void )
{
}

void MainWindow::dynamicStart( void )
{
  if( !_openGLWidget->dynamicActive( ))
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
  _openGLWidget->toggleDynamicMovement( );
}

void MainWindow::dynamicStop( void )
{
  _buttonDynamicStop->setEnabled( false  );
  _buttonDynamicStart->setText( "Start" );

  _openGLWidget->stopDynamic( );
}

void MainWindow::filteringStateChanged( void )
{
  _openGLWidget->filteringState( _colorMapWidget->filter( ));
}

void MainWindow::filteringBoundsChanged( void )
{
  auto bounds = _colorMapWidget->filterBounds( );
  _openGLWidget->filteringBounds( bounds.first, bounds.second );
}

void MainWindow::modeChanged( bool selectedModeSynapses )
{
  const TMode mode = selectedModeSynapses ? syncopa::SYNAPSES : syncopa::PATHS;
  if(_listPresynaptic->selectionModel())
  {
    const auto selectedPre = _listPresynaptic->selectionModel()->selectedIndexes();

    if( selectedPre.size( ) != 1 )
    {
      _listPresynaptic->clearSelection( );
    }
    else if ( !selectedModeSynapses )
    {
      const auto item = _modelListPre->itemFromIndex(selectedPre.front( ));
      const unsigned int idx = item->data( Qt::DisplayRole ).value< unsigned int >( );

      loadPostsynapticList( idx );
    }
    else
    {
      _listPostsynaptic->clearSelection( );
    }
  }

  if( mode == syncopa::SYNAPSES )
  {
    _checkShowMorphoPre->setChecked( false );
    _checkShowMorphoPost->setChecked( false );
    _checkShowMorphoContext->setChecked( false );
    _checkShowMorphoOther->setChecked( false );

    _checkShowMorphoPre->setEnabled( true );
    _checkShowMorphoPost->setEnabled( false );
    _checkShowMorphoContext->setEnabled( false );
    _checkShowMorphoOther->setEnabled( false );

    _frameColorMorphoPre->setEnabled( true );
    _frameColorMorphoPost->setEnabled( false );
    _frameColorMorphoContext->setEnabled( false );
    _frameColorMorphoOther->setEnabled( false );

    _buttonShowFullMorphoPre->setEnabled( true );
    _buttonShowFullMorphoPost->setEnabled( false );
    _buttonShowFullMorphoContext->setEnabled( false );
    _buttonShowFullMorphoOther->setEnabled( false );
  }
  else
  {
    _checkShowMorphoPre->setEnabled( true );
    _checkShowMorphoPost->setEnabled( true );
    _checkShowMorphoContext->setEnabled( true );
    _checkShowMorphoOther->setEnabled( true );

    _checkShowMorphoPre->setChecked( true );
    _checkShowMorphoPost->setChecked( true );
    _checkShowMorphoContext->setChecked( true );
    _checkShowMorphoOther->setChecked( true );

    _frameColorMorphoPre->setEnabled( true );
    _frameColorMorphoPost->setEnabled( true );
    _frameColorMorphoContext->setEnabled( true );
    _frameColorMorphoOther->setEnabled( true );
    _frameColorMorphoOther->setEnabled( true );

    _buttonShowFullMorphoPre->setEnabled( true );
    _buttonShowFullMorphoPost->setEnabled( true );
    _buttonShowFullMorphoContext->setEnabled( true );
    _buttonShowFullMorphoOther->setEnabled( true );

    _buttonShowFullMorphoContext->setChecked( false );
    _buttonShowFullMorphoOther->setChecked( false );
  }

  _groupBoxDynamic->setEnabled( false );

  if( selectedModeSynapses && _modelListPost )
    _modelListPost->clear( );

  _listPostsynaptic->setEnabled( !selectedModeSynapses );

  _groupBoxPaths->setEnabled( !selectedModeSynapses );

  _listPresynaptic->setSelectionMode( selectedModeSynapses ?
                                      QAbstractItemView::ExtendedSelection :
                                      QAbstractItemView::SingleSelection );

  _openGLWidget->mode( mode );
  updateInfoDock();
}

void MainWindow::alphaModeChanged( bool state )
{
  _openGLWidget->alphaMode( !state );
}

void MainWindow::aboutDialog()
{
  QString message =
        QString( "<h2>Syncopa" ) + "<br>" +
        tr( "Version " ) + syncopa::Version::getString( ).c_str( ) +
        tr( " rev (%1)<br>" ).arg( syncopa::Version::getRevision( ) ) + "</h2><br>"
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
        "<br>(C) 2015-" + QString::number(QDateTime::currentDateTime().date().year()) + "<br><br>"
        "<a href='https://vg-lab.es'><img src=':/icons/logoVGLab.png'/></a>"
        "&nbsp;&nbsp;&nbsp;&nbsp;"
        "<a href='https://www.urjc.es'><img src=':/icons/logoURJC.png' /></a>"
        "&nbsp;&nbsp;&nbsp;&nbsp;"
        "<a href='https://www.upm.es'><img src=':/icons/logoUPM.png' /></a>";

  QMessageBox::about( this, tr( "About Syncopa" ), message );
}

void MainWindow::onConnectionButtonTriggered(bool value)
{
  auto action = qobject_cast<QAction *>(sender());
  if(action)
  {
    action->setEnabled(false);
  }
  else
  {
    return;
  }

  if(value)
  {
    if(m_socket) return;

    bool ok = false;
    const auto title = tr("Network connection");
    const auto label = tr("hostname:port");
    auto addr = QInputDialog::getText(this, title, label, QLineEdit::Normal, "", &ok);
    if(!ok || addr.isEmpty()) return;

    if(!addr.startsWith("ws://", Qt::CaseInsensitive))
      addr = QString("ws://%1").arg(addr);

    const auto url = QUrl(addr);
    if(!url.isValid())
    {
      QMessageBox::critical(this, title, tr("Invalid url: %1").arg(addr), QMessageBox::Ok);
      return;
    }

    action->setIcon(QIcon(":/icons/disconnect.svg"));

    m_socket = new WebSocketThread( QUrl( addr ) , this );

    connect(m_socket, SIGNAL(connected()),       this, SLOT(onConnectionStateChanged()));
    connect(m_socket, SIGNAL(disconnected()),    this, SLOT(onConnectionStateChanged()));
    connect(m_socket, SIGNAL(messageReceived()), this, SLOT(onMessageReceived()));
    connect(m_socket, SIGNAL(error()),           this, SLOT(onConnectionError()));
    connect(m_socket, SIGNAL(finished()),        this, SLOT(onConnectionThreadTerminated()));
    
    connect(this, &QObject::destroyed, m_socket, [this]() {
        m_socket->stop( );
        m_socket->wait( );
    });

    m_socket->start();
  }
  else
  {
    if(!m_socket) return;
    m_socket->stop();
  }
}

void MainWindow::onMessageReceived()
{
  if(m_socket)
  {
    auto message = m_socket->getMessage( );
    switch ( message.id )
    {
      // SELECT
      case 0:
      {
        auto selectionData = std::dynamic_pointer_cast< SynapsesModeSelection >(
          message.data );
        parseSelectionJSON( selectionData );
      }
        break;
      case 1:
      {
        auto selectionData = std::dynamic_pointer_cast< PathsModeSelection >(
          message.data );
        parseSelectionJSON( selectionData );
      }
        break;
    }
  }
}

void MainWindow::onConnectionThreadTerminated()
{
  m_socket->deleteLater();
  m_socket = nullptr;
  _ui->actionNetworkConnection->blockSignals(true);
  _ui->actionNetworkConnection->setChecked(false);
  _ui->actionNetworkConnection->setEnabled(true);
  _ui->actionNetworkConnection->setIcon(QIcon(":/icons/connect.svg"));
  _ui->actionNetworkConnection->blockSignals(false);
}

void MainWindow::disableInterface(bool value)
{
  static bool current = false;

  if(value == current) return;

  current = value;

  _dockColor->setEnabled(!value);
  _dockInfo->setEnabled(!value);
  _dockList->setEnabled(!value);
  _ui->toolBar->setEnabled(!value);
  _ui->menubar->setEnabled(!value);

  if(value)
    QApplication::setOverrideCursor(Qt::WaitCursor);
  else
    QApplication::restoreOverrideCursor();
}

void LoadingThread::run()
{
  try
  {
    if(m_parent)
    {
      emit progress(tr("Loading data"), 0);

      connect(m_parent->_openGLWidget, SIGNAL(progress(const QString &, const unsigned int)),
              this,                    SIGNAL(progress(const QString &, const unsigned int)));

      m_parent->_openGLWidget->loadBlueConfig(m_blueconfig,m_target);
    }
  }
  catch(const std::exception &e)
  {
    m_errors = std::string(e.what());
    m_errors += std::string("\n") + __FILE__ + " " + std::to_string(__LINE__);
  }
  catch(...)
  {
    m_errors = std::string("Unspecified exception.");
    m_errors += std::string("\n") + __FILE__ + " " + std::to_string(__LINE__);
  }

  emit progress(tr("Finished"), 100);
}

LoadingDialog::LoadingDialog(QWidget *p)
: QDialog(p, Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint)
{
  setWindowIcon(QIcon{":/neurotessmessh.png"});

  auto layout = new QVBoxLayout();
  m_progress = new QProgressBar(this);
  m_progress->setMinimumWidth(590);
  layout->addWidget(m_progress, 1, Qt::AlignHCenter|Qt::AlignVCenter);
  layout->setMargin(4);
  setLayout(layout);

  setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
  setFixedSize(600, sizeHint().height());
}

void LoadingDialog::progress(const QString &message, const unsigned int value)
{
  // NOTE: adjusted to the number of loading steps.
  constexpr float STEP_WEIGHT = 33.33;

  static float steps = 0;
  static unsigned int lastValue = 0;

  if(lastValue > value) ++steps;
  lastValue = value;

  const int progressValue = std::nearbyint((steps + static_cast<float>(value)/100) * STEP_WEIGHT);

  m_progress->setValue(progressValue);
  if(!message.isEmpty())
    m_progress->setFormat(tr("%1 - %p%").arg(message));
}

void LoadingDialog::closeDialog()
{
  close();
  deleteLater();
}

void MainWindow::onConnectionStateChanged()
{
  _ui->actionNetworkConnection->setEnabled(true);
}

void MainWindow::onConnectionError()
{
  if ( m_socket )
  {
    const auto errorMessage = m_socket->errorMessage( );
    const auto title = tr( "Network error" );
    QMessageBox::critical( this , title , errorMessage , QMessageBox::Ok );

    m_socket->stop( );
  }
}

void MainWindow::parseSelectionJSON(
  std::shared_ptr< SynapsesModeSelection > selection )
{
  if ( _openGLWidget->dataset( ) == nullptr ) return;
  if ( _openGLWidget->mode( ) != syncopa::SYNAPSES )
  {
    _radioModeSynapses->setChecked( true );
  }

  std::vector< unsigned int > finalSelection;
  const int rows = _modelListPre->rowCount( );
  QItemSelection selectionSet;

  auto start = selection->selection.cbegin( );
  auto end = selection->selection.cend( );

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

  _openGLWidget->selectPresynapticNeuron( finalSelection );
  _groupBoxDynamic->setEnabled( false );
  dynamicStop( );
  updateInfoDock( );
}

void MainWindow::parseSelectionJSON(
  std::shared_ptr< PathsModeSelection > selection )
{
  if ( _openGLWidget->dataset( ) == nullptr ) return;
  if ( _openGLWidget->mode( ) != syncopa::PATHS )
  {
    _radioModePaths->setChecked( true );
  }

  auto preSelection = static_cast<unsigned int>(selection->preSelection);
  std::vector< unsigned int > finalSelection;
  const int rows = _modelListPre->rowCount( );
  QItemSelection pre;
  auto start = selection->postSelection.cbegin( );
  auto end = selection->postSelection.cend( );

  for ( int i = 0; i < rows; i++ )
  {
    const QModelIndex index = _modelListPre->index( i , 0 );
    const auto value = index.data( ).value< unsigned int >( );
    if ( value == preSelection )
    {
      pre.append( QItemSelectionRange( index ));
    }
  }

  _listPresynaptic->selectionModel()->blockSignals( true );
  _listPostsynaptic->selectionModel()->blockSignals( true );
  _listPresynaptic->selectionModel( )->select(
    pre ,
    QItemSelectionModel::ClearAndSelect
  );

  _openGLWidget->selectPresynapticNeuron( preSelection );
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

  _listPresynaptic->selectionModel()->blockSignals( false );
  _listPostsynaptic->selectionModel()->blockSignals( false );

  _openGLWidget->selectPostsynapticNeuron( finalSelection );
  _groupBoxDynamic->setEnabled( false );

  dynamicStop( );
  updateInfoDock( );
}
