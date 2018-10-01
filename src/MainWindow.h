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
#include <QGridLayout>
#include <QDoubleSpinBox>

#include "OpenGLWidget.h"

#include "ui_syncopa.h"

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

  void colorSelectionClicked( void );

  void clear( void );

  void dynamic( void );

protected:

  bool showDialog( QColor& current, const QString& message = "" );

  void initListDock( void );
  void initColorDock( void );
  void initInfoDock( void );
  void updateInfoDock( void );
  void clearInfoDock( void );

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
  QVBoxLayout* _layoutInfo;
  QWidget* _widgetInfoPre;
  QWidget* _widgetInfoPost;

  QDockWidget* _dockColor;
  QPushButton* _frameColorMorphoPre;
  QPushButton* _frameColorMorphoPost;
  QPushButton* _frameColorMorphoRelated;
  QPushButton* _frameColorMorphoContext;
  QPushButton* _frameColorSynapsesPre;
  QPushButton* _frameColorSynapsesPost;
  QPushButton* _frameColorPathsPre;
  QPushButton* _frameColorPathsPost;

  QSlider* _sliderAlphaSynapsesPre;
  QSlider* _sliderAlphaSynapsesPost;
  QSlider* _sliderAlphaPathsPre;
  QSlider* _sliderAlphaPathsPost;

  QDoubleSpinBox* _spinBoxSynapsesPre;
  QDoubleSpinBox* _spinBoxSynapsesPost;
  QDoubleSpinBox* _spinBoxPathsPre;
  QDoubleSpinBox* _spinBoxPathsPost;

  QPushButton* _buttonDynamic;
};
