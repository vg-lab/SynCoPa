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

protected:

  Ui::MainWindow* _ui;
  OpenGLWidget* _openGLWidget;
};
