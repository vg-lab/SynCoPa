/*
 * @file  PalleteColorWidget.cpp
 * @brief
 * @author Sergio E. Galindo <sergio.galindo@urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#include "PaletteColorWidget.h"

#include <QGroupBox>
#include <QGridLayout>
#include <assert.h>

using tscoopp = scoop::ColorPalette;
using tpSeq = tscoopp::ColorBrewerSequential;
using tpCat = tscoopp::ColorBrewerQualitative;

const static std::vector< std::string > paletteNamesSequential =
{
   "BuGn", "BuPu", "GnBu", "OrRd", "PuBu", "PuBuGn",
   "PuRd", "RdPu", "YlGn", "YlGnBu", "YlOrBr", "YlOrRd",
   "Blues", "Greens", "Greys", "Oranges", "Purples", "Reds"
};

const static std::vector< std::string > paletteNamesCategoric =
{
 "Accent", " Dark2", " Paired", " Pastel1", "Pastel2", " Set1", " Set2", " Set3"
};

PaletteColorWidget::PaletteColorWidget( QWidget* parent_)
: QWidget( parent_ )
, _selectionState( -1 )
, _currentPallete( -1 )
, _paletteSize( 4 )
, _invertPaletteColors( false )
, _frameResult( nullptr )
, _radioSequential( nullptr )
, _radioCategorical( nullptr )
//  QRadioButton* _radioCustom; //TODO
, _checkInvertPalette( nullptr )
, _comboPalettes( nullptr )
, _buttonApply( nullptr )
, _buttonCancel( nullptr )
{ }

void PaletteColorWidget::init( bool dialog )
{
  _frameResult = new GradientWidget( );
  _frameResult->setMinimumSize( 200, 50 );
  _frameResult->setMaximumSize( 200, 50 );
  _frameResult->setDirection( GradientWidget::HORIZONTAL );

  _radioSequential = new QRadioButton( "Sequential" );
  _radioCategorical = new QRadioButton( "Categorical" );

  _comboPalettes = new QComboBox( );

  _checkInvertPalette = new QCheckBox( "Invert colors" );

  _buttonApply = new QPushButton( "Apply" );

  if( dialog )
    _buttonCancel = new QPushButton( "Cancel" );

  QGroupBox* groupRadioButtons = new QGroupBox( "Palette type: " );
  QGroupBox* groupPaletteSelection= new QGroupBox( "Pallete: " );

//    QWidget* uppestContainer = new QWidget( );
  QGridLayout* uppestLayout = new QGridLayout( );
//  QGridLayout* uppestLayout = dynamic_cast< QGridLayout* >( layout( ));
//  assert( uppestLayout );
//    QWidget* upperLeftContainer = new QWidget( );
//    QWidget* upperRightContainer = new QWidget( );
  QWidget* confirmationButtonsContainer = new QWidget( );

  // Radio buttons widget
  QHBoxLayout* layoutRadio = new QHBoxLayout( );
  layoutRadio->addWidget( _radioSequential );
  layoutRadio->addWidget( _radioCategorical );

  groupRadioButtons->setLayout( layoutRadio );

  // Palette selection widget
  QGridLayout* paletteLayout = new QGridLayout( );
  paletteLayout->addWidget( _comboPalettes, 0, 0, 1, 1 );
  paletteLayout->addWidget( _checkInvertPalette, 0, 1, 1, 1 );
  paletteLayout->addWidget( _frameResult, 1, 0, 1, ( dialog ? 2 : 1 ) );
  groupPaletteSelection->setLayout( paletteLayout );

  if( dialog )
  {
    // Confirmation buttons widget
    QHBoxLayout* confirmLayout = new QHBoxLayout( );
    confirmLayout->addWidget( _buttonCancel );
    confirmLayout->addWidget( _buttonApply );
    confirmationButtonsContainer->setLayout( confirmLayout );
  }
  else
  {
    paletteLayout->addWidget( _buttonApply, 1, 1, 1, 1 );
  }

  uppestLayout->addWidget( groupRadioButtons );
  uppestLayout->addWidget( groupPaletteSelection );
  uppestLayout->addWidget( confirmationButtonsContainer );

  this->setLayout( uppestLayout );

  connect( _radioSequential, SIGNAL( toggled( bool )),
           this, SLOT( radioButtonClicked( bool )));

//  connect( _radioCategorical, SIGNAL( toggled( bool )),
//           this, SLOT( radioButtonClicked( bool )));

  connect( _comboPalettes, SIGNAL( currentIndexChanged( int )),
           this, SLOT( paletteSelectionChanged( int )));

  connect( _checkInvertPalette, SIGNAL( stateChanged( int )),
           this, SLOT( checkInvertColorsToggled( int )));

  connect( _buttonApply, SIGNAL( clicked( void )),
           this, SLOT( buttonAcceptClicked( void )));

  if( dialog )
  {
    connect( _buttonCancel, SIGNAL( clicked( void )),
             this, SLOT( buttonCancelClicked( void )));
  }

  _radioSequential->setChecked( true );

}

tQColorVec PaletteColorWidget::getColors( void ) const
{
  return _currentColors;
}

const QGradientStops& PaletteColorWidget::getGradientStops( void ) const
{
  return _frameResult->getGradientStops( );
}

void PaletteColorWidget::radioButtonClicked( bool checked )
{
  if( checked )
    _selectionState = ( int ) PALETTE_SEQUENTIAL;
  else
    _selectionState = ( int ) PALETTE_CATEGORICAL;

  _currentPallete = 0;

  _fillPaletteNames( );
  _fillColors( );
}

void PaletteColorWidget::_fillPaletteNames( void )
{
  const std::vector< std::string >* paletteNames = nullptr;

  switch(( tColorType ) _selectionState )
  {
    case PALETTE_SEQUENTIAL:
     paletteNames = &paletteNamesSequential;
     break;
   case PALETTE_CATEGORICAL:
     paletteNames = &paletteNamesCategoric;
     break;
   default:
     return;
  }

  QStringList names;
  for( auto name : *paletteNames )
  {
    QString qname = QString::fromStdString( name );
    names.append( qname );
  }
  std::cout << std::endl;

  _comboPalettes->clear( );
  _comboPalettes->addItems( names );
  _comboPalettes->update( );
}

void PaletteColorWidget::_fillColors( void )
{
  auto colors =
      _selectionState == ( tColorType ) PALETTE_SEQUENTIAL ?
          tscoopp::colorBrewerSequential(( tpSeq ) _currentPallete,
                                         _paletteSize, _invertPaletteColors ) :
          tscoopp::colorBrewerQualitative(( tpCat ) _currentPallete,
                                          _paletteSize, _invertPaletteColors );

  assert( !colors.colors( ).empty( ));

  float step = 1.0 / ( colors.colors( ).size( ) - 1 );
  float acc = 0.0f;

  QGradientStops stops;

  for( auto color : colors.colors( ))
  {
    _currentColors.emplace_back( std::make_pair( acc, color ));

    stops << qMakePair( acc, color );

    acc += step;
  }

  _frameResult->setGradientStops( stops );

}

void PaletteColorWidget::buttonAcceptClicked( void )
{
  emit acceptClicked( );
}

void PaletteColorWidget::buttonCancelClicked( void )
{
  emit cancelClicked( );
}


void PaletteColorWidget::paletteSelectionChanged( int palette )
{
  if( palette < 0 )
    return;

  _currentPallete = palette;

  _fillColors( );
}

void PaletteColorWidget::checkInvertColorsToggled( int checked )
{
  _invertPaletteColors = checked;

  _fillColors( );
}

void PaletteColorWidget::setPlot( QPolygonF plot )
{
  _frameResult->plot( plot );

  std::cout << "Plot: ";
  for( auto bin : plot )
    std::cout << " " << bin.x( ) << " " << bin.y( );
  std::cout << std::endl;

}
