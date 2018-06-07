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
// #include "qt/CustomSlider.h"

#ifdef VISIMPL_USE_GMRVLEX
  #include <gmrvlex/gmrvlex.h>
#endif

#include <thread>

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

  initListDock( );
  initInfoDock( );

  _openGLWidget->createParticleSystem( );
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
  dockList->setMaximumHeight( 500 );

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

void MainWindow::initInfoDock( void )
{
  _dockInfo = new QDockWidget( tr( "Information" ));

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

  std::cout << "Selected post: ";
  for( auto gid : selection )
    std::cout << " " << gid;
  std::cout << std::endl;

  updateInfoDock( );
}

void MainWindow::clear( void )
{
  _openGLWidget->clear( );

  _modelListPost->clear( );

  updateInfoDock( );
}
