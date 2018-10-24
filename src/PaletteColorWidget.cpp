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
 "Accent", " Dark2", " Paired", " Pastel1",
 "Pastel2", " Set1", " Set2", " Set3"
};

PaletteColorWidget::PaletteColorWidget( QWidget* parent_)
: QWidget( parent_ )
, _selectionState( -1 )
, _currentPallete( -1 )
, _paletteSize( 4 )
, _frameResult( nullptr )
, _radioSequential( nullptr )
, _radioCategorical( nullptr )
//  QRadioButton* _radioCustom; //TODO
, _comboPalettes( nullptr )
, _buttonAccept( nullptr )
, _buttonCancel( nullptr )
{ }

void PaletteColorWidget::init( void )
{
  _frameResult = new GradientWidget( );
  _frameResult->setMinimumSize( 200, 50 );
  _frameResult->setMaximumSize( 200, 50 );
  _frameResult->setDirection( GradientWidget::HORIZONTAL );

  _radioSequential = new QRadioButton( "Sequential" );
  _radioCategorical = new QRadioButton( "Categorical" );

  _comboPalettes = new QComboBox( );

  _buttonAccept = new QPushButton( "Accept" );
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
  QVBoxLayout* layoutRadio = new QVBoxLayout( );
  layoutRadio->addWidget( _radioSequential );
  layoutRadio->addWidget( _radioCategorical );

  groupRadioButtons->setLayout( layoutRadio );

  // Palette selection widget
  QVBoxLayout* paletteLayout = new QVBoxLayout( );
  paletteLayout->addWidget( _comboPalettes );
  paletteLayout->addWidget( _frameResult );
  groupPaletteSelection->setLayout( paletteLayout );

  // Confirmation buttons widget
  QHBoxLayout* confirmLayout = new QHBoxLayout( );
  confirmLayout->addWidget( _buttonCancel );
  confirmLayout->addWidget( _buttonAccept );
  confirmationButtonsContainer->setLayout( confirmLayout );

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

  connect( _buttonAccept, SIGNAL( clicked( void )),
           this, SLOT( buttonAcceptClicked( void )));

  connect( _buttonCancel, SIGNAL( clicked( void )),
           this, SLOT( buttonCancelClicked( void )));

_radioSequential->setChecked( true );
}

void PaletteColorWidget::loadScoopPalletes( unsigned int )
{
//    unsigned int maxSequential = (( unsigned int ) scoop::ColorPalette::Reds ) + 1;
//    unsigned int maxCategorical = (( unsigned int ) scoop::ColorPalette::Set3 ) + 1;
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
          tscoopp::colorBrewerSequential(( tpSeq ) _currentPallete, _paletteSize ) :
          tscoopp::colorBrewerQualitative(( tpCat ) _currentPallete, _paletteSize );

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

