/*
 * @file  MainWindow.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "MainWindow.h"
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QGridLayout>
#include <QScrollArea>
#include <QColorDialog>

// #include "qt/CustomSlider.h"

#ifdef VISIMPL_USE_GMRVLEX
  #include <gmrvlex/gmrvlex.h>
#endif

#include <thread>

static float invRGB = 1.0 / 255;

syncopa::vec3 colorQtToEigen( const QColor& color_ )
{
  return syncopa::vec3( std::min( 1.0f, std::max( 0.0f, color_.red( ) * invRGB )),
                       std::min( 1.0f, std::max( 0.0f, color_.green( ) * invRGB )),
                       std::min( 1.0f, std::max( 0.0f, color_.blue( ) * invRGB )));
}

QColor colorEigenToQt( const syncopa::vec3& color_ )
{
  return QColor( std::min( 255, std::max( 0, int( color_.x( ) * 255 ))),
                 std::min( 255, std::max( 0, int( color_.y( ) * 255 ))),
                 std::min( 255, std::max( 0, int( color_.z( ) * 255 ))));
}

MainWindow::MainWindow( QWidget* parent_,
                        bool updateOnIdle )
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
//, _frameColorSynapseMapGradient( nullptr )
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

, _buttonDynamic( nullptr )
, _comboSynapseMapAttrib( nullptr )
, _groupBoxMorphologies( nullptr )
, _groupBoxSynapses( nullptr )
, _groupBoxPaths( nullptr )
{
  _ui->setupUi( this );

  _ui->actionUpdateOnIdle->setChecked( updateOnIdle );
  _ui->actionShowFPSOnIdleUpdate->setChecked( false );


  connect( _ui->actionQuit, SIGNAL( triggered( )),
           QApplication::instance(), SLOT( quit( )));

  _invRangeSliders = 1.0 / ( _sliderMax - _sliderMin );

}


MainWindow::~MainWindow( void )
{
    delete _ui;
}


void MainWindow::init( void )
{

  _openGLWidget = new OpenGLWidget( 0, 0 );
  this->setCentralWidget( _openGLWidget );
  qDebug( ) << _openGLWidget->format( );

  if( _openGLWidget->format( ).version( ).first < 4 )
  {
    std::cerr << "This application requires at least OpenGL 4.0" << std::endl;
    exit( -1 );
  }

  _openGLWidget->createParticleSystem( );

  initListDock( );
  initColorDock( );
  initInfoDock( );

  _openGLWidget->idleUpdate( _ui->actionUpdateOnIdle->isChecked( ));

  connect( _ui->actionUpdateOnIdle, SIGNAL( triggered( )),
           _openGLWidget, SLOT( toggleUpdateOnIdle( )));

  _ui->actionUpdateOnIdle->setChecked( false );

  connect( _ui->actionBackgroundColor, SIGNAL( triggered( )),
           _openGLWidget, SLOT( changeClearColor( )));

  connect( _ui->actionShowFPSOnIdleUpdate, SIGNAL( triggered( )),
           _openGLWidget, SLOT( toggleShowFPS( )));

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

  QDockWidget* dockList = new QDockWidget( tr( "Selection" ));
  dockList->setMaximumHeight( 250 );

  QGridLayout* dockLayout = new QGridLayout( );
  dockLayout->setAlignment( Qt::AlignTop );
//  dockList->setLayout( dockLayout );
  QWidget* container = new QWidget( );
  container->setLayout( dockLayout );

  QPushButton* clearButton = new QPushButton( "Clear selection" );
  connect( clearButton, SIGNAL( clicked( void )),
           this, SLOT( clear( void )));

  unsigned int row = 0;
  unsigned int col = 0;

  dockLayout->addWidget( groupSelection, row, col, 1, 4 );
  ++row;
  col = 0;

  dockLayout->addWidget( new QLabel( "Presynaptic:" ), row, col++, 1, 2 );
  ++col;

  dockLayout->addWidget( new QLabel( "Postsynaptic:" ), row, col, 1, 2 );

  ++row;
  col = 0;

  dockLayout->addWidget( _listPresynaptic, row, col++, 1, 2 );
  ++col;
  dockLayout->addWidget( _listPostsynaptic, row, col, 1, 2 );

  dockLayout->addWidget( clearButton );

  dockList->setWidget( container );


  connect( _radioModeSynapses, SIGNAL( toggled( bool )),
           this, SLOT( modeChanged( bool )));

  connect( _listPresynaptic, SIGNAL( clicked( const QModelIndex& )),
           this, SLOT( presynapticNeuronClicked( const QModelIndex& )));

  connect( _listPostsynaptic, SIGNAL( clicked( const QModelIndex& )),
           this, SLOT( postsynapticNeuronClicked( const QModelIndex& )));

  addDockWidget( Qt::RightDockWidgetArea, dockList );

}

void MainWindow::initColorDock( void )
{
  _dockColor = new QDockWidget( "Aspect" );
  _dockColor->setMinimumHeight( 300 );

  QScrollArea* scrollArea = new QScrollArea( );

  QWidget* container = new QWidget( );

  QVBoxLayout* layout = new QVBoxLayout( );

  container->setLayout( layout );

  _groupBoxMorphologies = new QGroupBox( "Morphologies" );
  _groupBoxSynapses = new QGroupBox( "Synapses" );
  _groupBoxPaths = new QGroupBox( "Paths" );

  _checkShowMorphoPre = new QCheckBox( "Presynaptic" );
  _checkShowMorphoPost = new QCheckBox( "Postsynaptic" );
  _checkShowMorphoContext = new QCheckBox( "Context" );
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

  QGridLayout* morphoLayout = new QGridLayout( );
  _groupBoxMorphologies->setLayout( morphoLayout );

  QGridLayout* synLayout = new QGridLayout( );
  _groupBoxSynapses->setLayout( synLayout );

  QGridLayout* pathLayout = new QGridLayout( );
  _groupBoxPaths->setLayout( pathLayout );

  QPalette palette;
  QColor color;

  _frameColorMorphoPre = new QPushButton( );
  _frameColorMorphoPre->setMinimumSize( 20, 20 );
  _frameColorMorphoPre->setMaximumSize( 20, 20 );

  color = colorEigenToQt( _openGLWidget->colorSelectedPre( ));
  _frameColorMorphoPre->setStyleSheet( "background-color: " + color.name( ));

  _frameColorMorphoPost = new QPushButton( );
  _frameColorMorphoPost->setMinimumSize( 20, 20 );
  _frameColorMorphoPost->setMaximumSize( 20, 20 );

  color = colorEigenToQt( _openGLWidget->colorSelectedPost( ));
  _frameColorMorphoPost->setStyleSheet( "background-color: " + color.name( ));

  _frameColorMorphoContext = new QPushButton( );
  _frameColorMorphoContext->setMinimumSize( 20, 20 );
  _frameColorMorphoContext->setMaximumSize( 20, 20 );

  color = colorEigenToQt(_openGLWidget->colorRelated( ));
  _frameColorMorphoContext->setStyleSheet( "background-color: " + color.name( ));

  _frameColorMorphoOther = new QPushButton( );
  _frameColorMorphoOther->setMinimumSize( 20, 20 );
  _frameColorMorphoOther->setMaximumSize( 20, 20 );

  color = colorEigenToQt( _openGLWidget->colorContext( ));
  _frameColorMorphoOther->setStyleSheet( "background-color: " + color.name( ));

  _frameColorSynapsesPre = new QPushButton( );
  _frameColorSynapsesPre->setMinimumSize( 20, 20 );
  _frameColorSynapsesPre->setMaximumSize( 20, 20 );

  color = colorEigenToQt( _openGLWidget->colorSynapsesPre( ));
  _frameColorSynapsesPre->setStyleSheet( "background-color: " + color.name( ));

  _frameColorSynapsesPost = new QPushButton( );
  _frameColorSynapsesPost->setMinimumSize( 20, 20 );
  _frameColorSynapsesPost->setMaximumSize( 20, 20 );

  color = colorEigenToQt( _openGLWidget->colorSynapsesPost( ));
  _frameColorSynapsesPost->setStyleSheet( "background-color: " + color.name( ));

  _frameColorPathsPre = new QPushButton( );
  _frameColorPathsPre->setMinimumSize( 20, 20 );
  _frameColorPathsPre->setMaximumSize( 20, 20 );

  color = colorEigenToQt( _openGLWidget->colorPathsPre( ));
  _frameColorPathsPre->setStyleSheet( "background-color: " + color.name( ));

  _frameColorPathsPost = new QPushButton( );
  _frameColorPathsPost->setMinimumSize( 20, 20 );
  _frameColorPathsPost->setMaximumSize( 20, 20 );

  color = colorEigenToQt( _openGLWidget->colorPathsPost( ));
  _frameColorPathsPost->setStyleSheet( "background-color: " + color.name( ));

  QIcon neuronIcon;//( ":/icons/neurotessmesh.png" );
  neuronIcon.addFile(QStringLiteral(":/icons/neurotessmesh.png"), QSize(), QIcon::Normal, QIcon::On);
  neuronIcon.addFile(QStringLiteral(":/icons/neurolots.png"), QSize(), QIcon::Normal, QIcon::Off);

  _buttonShowFullMorphoPre = new QPushButton( neuronIcon, "" );
  _buttonShowFullMorphoPre->setMaximumSize( 20, 20 );
  _buttonShowFullMorphoPre->setCheckable( true );

  _buttonShowFullMorphoPost = new QPushButton( neuronIcon, "" );
  _buttonShowFullMorphoPost->setMaximumSize( 20, 20 );
  _buttonShowFullMorphoPost->setCheckable( true );

  _buttonShowFullMorphoContext = new QPushButton( neuronIcon, "" );
  _buttonShowFullMorphoContext->setMaximumSize( 20, 20 );
  _buttonShowFullMorphoContext->setCheckable( true );

  _buttonShowFullMorphoOther = new QPushButton( neuronIcon, "" );
  _buttonShowFullMorphoOther->setMaximumSize( 20, 20 );
  _buttonShowFullMorphoOther->setCheckable( true );


//  _frameColorSynapseMapGradient = new GradientWidget( );
//  _frameColorSynapseMapGradient->setDirection( GradientWidget::HORIZONTAL );
//  _frameColorSynapseMapGradient->setMinimumSize( 200, 50 );
//  _frameColorSynapseMapGradient->setMaximumSize( 200, 50 );
//
//  color = colorEigenToQt( _openGLWidget->colorSynapseMapPre( ));
//  _frameColorSynapseMapGradient->setStyleSheet( "background-color: " + color.name( ));

  // Morphologies

  unsigned int row = 0;
  unsigned int col = 0;

  morphoLayout->addWidget( _frameColorMorphoPre, row, col++, 1, 1 );
  morphoLayout->addWidget( _buttonShowFullMorphoPre, row, col++, 1, 1 );
  morphoLayout->addWidget( _checkShowMorphoPre, row, col++, 1, 2 );
//  morphoLayout->addWidget( new QLabel( "Presynaptic" ), row, col++, 1, 2 );
  ++col;

  morphoLayout->addWidget( _frameColorMorphoPost, row, col++, 1, 1 );
  morphoLayout->addWidget( _buttonShowFullMorphoPost, row, col++, 1, 1 );
  morphoLayout->addWidget( _checkShowMorphoPost, row, col++, 1, 2 );
//  morphoLayout->addWidget( new QLabel( "Postsynaptic" ), row, col, 1, 2 );

  ++row;
  col = 0;

  morphoLayout->addWidget( _frameColorMorphoContext, row, col++, 1, 1 );
  morphoLayout->addWidget( _buttonShowFullMorphoContext, row, col++, 1, 1 );
  morphoLayout->addWidget( _checkShowMorphoContext, row, col++, 1, 2 );
//  morphoLayout->addWidget( new QLabel( "Related" ), row, col++, 1, 2 );
  ++col;

  morphoLayout->addWidget( _frameColorMorphoOther, row, col++, 1, 1 );
  morphoLayout->addWidget( _buttonShowFullMorphoOther, row, col++, 1, 1 );
  morphoLayout->addWidget( _checkShowMorphoOther, row, col++, 1, 2 );
//  morphoLayout->addWidget( new QLabel( "Other" ), row, col, 1, 2 );
//TODO
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
  _sliderAlphaSynapsesPre->setMinimum( _sliderMin );
  _sliderAlphaSynapsesPre->setMaximum( _sliderMax );

  _sliderAlphaSynapsesPost = new QSlider( Qt::Horizontal );
  _sliderAlphaSynapsesPost->setMinimum( _sliderMin );
  _sliderAlphaSynapsesPost->setMaximum( _sliderMax );

  _sliderAlphaPathsPre = new QSlider( Qt::Horizontal );
  _sliderAlphaPathsPre->setMinimum( _sliderMin );
  _sliderAlphaPathsPre->setMaximum( _sliderMax );

  _sliderAlphaPathsPost = new QSlider( Qt::Horizontal );
  _sliderAlphaPathsPost->setMinimum( _sliderMin );
  _sliderAlphaPathsPost->setMaximum( _sliderMax );

  _sliderAlphaSynapsesMap = new QSlider( Qt::Horizontal );
  _sliderAlphaSynapsesMap->setMinimum( _sliderMin );
  _sliderAlphaSynapsesMap->setMaximum( _sliderMax );


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

  QCheckBox* checkMapSynapses = new QCheckBox( tr( "Map attribute to color" ));
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

  QFrame* line = new QFrame( );
  line->setFrameShape( QFrame::HLine );
  line->setFrameShadow( QFrame::Sunken );

  row = 0;
  col = 0;


//  synLayout->addWidget( line, row, col );

//  ++row;
  col = 0;

  synLayout->addWidget( _frameColorSynapsesPre, row, col++, 1, 1 );
  synLayout->addWidget( _checkSynapsesPre, row, col++, 1, 2 );
//  synLayout->addWidget( new QLabel( "Presynaptic" ), row, col++, 1, 2 );
  ++col;
  synLayout->addWidget( _spinBoxSizeSynapsesPre, row, col++, 1, 1 );
  synLayout->addWidget( _sliderAlphaSynapsesPre, row, col, 1, 2 );

  ++row;
  col = 0;

  synLayout->addWidget( _frameColorSynapsesPost, row, col++, 1, 1 );
  synLayout->addWidget( _checkSynapsesPost, row, col++, 1, 2 );
//  synLayout->addWidget( new QLabel( "Postsynaptic" ), row, col++, 1, 2 );
  ++col;
  synLayout->addWidget( _spinBoxSizeSynapsesPost, row, col++, 1, 1 );
  synLayout->addWidget( _sliderAlphaSynapsesPost, row, col, 1, 2 );

  ++row;
  col = 0;

  synLayout->addWidget( line, row, col, 1, 6 );
//
  ++row;
  synLayout->addWidget( checkMapSynapses, row, col++, 1, 4 );
  ++col;
//  ++row;
//  col = 0;
//  synLayout->addWidget( new QLabel( tr( "Synapse attribute" )), row, col++, 1, 2 );
  ++col;
  ++col;
  synLayout->addWidget( _comboSynapseMapAttrib, row, col, 1, 2 );
//
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
//  pathLayout->addWidget( new QLabel( "Presynaptic" ), row, col++, 1, 2 );
  ++col;
  pathLayout->addWidget( _spinBoxSizePathsPre, row, col++, 1, 1 );
  pathLayout->addWidget( _sliderAlphaPathsPre, row, col, 1, 2 );

  ++row;
  col = 0;

  pathLayout->addWidget( _frameColorPathsPost, row, col++, 1, 1 );
  pathLayout->addWidget( _checkPathsPost, row, col++, 1, 2 );
//  pathLayout->addWidget( new QLabel( "Postsynaptic" ), row, col++, 1, 2 );
  ++col;
  pathLayout->addWidget( _spinBoxSizePathsPost, row, col++, 1, 1 );
  pathLayout->addWidget( _sliderAlphaPathsPost, row, col, 1, 2 );

  layout->addWidget( _groupBoxMorphologies );
  layout->addWidget( _groupBoxSynapses );
  layout->addWidget( _groupBoxPaths );

  _buttonDynamic = new QPushButton( "Dynamic" );
  _buttonDynamic->setCheckable( true );

  layout->addWidget( _buttonDynamic );

  connect( _buttonDynamic, SIGNAL( clicked( )), this, SLOT( dynamic( )));


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

//  connect( _frameColorSynapseMapGradient, SIGNAL( clicked( )),
//           this, SLOT( colorSynapseMapClicked()));

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

  checkMapSynapses->setChecked( false );

  _buttonShowFullMorphoPre->setChecked( true );
  _buttonShowFullMorphoPost->setChecked( true );
  _buttonShowFullMorphoContext->setChecked( false );
  _buttonShowFullMorphoOther->setChecked( false );

  scrollArea->setWidget( container );
  scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

  _dockColor->setWidget( scrollArea );
  addDockWidget( Qt::RightDockWidgetArea, _dockColor );
}

void MainWindow::initInfoDock( void )
{
  _dockInfo = new QDockWidget( tr( "Information" ));
  _dockInfo->setMinimumHeight( 100 );

  QWidget* container = new QWidget( );

  _layoutInfo = new QVBoxLayout( );
  container->setLayout( _layoutInfo );

//  QStringList header = { "Neuron GID", "Total synapses", "Related synapses" };



//  _layoutInfo->addWidget(  );

  _dockInfo->setWidget( container );

  addDockWidget( Qt::RightDockWidgetArea, _dockInfo );

}

void MainWindow::_loadDefaultValues( void )
{
  int range = _sliderMax - _sliderMin;

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
//static bool sortDescending ( const std::pair< unsigned int, unsigned int >& lhs,
//                      const std::pair< unsigned int, unsigned int >& rhs )
//{
//  return lhs.first > rhs.first;
//}

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
    unsigned int gidPre = syn->preSynapticNeuron( );
    unsigned int gidPost = syn->postSynapticNeuron( );

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

  unsigned int endLoop = ( _openGLWidget->mode( ) == syncopa::PATHS ) ? 2 : 1;

  for( unsigned int i = 0; i < endLoop; ++i )
  {

    QWidget* widget = nullptr;
    std::vector< std::pair< unsigned int, unsigned int >>* presentSynapses;

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

    QGridLayout* upperLayout = new QGridLayout( );
    widget->setLayout( upperLayout );

    QWidget* container = new QWidget( );
    QGridLayout* layoutWidget = new QGridLayout( );
    container->setLayout( layoutWidget );

    QScrollArea* scroll = new QScrollArea( );
    scroll->setWidgetResizable( true );
    scroll->setWidget( container );


    unsigned int currentRow = 0;
    unsigned int currentColumn = 0;

    unsigned int maxColumns = 3;

    QFrame* line = new QFrame( );
    line->setFrameShape( QFrame::VLine );
    line->setFrameShadow( QFrame::Sunken );

    upperLayout->addWidget( new QLabel( "  GID" ), 0, currentColumn++, 1, 1 );

    upperLayout->addWidget( line, 0, currentColumn++, 1, 1 );

    upperLayout->addWidget( new QLabel( "# synapses" ), 0, currentColumn, 1, 1 );

    upperLayout->addWidget( scroll, 1, 0, 1, maxColumns );

    currentColumn = 0;

//    layoutWidget->addWidget( line, currentRow++, currentColumn, 1, maxColumns );
  //  layoutPre->addWidget( new QLabel( "Total Synapses" ), currentRow, currentColumn++, 1, 1 );

    for( auto syn : *presentSynapses )
    {
      currentColumn = 0;

      auto gidString = QString::number( syn.first );

      layoutWidget->addWidget(
          new QLabel( gidString ), currentRow, currentColumn++, 1, 1 );

      QFrame* vline = new QFrame( );
      vline->setFrameShape( QFrame::VLine );
      vline->setFrameShadow( QFrame::Sunken );

      layoutWidget->addWidget(
          vline, currentRow, currentColumn++, 1, 1);

      auto usedSynapsesString = QString::number( syn.second );
      layoutWidget->addWidget(
          new QLabel( usedSynapsesString ), currentRow, currentColumn, 1, 1 );

  //    layoutPre->addWidget( new QLabel( "Total Synapses" ), currentRow, currentColumn++, 1, 1 );

//      std::cout << " " << gidString.toStdString( ) << "->" << usedSynapsesString.toStdString( ) << std::endl;

      ++currentRow;
    }

    _layoutInfo->addWidget( widget );

  }

  update( );
}

void MainWindow::clearInfoDock( void )
{
  if( _widgetInfoPre )
  {
    _layoutInfo->removeWidget( _widgetInfoPre );
    delete _widgetInfoPre;
    _widgetInfoPre = nullptr;
  }

  if( _widgetInfoPost )
  {
    _layoutInfo->removeWidget( _widgetInfoPost );
    delete _widgetInfoPost;
    _widgetInfoPost = nullptr;
  }
}

void MainWindow::loadData( const std::string& dataset,
                           const std::string& target )
{
  _openGLWidget->loadBlueConfig( dataset, target );

  loadPresynapticList( );
}

void MainWindow::loadPresynapticList( void )
{
  nsol::DataSet* nsolData = _openGLWidget->dataset( );

  if( _modelListPre )
    _modelListPre->clear( );
  else
    _modelListPre = new QStandardItemModel( );

  QStandardItem* item;
  for( auto neuron : nsolData->neurons( ))
  {
    item = new QStandardItem( );
    item->setData( neuron.first, Qt::DisplayRole );

    _modelListPre->appendRow( item );
  }

  _modelListPre->sort( 0, Qt::AscendingOrder );

  _listPresynaptic->setModel( _modelListPre );
  update( );
}

void MainWindow::loadPostsynapticList( unsigned int gid )
{
  nsol::DataSet* nsolData = _openGLWidget->dataset( );

  if( _modelListPost )
    _modelListPost->clear( );
  else
    _modelListPost = new QStandardItemModel( );

//  auto selection =
//      _modelListPre->itemFromIndex( _listPresynaptic->selectionModel( )->selectedIndexes( ).first( ))->data().value< unsigned int >();

  std::set< unsigned int > selection = { gid };
  auto synapses = nsolData->circuit( ).synapses( selection, nsol::Circuit::PRESYNAPTICCONNECTIONS );

  selection.clear( );

  QStandardItem* item;
  for( auto syn : synapses )
  {
    unsigned int postGid = syn->postSynapticNeuron( );
    if( selection.find( postGid ) != selection.end( ))
      continue;

    item = new QStandardItem( );
    item->setData( postGid, Qt::DisplayRole );

    _modelListPost->appendRow( item );

    selection.insert( postGid );
  }

  _modelListPost->sort( 0, Qt::SortOrder::AscendingOrder );

  _listPostsynaptic->setModel( _modelListPost );

  update( );
}


void MainWindow::showStatusBarMessage ( const QString& message )
{
  _ui->statusbar->showMessage( message );
}

void MainWindow::presynapticNeuronClicked( const QModelIndex& index )
{

  if( _openGLWidget->mode( ) == syncopa::PATHS )
  {
    QStandardItem* value = _modelListPre->itemFromIndex( index );
    unsigned int gid = value->data( Qt::DisplayRole ).value< unsigned int >( );

    _openGLWidget->selectPresynapticNeuron( gid );

    loadPostsynapticList( gid );

    std::cout << "Selected pre: " << gid << std::endl;
  }
  else
  {
    std::vector< unsigned int > selection;
    for( auto item : _listPresynaptic->selectionModel( )->selectedIndexes( ))
    {
      selection.push_back( _modelListPre->itemFromIndex( item )->data( Qt::DisplayRole ).value< unsigned int >( ));
    }

    _openGLWidget->selectPresynapticNeuron( selection );

    std::cout << "Selected pre: ";
    for( auto gid : selection )
      std::cout << " " << gid;
    std::cout << std::endl;
  }

  _buttonDynamic->setChecked( false );
  _buttonDynamic->setEnabled( false );

  updateInfoDock( );
}

void MainWindow::postsynapticNeuronClicked( const QModelIndex&  )
{
  std::vector< unsigned int > selection;
  for( auto item : _listPostsynaptic->selectionModel( )->selectedIndexes( ))
  {
    selection.push_back( _modelListPost->itemFromIndex( item )->data( Qt::DisplayRole ).value< unsigned int >( ));
  }

  _openGLWidget->selectPostsynapticNeuron( selection );

  _buttonDynamic->setChecked( false );
  _buttonDynamic->setEnabled( true );

  std::cout << "Selected post: ";
  for( auto gid : selection )
    std::cout << " " << gid;
  std::cout << std::endl;

  updateInfoDock( );
}


void MainWindow::setSynapseMappingState( int state )
{
//  if( !state )
//  {
//    // Disable GUI
//
//    std::cout << "Normal mode" << std::endl;
//  }
//  else
//  {
//
//    // Enable GUI
//    std::cout << "Mapping mode" << std::endl;
//  }

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
//  _frameColorSynapseMapGradient->setEnabled( state );

  _openGLWidget->setSynapseMappingState( state );
//  setSynapseMappingAttribute( _comboSynapseMapAttrib->currentIndex( ));

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
  QColor result = QColorDialog::getColor( current, this,
                                          QString( message ),
                                          QColorDialog::DontUseNativeDialog);

  if( result.isValid( ))
  {
    current = result;
    return true;
  }
  else
    return false;

}

void MainWindow::colorSelectionClicked( void )
{
  QPushButton* frame = dynamic_cast< QPushButton* >( sender( ));
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
  QSlider* source = dynamic_cast< QSlider* >( sender( ));

  float normValue = ( position - _sliderMin) * _invRangeSliders;

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
  QDoubleSpinBox* source = dynamic_cast< QDoubleSpinBox* >( sender( ));

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
  QPushButton* source = dynamic_cast< QPushButton* >( sender( ));

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
//  _colorMapWidget->hide( );
}

void MainWindow::dynamic( void )
{
  if( _buttonDynamic->isChecked( ))
    _openGLWidget->startDynamic( );
  else
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
  TMode mode = selectedModeSynapses ? syncopa::SYNAPSES : syncopa::PATHS;

  if( _listPresynaptic->selectionModel( ) &&
      _listPresynaptic->selectionModel( )->selectedIndexes( ).size( ) != 1 )
    _listPresynaptic->clearSelection( );
  else if ( !selectedModeSynapses && _modelListPost )
  {
    unsigned int idx =
        _modelListPre->itemFromIndex(
            _listPresynaptic->selectionModel( )->selectedIndexes( ).front( ))->
            data( Qt::DisplayRole ).value< unsigned int >( );

    loadPostsynapticList( idx );
  }
  else
    _listPostsynaptic->clearSelection( );

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

    _buttonShowFullMorphoPre->setEnabled( true );
    _buttonShowFullMorphoPost->setEnabled( true );
    _buttonShowFullMorphoContext->setEnabled( true );
    _buttonShowFullMorphoOther->setEnabled( true );

    _buttonShowFullMorphoContext->setChecked( false );
    _buttonShowFullMorphoOther->setChecked( false );

  }

  _buttonDynamic->setChecked( false );
  _buttonDynamic->setEnabled( false );

  if( selectedModeSynapses && _modelListPost )
    _modelListPost->clear( );

  _listPostsynaptic->setEnabled( !selectedModeSynapses );

  _groupBoxPaths->setEnabled( !selectedModeSynapses );

  _listPresynaptic->setSelectionMode( selectedModeSynapses ?
                                      QAbstractItemView::ExtendedSelection :
                                      QAbstractItemView::SingleSelection );

  _openGLWidget->mode( mode );


}

