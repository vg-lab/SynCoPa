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
//, _dockList( nullptr )
, _dockInfo( nullptr )
, _listPresynaptic( nullptr )
, _modelListPre( nullptr )
, _listPostsynaptic( nullptr )
, _modelListPost( nullptr )
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

  _listPresynaptic = new QListView( );

  connect( _listPresynaptic, SIGNAL( clicked( const QModelIndex& )),
           this, SLOT( presynapticNeuronClicked( const QModelIndex& )));

  _listPostsynaptic = new QListView( );

  connect( _listPostsynaptic, SIGNAL( clicked( const QModelIndex& )),
           this, SLOT( postsynapticNeuronClicked( const QModelIndex& )));


  QDockWidget* dockList = new QDockWidget( tr( "Data" ));


  QVBoxLayout* dockLayout = new QVBoxLayout( );
//  dockList->setLayout( dockLayout );
  QWidget* container = new QWidget( );
  container->setLayout( dockLayout );

  QPushButton* clearButton = new QPushButton( "clear" );
  connect( clearButton, SIGNAL( clicked( void )),
           this, SLOT( clear( void )));

  dockLayout->addWidget( _listPresynaptic );
  dockLayout->addWidget( _listPostsynaptic );
  dockLayout->addWidget( clearButton );


//  dockLayout->addWidget( _listPresynaptic );
  dockList->setWidget( container );

  addDockWidget( Qt::RightDockWidgetArea, dockList );


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
}

void MainWindow::clear( void )
{
  _openGLWidget->clear( );
}
