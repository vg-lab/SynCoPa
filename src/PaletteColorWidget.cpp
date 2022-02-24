/*
 * @file  PaletteColorWidget.cpp
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

using tscoop = scoop::ColorPalette;
using tpSeq = tscoop::ColorBrewerSequential;
using tpCat = tscoop::ColorBrewerQualitative;
using tpDiv = tscoop::ColorBrewerDiverging;
using tpUni = tscoop::MatplotlibPerceptualUniform;

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

const static std::vector< std::string > paletteNamesDiverging =
{
  "BrBG", "PiYG", "PRGn", "PurOr", "RdBu", "RdGy", "RdYlBu", "RdYlGn", "Spectral"
};

const static std::vector< std::string > paletteNamesUniform =
{
  "Viridis", "Magma", "Inferno", "Plasma"
};

PaletteColorWidget::PaletteColorWidget( QWidget* parent_)
: QWidget( parent_ )
, _selectionState( -1 )
, _currentPalette( 0 )
, _paletteSize( 4 )
, _invertPaletteColors( false )
, _frameResult( nullptr )
, _radioSequential( nullptr )
, _radioCategorical( nullptr )
, _radioDiverging( nullptr )
, _radioUniform( nullptr )
, _checkInvertPalette( nullptr )
, _checkFilterActive( nullptr )
, _comboPalettes( nullptr )
, _buttonApply( nullptr )
, _buttonCancel( nullptr )
, _labelTotalRange( nullptr )
, _labelActualRange( nullptr )
, _rangeFilterSlider( nullptr )
, _minPosSlider( 0 )
, _maxPosSlider( 100 )
, _invPosSlider( 0 )
, _filtering( false )
, _currentLowerLimit( 0.0f )
, _currentUpperLimit( 1.0f )
, _minRange( 0 )
, _maxRange( 1 )
, _currentMinValue( 0 )
, _currentMaxValue( 0 )
{ }

void PaletteColorWidget::init( bool dialog )
{
  _frameResult = new GradientWidget( );
  _frameResult->setMinimumHeight( 50 );
  _frameResult->setMaximumHeight( 50 );
  _frameResult->setDirection( GradientWidget::HORIZONTAL );

  _radioSequential = new QRadioButton( "Sequential" );
  _radioCategorical = new QRadioButton( "Categorical" );
  _radioDiverging = new QRadioButton( "Diverging" );
  _radioUniform = new QRadioButton( "Uniform" );
  _comboPalettes = new QComboBox( );

  _checkInvertPalette = new QCheckBox( "Invert colors" );
  _checkFilterActive = new QCheckBox( "Filtering" );

  _buttonApply = new QPushButton( "Apply" );

  unsigned int colDialog = 1;

  if( dialog )
  {
    _buttonCancel = new QPushButton( "Cancel" );
    colDialog = 2;
  }

  _rangeFilterSlider = new ctkRangeSlider( Qt::Horizontal );
  _rangeFilterSlider->setValues( _minPosSlider, _maxPosSlider );
  _rangeFilterSlider->setPositions( _minPosSlider, _maxPosSlider );
  _rangeFilterSlider->setEnabled( false );
  _invPosSlider = 1.0 / ( _maxPosSlider - _minPosSlider - 1 );

  _labelTotalRange = new QLabel( "Min:" );
  _labelActualRange = new QLabel( "Max:" );

  QGroupBox* groupRadioButtons = new QGroupBox( "Palette type: " );
  QGroupBox* groupPaletteSelection= new QGroupBox( "Palette: " );

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
  layoutRadio->addWidget( _radioDiverging );
  layoutRadio->addWidget( _radioUniform );

  groupRadioButtons->setLayout( layoutRadio );

  // Palette selection widget
  QGridLayout* paletteLayout = new QGridLayout( );
  paletteLayout->addWidget( _comboPalettes, 0, 0, 1, 1 );
  paletteLayout->addWidget( _checkInvertPalette, 0, 1, 1, 1 );
  paletteLayout->addWidget( _frameResult, 1, 0, 1, colDialog );
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

  paletteLayout->addWidget( _rangeFilterSlider, 2, 0, 1, colDialog );
  paletteLayout->addWidget( _checkFilterActive, 2, colDialog, 1, 1 );

  paletteLayout->addWidget( _labelActualRange, 3, 0, 1, 1 );
  paletteLayout->addWidget( _labelTotalRange, 3, 1, 1, 1 );

  uppestLayout->addWidget( groupRadioButtons );
  uppestLayout->addWidget( groupPaletteSelection );
  uppestLayout->addWidget( confirmationButtonsContainer );

  this->setLayout( uppestLayout );

  connect( _radioSequential, SIGNAL( toggled( bool )),
           this, SLOT( radioButtonClicked( bool )));

  connect( _radioCategorical, SIGNAL( toggled( bool )),
           this, SLOT( radioButtonClicked( bool )));

  connect( _radioDiverging, SIGNAL( toggled( bool )),
           this, SLOT( radioButtonClicked( bool )));

  connect( _radioUniform, SIGNAL( toggled( bool )),
           this, SLOT( radioButtonClicked( bool )));


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

  connect( _checkFilterActive, SIGNAL( toggled( bool )),
           this, SLOT( setFilterActive( bool )));

  connect( _rangeFilterSlider, SIGNAL( positionsChanged( int, int )),
           this, SLOT( filterSliderChanged( int, int )));

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

void PaletteColorWidget::radioButtonClicked( bool )
{
//  if( checked )
//    _selectionState = ( int ) PALETTE_SEQUENTIAL;
//  else
//    _selectionState = ( int ) PALETTE_CATEGORICAL;

  auto src = sender( );

  if( src == _radioSequential )
    _selectionState = ( int ) PALETTE_SEQUENTIAL;
  if( src == _radioCategorical )
      _selectionState = ( int ) PALETTE_CATEGORICAL;
  if( src == _radioDiverging )
      _selectionState = ( int ) PALETTE_DIVERGING;
  if( src == _radioUniform )
      _selectionState = ( int ) PALETTE_UNIFORM;


  _currentPalette = 0;

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
   case PALETTE_DIVERGING:
     paletteNames = &paletteNamesDiverging;
     break;
   case PALETTE_UNIFORM:
     paletteNames = &paletteNamesUniform;
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
  scoop::ColorPalette colors;

  switch( ( tColorType ) _selectionState )
  {
    case PALETTE_SEQUENTIAL:

      colors = tscoop::colorBrewerSequential(( tpSeq ) _currentPalette,
                                       _paletteSize, _invertPaletteColors );
      break;

    case PALETTE_CATEGORICAL:

      colors = tscoop::colorBrewerQualitative(( tpCat ) _currentPalette,
                                      _paletteSize, _invertPaletteColors );
      break;

    case PALETTE_DIVERGING:

      colors = tscoop::colorBrewerDiverging(( tpDiv ) _currentPalette,
                                      _paletteSize, _invertPaletteColors );
      break;

    case PALETTE_UNIFORM:

      colors = tscoop::matplotlibPerceptualUniform(( tpUni ) _currentPalette ,
                                                   _invertPaletteColors );
      break;

  }

  assert( !colors.colors( ).empty( ));
  float step = 1.0f / ( static_cast<float>(colors.colors( ).size( ) - 1 ));
  float acc = 0.0f;

  QGradientStops stops;

  _currentColors.clear( );
  for ( const auto& color: colors.colors( ))
  {
    _currentColors.emplace_back( std::make_pair( acc , color ));
    stops << qMakePair( acc , color );
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

  _currentPalette = palette;

  _fillColors( );
}

void PaletteColorWidget::checkInvertColorsToggled( int checked )
{
  _invertPaletteColors = checked;

  _fillColors( );
}

void PaletteColorWidget::setPlot( QPolygonF plot, float minRange, float maxRange )
{
  _frameResult->plot( plot );

  _minRange = minRange;
  _maxRange = maxRange;

  QString rangetext = QString( "Total: [" ) +
      QString::number( minRange, 'f', 3 ) +
      QString( ", " ) +
      QString::number( maxRange, 'f', 3 ) + QString( "]");

  float range = ( _maxRange - _minRange );
  _currentMinValue = _currentLowerLimit * range + _minRange;
  _currentMaxValue = _currentUpperLimit * range + _minRange;

  QString currentRangetext = QString( "Current: [" ) +
      QString::number( _currentMinValue, 'f', 3 ) +
      QString( ", " ) +
      QString::number( _currentMaxValue, 'f', 3 ) + QString( "]");


  _labelTotalRange->setText( rangetext );
  _labelActualRange->setText( currentRangetext );

//  std::cout << "Plot: ";
//  for( auto bin : plot )
//    std::cout << " " << bin.x( ) << " " << bin.y( );
//  std::cout << std::endl;

}

bool PaletteColorWidget::filter( void ) const
{
  return _filtering;
}

std::pair< float, float > PaletteColorWidget::filterBounds( void ) const
{
  return std::make_pair( _currentLowerLimit, _currentUpperLimit );
}

void PaletteColorWidget::setFilterActive( bool active )
{
  std::cout << "Filtering " << std::boolalpha << active << std::endl;

  _filtering = active;

  _rangeFilterSlider->setEnabled( active );

  _frameResult->showLimits( active );
  _frameResult->update( );

  emit filterStateChanged( );
}


void PaletteColorWidget::filterSliderChanged( int min, int max )
{

  _currentLowerLimit = std::min( std::max( ( min - _minPosSlider ) * _invPosSlider, 0.0f ), 1.0f );

  _currentUpperLimit = std::min( std::max(( max - _minPosSlider ) * _invPosSlider, 0.0f ), 1.0f );

  std::cout << "Slider min " << min << " " << _currentLowerLimit
            << " max " << max << " " << _currentUpperLimit << std::endl;

  _frameResult->limits( _currentLowerLimit, _currentUpperLimit );
  _frameResult->update( );

  float range = ( _maxRange - _minRange );
  _currentMinValue = _currentLowerLimit * range + _minRange;
  _currentMaxValue = _currentUpperLimit * range + _minRange;

  QString currentRangetext = QString( "Current: [" ) +
      QString::number( _currentMinValue, 'f', 3 ) +
      QString( ", " ) +
      QString::number( _currentMaxValue, 'f', 3 ) + QString( "]");

  _labelActualRange->setText( currentRangetext );

  emit filterBoundsChanged( );
}
