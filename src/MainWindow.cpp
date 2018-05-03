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

  _openGLWidget->createParticleSystem( 1000, 10);
  _openGLWidget->idleUpdate( _ui->actionUpdateOnIdle->isChecked( ));

  connect( _ui->actionUpdateOnIdle, SIGNAL( triggered( )),
           _openGLWidget, SLOT( toggleUpdateOnIdle( )));

  connect( _ui->actionBackgroundColor, SIGNAL( triggered( )),
           _openGLWidget, SLOT( changeClearColor( )));

  connect( _ui->actionShowFPSOnIdleUpdate, SIGNAL( triggered( )),
           _openGLWidget, SLOT( toggleShowFPS( )));
}

void MainWindow::loadData( const std::string& dataset,
                           const std::string& target )
{
  _openGLWidget->loadBlueConfig( dataset, target );
}


void MainWindow::showStatusBarMessage ( const QString& message )
{
  _ui->statusbar->showMessage( message );
}
