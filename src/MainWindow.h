/*
 * @file  MainWindow.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include <QMainWindow>
#include <QDockWidget>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QRadioButton>
#include <QListView>
#include <QTableView>
#include <QStandardItemModel>


#include "OpenGLWidget.h"

#include "ui_streaminApp.h"

namespace Ui
{
class MainWindow;
}

class MainWindow
  : public QMainWindow
{
  Q_OBJECT

public:

  explicit MainWindow( QWidget* parent = 0,
                       bool updateOnIdle = true );
  ~MainWindow( void );

  void init( void );

  void loadData( const std::string& dataset, const std::string& target );

  void showStatusBarMessage ( const QString& message );

protected slots:

  void presynapticNeuronClicked( const QModelIndex& index );
  void postsynapticNeuronClicked( const QModelIndex& index );


  void clear( void );

protected:

  void initListDock( void );
  void initInfoDock( void );

  void loadPresynapticList( void );
  void loadPostsynapticList( unsigned int gid );

  Ui::MainWindow* _ui;
  OpenGLWidget* _openGLWidget;

  QDockWidget* _dockList;
  QListView* _listPresynaptic;
  QStandardItemModel* _modelListPre;

  QListView* _listPostsynaptic;
  QStandardItemModel* _modelListPost;

  QDockWidget* _dockInfo;
  QTableView* _tableInfo;
  QStandardItemModel* _modelTableInfo;
};
