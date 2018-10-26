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
#include <QGroupBox>
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
, _dockInfo( nullptr )
, _layoutInfo( nullptr )
, _widgetInfoPre( nullptr )
, _widgetInfoPost( nullptr )
, _dockColor( nullptr )
, _frameColorMorphoPre( nullptr )
, _frameColorMorphoPost( nullptr )
, _frameColorMorphoRelated( nullptr )
, _frameColorMorphoContext( nullptr )
, _frameColorSynapsesPre( nullptr )
, _frameColorSynapsesPost( nullptr )
, _frameColorPathsPre( nullptr )
, _frameColorPathsPost( nullptr )
, _checkMorphoPre( nullptr )
, _checkMorphoPost( nullptr )
, _checkMorphoRelated( nullptr )
, _checkMorphoContext( nullptr )
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
, _sliderAlphaSynapseMap( nullptr )
, _spinBoxSynapsesPre( nullptr )
, _spinBoxSynapsesPost( nullptr )
, _spinBoxPathsPre( nullptr )
, _spinBoxPathsPost( nullptr )
, _spinBoxSynapseMapSize( nullptr )
, _labelTransSynPre( nullptr )
, _labelTransSynPost( nullptr )
, _labelTransPathPre( nullptr )
, _labelTransPathPost( nullptr )
, _labelTransSynMap( nullptr )

, _buttonDynamic( nullptr )
, _comboSynapseMapAttrib( nullptr )

{
  _ui->setupUi( this );

  _ui->actionUpdateOnIdle->setChecked( updateOnIdle );
  _ui->actionShowFPSOnIdleUpdate->setChecked( false );


  connect( _ui->actionQuit, SIGNAL( triggered( )),
           QApplication::instance(), SLOT( quit( )));

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
}

void MainWindow::initListDock( void )
{
  _listPresynaptic = new QListView( );
  _listPresynaptic->setMaximumWidth( 100 );

  connect( _listPresynaptic, SIGNAL( clicked( const QModelIndex& )),
           this, SLOT( presynapticNeuronClicked( const QModelIndex& )));

  _listPostsynaptic = new QListView( );
  _listPostsynaptic->setMaximumWidth( 100 );
  _listPostsynaptic->setSelectionMode( QAbstractItemView::ExtendedSelection );

  connect( _listPostsynaptic, SIGNAL( clicked( const QModelIndex& )),
           this, SLOT( postsynapticNeuronClicked( const QModelIndex& )));


  QDockWidget* dockList = new QDockWidget( tr( "Selection" ));
  dockList->setMaximumHeight( 250 );

  QGridLayout* dockLayout = new QGridLayout( );
//  dockList->setLayout( dockLayout );
  QWidget* container = new QWidget( );
  container->setLayout( dockLayout );

  QPushButton* clearButton = new QPushButton( "Clear selection" );
  connect( clearButton, SIGNAL( clicked( void )),
           this, SLOT( clear( void )));

  dockLayout->addWidget( new QLabel( "Presynaptic:" ), 0, 0, 1, 1 );
  dockLayout->addWidget( new QLabel( "Postsynaptic:" ), 0, 1, 1, 1 );
  dockLayout->addWidget( _listPresynaptic, 1, 0, 1, 1 );
  dockLayout->addWidget( _listPostsynaptic, 1, 1, 1, 1);
  dockLayout->addWidget( clearButton );

  dockList->setWidget( container );

  addDockWidget( Qt::RightDockWidgetArea, dockList );

}

void MainWindow::initColorDock( void )
{
  _dockColor = new QDockWidget( "Aspect" );

  QScrollArea* scrollArea = new QScrollArea( );

  QWidget* container = new QWidget( );

  QVBoxLayout* layout = new QVBoxLayout( );

  container->setLayout( layout );

  QGroupBox* groupBoxMorphologies = new QGroupBox( "Morphologies" );
  QGroupBox* groupBoxSynapses = new QGroupBox( "Synapses" );
  QGroupBox* groupBoxPaths = new QGroupBox( "Paths" );

  _checkMorphoPre = new QCheckBox( "Presynaptic" );
  _checkMorphoPost = new QCheckBox( "Postsynaptic" );
  _checkMorphoRelated = new QCheckBox( "Related" );
  _checkMorphoContext = new QCheckBox( "Context" );
  _checkSynapsesPre = new QCheckBox( "Presynaptic" );
  _checkSynapsesPost = new QCheckBox( "Postsynaptic" );
  _checkPathsPre = new QCheckBox( "Presynaptic" );
  _checkPathsPost = new QCheckBox( "Postsynaptic" );

  _checkMorphoPre->setChecked( true );
  _checkMorphoPost->setChecked( true );
  _checkMorphoRelated->setChecked( true );
  _checkMorphoContext->setChecked( true );
  _checkSynapsesPre->setChecked( true );
  _checkSynapsesPost->setChecked( true );
  _checkPathsPre->setChecked( true );
  _checkPathsPost->setChecked( true );

  QGridLayout* morphoLayout = new QGridLayout( );
  groupBoxMorphologies->setLayout( morphoLayout );

  QGridLayout* synLayout = new QGridLayout( );
  groupBoxSynapses->setLayout( synLayout );

  QGridLayout* pathLayout = new QGridLayout( );
  groupBoxPaths->setLayout( pathLayout );

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

  _frameColorMorphoRelated = new QPushButton( );
  _frameColorMorphoRelated->setMinimumSize( 20, 20 );
  _frameColorMorphoRelated->setMaximumSize( 20, 20 );

  color = colorEigenToQt(_openGLWidget->colorRelated( ));
  _frameColorMorphoRelated->setStyleSheet( "background-color: " + color.name( ));

  _frameColorMorphoContext = new QPushButton( );
  _frameColorMorphoContext->setMinimumSize( 20, 20 );
  _frameColorMorphoContext->setMaximumSize( 20, 20 );

  color = colorEigenToQt( _openGLWidget->colorContext( ));
  _frameColorMorphoContext->setStyleSheet( "background-color: " + color.name( ));

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
  morphoLayout->addWidget( _checkMorphoPre, row, col++, 1, 2 );
//  morphoLayout->addWidget( new QLabel( "Presynaptic" ), row, col++, 1, 2 );
  ++col;

  morphoLayout->addWidget( _frameColorMorphoPost, row, col++, 1, 1 );
  morphoLayout->addWidget( _checkMorphoPost, row, col++, 1, 2 );
//  morphoLayout->addWidget( new QLabel( "Postsynaptic" ), row, col, 1, 2 );

  ++row;
  col = 0;

  morphoLayout->addWidget( _frameColorMorphoRelated, row, col++, 1, 1 );
  morphoLayout->addWidget( _checkMorphoRelated, row, col++, 1, 2 );
//  morphoLayout->addWidget( new QLabel( "Related" ), row, col++, 1, 2 );
  ++col;

  morphoLayout->addWidget( _frameColorMorphoContext, row, col++, 1, 1 );
  morphoLayout->addWidget( _checkMorphoContext, row, col++, 1, 2 );
//  morphoLayout->addWidget( new QLabel( "Other" ), row, col, 1, 2 );
//TODO
  // Synapses

  _colorMapWidget = new PaletteColorWidget( );
  _colorMapWidget->init( false );

  connect( _colorMapWidget, SIGNAL( acceptClicked( void )),
           this, SLOT( colorSynapseMapAccepted( void )));


  _sliderAlphaSynapsesPre = new QSlider( Qt::Horizontal );
  _sliderAlphaSynapsesPost = new QSlider( Qt::Horizontal );
  _sliderAlphaPathsPre = new QSlider( Qt::Horizontal );
  _sliderAlphaPathsPost = new QSlider( Qt::Horizontal );
  _sliderAlphaSynapseMap = new QSlider( Qt::Horizontal );


  _labelTransSynPre = new QLabel( );
  _labelTransSynPost = new QLabel( );
  _labelTransPathPre = new QLabel( );
  _labelTransPathPost = new QLabel( );
  _labelTransSynMap = new QLabel( );

  _spinBoxSynapsesPre = new QDoubleSpinBox( );
  _spinBoxSynapsesPre->setValue( 3.0f );

  _spinBoxSynapsesPost = new QDoubleSpinBox( );
  _spinBoxSynapsesPost->setValue( 3.0f );

  _spinBoxPathsPre = new QDoubleSpinBox( );
  _spinBoxPathsPre->setValue( 3.0f );

  _spinBoxPathsPost = new QDoubleSpinBox( );
  _spinBoxPathsPost->setValue( 3.0f );

  _spinBoxSynapseMapSize = new QDoubleSpinBox( );
  _spinBoxPathsPost->setValue( 3.0f );

  QCheckBox* checkMapSynapses = new QCheckBox( tr( "Map color" ));
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
//  ++col;
  synLayout->addWidget( _spinBoxSynapsesPre, row, col++, 1, 1 );
  synLayout->addWidget( _sliderAlphaSynapsesPre, row, col, 1, 2 );

  ++row;
  col = 0;

  synLayout->addWidget( _frameColorSynapsesPost, row, col++, 1, 1 );
  synLayout->addWidget( _checkSynapsesPost, row, col++, 1, 2 );
//  synLayout->addWidget( new QLabel( "Postsynaptic" ), row, col++, 1, 2 );
//  ++col;
  synLayout->addWidget( _spinBoxSynapsesPost, row, col++, 1, 1 );
  synLayout->addWidget( _sliderAlphaSynapsesPost, row, col, 1, 2 );

  ++row;
  col = 0;

  synLayout->addWidget( line, row, col, 1, 6 );

  ++row;
  synLayout->addWidget( checkMapSynapses, row, col++, 1, 2 );
  ++row;
  col = 0;
  synLayout->addWidget( new QLabel( tr( "Synapse attribute" )), row, col++, 1, 2 );
  ++col;
  synLayout->addWidget( _comboSynapseMapAttrib, row, col, 1, 2 );

  ++row;
  col = 0;

//  synLayout->addWidget( _frameColorSynapseMapGradient, row, col++, 1, 2 );
//  ++col;

  synLayout->addWidget( _spinBoxSynapseMapSize, row, col++, 1, 1 );
  synLayout->addWidget( _sliderAlphaSynapseMap, row, col, 1, 2 );


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
  pathLayout->addWidget( _spinBoxPathsPre, row, col++, 1, 1 );
  pathLayout->addWidget( _sliderAlphaPathsPre, row, col, 1, 2 );

  ++row;
  col = 0;

  pathLayout->addWidget( _frameColorPathsPost, row, col++, 1, 1 );
  pathLayout->addWidget( _checkPathsPost, row, col++, 1, 2 );
//  pathLayout->addWidget( new QLabel( "Postsynaptic" ), row, col++, 1, 2 );
  ++col;
  pathLayout->addWidget( _spinBoxPathsPost, row, col++, 1, 1 );
  pathLayout->addWidget( _sliderAlphaPathsPost, row, col, 1, 2 );

  layout->addWidget( groupBoxMorphologies );
  layout->addWidget( groupBoxSynapses );
  layout->addWidget( groupBoxPaths );

  _buttonDynamic = new QPushButton( "Dynamic" );
  _buttonDynamic->setCheckable( true );
  connect( _buttonDynamic, SIGNAL( clicked( )), this, SLOT( dynamic( )));

  layout->addWidget( _buttonDynamic );

  connect( _frameColorMorphoPre, SIGNAL( clicked( )),
           this, SLOT( colorSelectionClicked()));

  connect( _frameColorMorphoPost, SIGNAL( clicked( )),
           this, SLOT( colorSelectionClicked()));

  connect( _frameColorMorphoRelated, SIGNAL( clicked( )),
           this, SLOT( colorSelectionClicked()));

  connect( _frameColorMorphoContext, SIGNAL( clicked( )),
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

  connect(  _checkMorphoPre, SIGNAL( stateChanged( int )),
           _openGLWidget, SLOT( showSelectedPre( int )));
  connect(  _checkMorphoPost, SIGNAL( stateChanged( int )),
           _openGLWidget, SLOT( showSelectedPost( int )));
  connect(  _checkMorphoRelated, SIGNAL( stateChanged( int )),
           _openGLWidget, SLOT( showRelated( int )));
  connect(  _checkMorphoContext, SIGNAL( stateChanged( int )),
           _openGLWidget, SLOT( showContext( int )));
  connect(  _checkSynapsesPre, SIGNAL( stateChanged( int )),
           _openGLWidget, SLOT( showSynapsesPre( int )));
  connect(  _checkSynapsesPost, SIGNAL( stateChanged( int )),
           _openGLWidget, SLOT( showSynapsesPost( int )));
  connect(  _checkPathsPre, SIGNAL( stateChanged( int )),
           _openGLWidget, SLOT( showPathsPre( int )));
  connect(  _checkPathsPost, SIGNAL( stateChanged( int )),
           _openGLWidget, SLOT( showPathsPost( int )));

  checkMapSynapses->setChecked( true );
  connect( checkMapSynapses, SIGNAL( stateChanged( int )),
           this, SLOT( setSynapseMappingState( int )));

  connect( _comboSynapseMapAttrib, SIGNAL( currentIndexChanged( int )),
           this, SLOT( setSynapseMappingAttribute( int )));

  checkMapSynapses->setChecked( false );

  scrollArea->setWidget( container );

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

  for( unsigned int i = 0; i < 2; ++i )
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
  unsigned int gid;

  QStandardItem* value = _modelListPre->itemFromIndex( index );
  gid = value->data( Qt::DisplayRole ).value< unsigned int >( );

  _openGLWidget->selectPresynapticNeuron( gid );

  _buttonDynamic->setChecked( false );

  std::cout << "Selected pre: " << gid << std::endl;

  loadPostsynapticList( gid );

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
  _spinBoxSynapsesPre->setEnabled( !state );
  _spinBoxSynapsesPost->setEnabled( !state );

  _comboSynapseMapAttrib->setEnabled( state );
  _sliderAlphaSynapseMap->setEnabled( state );
  _spinBoxSynapseMapSize->setEnabled( state );
  _colorMapWidget->setEnabled( state );
//  _frameColorSynapseMapGradient->setEnabled( state );

  _openGLWidget->setSynapseMappingState( state );

  _colorMapWidget->setPlot( _openGLWidget->getSynapseMappingPlot( ));
}

void MainWindow::setSynapseMappingAttribute( int attrib )
{
  _openGLWidget->setSynapseMapping( attrib );
  _colorMapWidget->setPlot( _openGLWidget->getSynapseMappingPlot( ));
}

void MainWindow::clear( void )
{
  _openGLWidget->clear( );

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
    else if( frame == _frameColorMorphoRelated )
      color = colorEigenToQt( _openGLWidget->colorRelated( ));
    else if( frame == _frameColorMorphoContext )
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
      else if( frame == _frameColorMorphoRelated )
        _openGLWidget->colorRelated( colorQtToEigen( color ));
      else if( frame == _frameColorMorphoContext )
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

void MainWindow::colorSynapseMapClicked( void )
{
//  if( !_colorMapWidget )
//  {
//    _colorMapWidget = new PaletteColorWidget( );
//    _colorMapWidget->init( );
//
//    connect( _colorMapWidget, SIGNAL( acceptClicked( void )),
//             this, SLOT( colorSynapseMapAccepted( void )));
//
//    connect( _colorMapWidget, SIGNAL( cancelClicked( void )),
//                 this, SLOT( colorSynapseMapCancelled( void )));
//  }

//  _colorMapWidget->show( );
}

void MainWindow::colorSynapseMapAccepted( void )
{
//  _frameColorSynapseMapGradient->setGradientStops(
//      _colorMapWidget->getGradientStops( ));

  _openGLWidget->colorSynapseMap( _colorMapWidget->getColors( ));

//  _colorMapWidget->hide( );
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

