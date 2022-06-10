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
#include <QLabel>

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
    PALETTE_CATEGORICAL,
    PALETTE_DIVERGING,
    PALETTE_UNIFORM
  };

public:

  PaletteColorWidget( QWidget* parent_ = nullptr );

  void init( bool dialog = true );

  tQColorVec getColors( void ) const;
  const QGradientStops& getGradientStops( void ) const;

  void setPlot( QPolygonF plot, float minRange, float maxRange );

  bool filter( void ) const;
  std::pair< float, float > filterBounds( void ) const;

signals:

  void acceptClicked( );
  void cancelClicked( );

  void filterStateChanged( );
  void filterBoundsChanged( );
  void filterPaletteChanged();

protected slots:

  void buttonAcceptClicked( void );
  void buttonCancelClicked( void );

  void paletteTypeChanged( int index );
  void paletteSelectionChanged( int pallete );

  void checkInvertColorsToggled( int checked );

  void setFilterActive( bool active );
  void filterSliderChanged( );

protected:

  void _fillPaletteNames( void );
  void _fillColors( void );

  int _selectionState;
  int _currentPalette;
  unsigned int _paletteSize;
  bool _invertPaletteColors;

  tQColorVec _currentColors;

  GradientWidget* _frameResult;

  QCheckBox* _checkInvertPalette;
  QCheckBox* _checkFilterActive;

  QComboBox* _comboPalettes;

  QPushButton* _buttonApply;
  QPushButton* _buttonCancel;

  QLabel* _labelTotalRange;
  QLabel* _labelActualRange;

  ctkRangeSlider* _rangeFilterSlider;

  bool _filtering;
  float _currentLowerLimit;
  float _currentUpperLimit;

  float _minRange;
  float _maxRange;

  float _currentMinValue;
  float _currentMaxValue;

};

#endif /* SRC_PALETTECOLORWIDGET_H_ */
