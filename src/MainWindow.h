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
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QPolygonF>

#include "GradientWidget.h"

#include "OpenGLWidget.h"
#include "PaletteColorWidget.h"

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

  void setSynapseMappingState( int state );
  void setSynapseMappingAttribute( int attrib );

  void colorSelectionClicked( void );
  void colorSynapseMapAccepted( void );
  void colorSynapseMapCancelled( void );

  void transparencySliderMoved( int );

  void sizeSpinBoxChanged( double );

  void showFullMorphologyChecked( bool );

  void filteringStateChanged( void );
  void filteringBoundsChanged( void );

  void modeChanged( bool state );
  void alphaModeChanged( bool state );

  void clear( void );

  void dynamicStart( void );
  void dynamicPause( void );
  void dynamicStop( void );

protected:

  bool showDialog( QColor& current, const QString& message = "" );

  void initListDock( void );
  void initColorDock( void );
  void initInfoDock( void );

  void updateInfoDock( void );
  void clearInfoDock( void );

  void loadPresynapticList( void );
  void loadPostsynapticList( unsigned int gid );

  void _loadDefaultValues( void );

  Ui::MainWindow* _ui;
  OpenGLWidget* _openGLWidget;

  QDockWidget* _dockList;
  QListView* _listPresynaptic;
  QStandardItemModel* _modelListPre;

  QListView* _listPostsynaptic;
  QStandardItemModel* _modelListPost;

  QRadioButton* _radioModeSynapses;
  QRadioButton* _radioModePaths;

  QRadioButton* _radioAlphaModeNormal;
  QRadioButton* _radioAlphaModeAccumulative;

  QDockWidget* _dockInfo;
  QVBoxLayout* _layoutInfo;
  QWidget* _widgetInfoPre;
  QWidget* _widgetInfoPost;

  QDockWidget* _dockColor;
  QPushButton* _frameColorMorphoPre;
  QPushButton* _frameColorMorphoPost;
  QPushButton* _frameColorMorphoContext;
  QPushButton* _frameColorMorphoOther;
  QPushButton* _frameColorSynapsesPre;
  QPushButton* _frameColorSynapsesPost;
  QPushButton* _frameColorPathsPre;
  QPushButton* _frameColorPathsPost;

  QPushButton* _buttonShowFullMorphoPre;
  QPushButton* _buttonShowFullMorphoPost;
  QPushButton* _buttonShowFullMorphoContext;
  QPushButton* _buttonShowFullMorphoOther;

  QIcon _fullMorphoOn;
  QIcon _fullMorphoOff;

  QCheckBox* _checkShowMorphoPre;
  QCheckBox* _checkShowMorphoPost;
  QCheckBox* _checkShowMorphoContext;
  QCheckBox* _checkShowMorphoOther;
  QCheckBox* _checkSynapsesPre;
  QCheckBox* _checkSynapsesPost;
  QCheckBox* _checkPathsPre;
  QCheckBox* _checkPathsPost;

//  GradientWidget* _frameColorSynapseMapGradient;
  PaletteColorWidget* _colorMapWidget;

  QSlider* _sliderAlphaSynapsesPre;
  QSlider* _sliderAlphaSynapsesPost;
  QSlider* _sliderAlphaPathsPre;
  QSlider* _sliderAlphaPathsPost;

  QSlider* _sliderAlphaSynapsesMap;

  float _invRangeSliders;
  int _sliderMin;
  int _sliderMax;

  QDoubleSpinBox* _spinBoxSizeSynapsesPre;
  QDoubleSpinBox* _spinBoxSizeSynapsesPost;
  QDoubleSpinBox* _spinBoxSizePathsPre;
  QDoubleSpinBox* _spinBoxSizePathsPost;

  QDoubleSpinBox* _spinBoxSizeSynapsesMap;

  QLabel* _labelTransSynPre;
  QLabel* _labelTransSynPost;
  QLabel* _labelTransPathPre;
  QLabel* _labelTransPathPost;
  QLabel* _labelTransSynMap;

  QPushButton* _buttonDynamicStart;
  QPushButton* _buttonDynamicStop;

  QComboBox* _comboSynapseMapAttrib;

  QGroupBox* _groupBoxGeneral;
  QGroupBox* _groupBoxMorphologies;
  QGroupBox* _groupBoxSynapses;
  QGroupBox* _groupBoxPaths;
  QGroupBox* _groupBoxDynamic;
};
