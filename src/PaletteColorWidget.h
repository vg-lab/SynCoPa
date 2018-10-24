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

#include <scoop/scoop.h>
#include "GradientWidget.h"

#include "types.h"

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

  void init( void );

  void loadScoopPalletes( unsigned int colorsNumber = 4 );

  tQColorVec getColors( void ) const;
  const QGradientStops& getGradientStops( void ) const;

signals:

  void acceptClicked( );
  void cancelClicked( );

protected slots:

  void buttonAcceptClicked( void );
  void buttonCancelClicked( void );

  void radioButtonClicked( bool checked );
  void paletteSelectionChanged( int pallete );

protected:

  void _fillPaletteNames( void );
  void _fillColors( void );

  int _selectionState;
  int _currentPallete;
  unsigned int _paletteSize;

  tQColorVec _currentColors;

  GradientWidget* _frameResult;

  QRadioButton* _radioSequential;
  QRadioButton* _radioCategorical;
//  QRadioButton* _radioCustom; //TODO

  QComboBox* _comboPalettes;

  QPushButton* _buttonAccept;
  QPushButton* _buttonCancel;

};

#endif /* SRC_PALETTECOLORWIDGET_H_ */
