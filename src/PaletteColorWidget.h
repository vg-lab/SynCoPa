/*
 * @file  PaletteColorWidget.h
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */
#ifndef SRC_PALETTECOLORWIDGET_H_
#define SRC_PALETTECOLORWIDGET_H_

#include <QRadioButton>
#include <QComboBox>
#include <QPushButton>
#include <QCheckBox>

#include <scoop/scoop.h>
#include "GradientWidget.h"

#include "types.h"

#include "ext/ctkrangeslider.h"

class PaletteColorWidget : public QWidget
{

  Q_OBJECT

  enum tColorType
  {
    PALETTE_SEQUENTIAL = 0,
    PALETTE_CATEGORICAL
  };

public:

  PaletteColorWidget( QWidget* parent_ = nullptr );

  void init( bool dialog = true );

  tQColorVec getColors( void ) const;
  const QGradientStops& getGradientStops( void ) const;

  void setPlot( QPolygonF plot );

  bool filter( void ) const;
  std::pair< float, float > filterBounds( void ) const;

signals:

  void acceptClicked( );
  void cancelClicked( );

  void filterStateChanged( );
  void filterBoundsChanged( );

protected slots:

  void buttonAcceptClicked( void );
  void buttonCancelClicked( void );

  void radioButtonClicked( bool checked );
  void paletteSelectionChanged( int pallete );

  void checkInvertColorsToggled( int checked );

  void setFilterActive( bool active );
  void filterSliderChanged( int min, int max );

protected:

  void _fillPaletteNames( void );
  void _fillColors( void );

  int _selectionState;
  int _currentPalette;
  unsigned int _paletteSize;
  bool _invertPaletteColors;

  tQColorVec _currentColors;

  GradientWidget* _frameResult;

  QRadioButton* _radioSequential;
  QRadioButton* _radioCategorical;
//  QRadioButton* _radioCustom; //TODO

  QCheckBox* _checkInvertPalette;
  QCheckBox* _checkFilterActive;

  QComboBox* _comboPalettes;

  QPushButton* _buttonApply;
  QPushButton* _buttonCancel;

  ctkRangeSlider* _rangeFilterSlider;
  int _minPosSlider;
  int _maxPosSlider;
  float _invPosSlider;

  bool _filtering;
  float _currentLowerLimit;
  float _currentUpperLimit;
};

#endif /* SRC_PALETTECOLORWIDGET_H_ */
