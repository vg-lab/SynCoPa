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

constexpr int MIN_SLIDER = 0;
constexpr int MAX_SLIDER = 99;
constexpr float INV_RANGE = 1.0 / (MAX_SLIDER - MIN_SLIDER);

const QStringList PALETTE_TYPES = { "Sequential", "Categorical", "Diverging", "Uniform" };

PaletteColorWidget::PaletteColorWidget( QWidget* parent_)
: QWidget( parent_ )
, _selectionState( -1 )
, _currentPalette( 0 )
, _paletteSize( 4 )
, _invertPaletteColors( false )
, _frameResult( nullptr )
, _checkInvertPalette( nullptr )
, _checkFilterActive( nullptr )
, _comboPalettes( nullptr )
, _buttonApply( nullptr )
, _buttonCancel( nullptr )
, _labelTotalRange( nullptr )
, _labelActualRange( nullptr )
, _rangeFilterSlider( nullptr )
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

  _comboPalettes = new QComboBox( );

  _checkInvertPalette = new QCheckBox( "Invert colors" );
  _checkFilterActive = new QCheckBox( "Filtering" );

  if( dialog )
  {
    _buttonApply = new QPushButton( "Apply" );
    _buttonCancel = new QPushButton( "Cancel" );
  }

  _rangeFilterSlider = new ctkRangeSlider( Qt::Horizontal );
  _rangeFilterSlider->setValues( MIN_SLIDER, MAX_SLIDER );
  _rangeFilterSlider->setPositions( MIN_SLIDER, MAX_SLIDER );
  _rangeFilterSlider->setEnabled( false );

  _labelTotalRange = new QLabel( "Min:" );
  _labelActualRange = new QLabel( "Max:" );

  QGroupBox* groupPaletteType = new QGroupBox( "Palette type: " );
  QGroupBox* groupPaletteSelection= new QGroupBox( "Palette: " );

  QGridLayout* uppestLayout = new QGridLayout( );
  QWidget* confirmationButtonsContainer = new QWidget( );

  // Palettes type combobox
  auto typeCombo = new QComboBox();
  typeCombo->addItems(PALETTE_TYPES);

  connect(typeCombo, SIGNAL(currentIndexChanged(int)),
          this, SLOT(paletteTypeChanged(int)));

  auto typeLayout = new QHBoxLayout();
  typeLayout->addWidget(typeCombo, 1);

  groupPaletteType->setLayout( typeLayout );

  // Palette selection widget
  QGridLayout* paletteLayout = new QGridLayout( );
  paletteLayout->addWidget( _comboPalettes, 0, 0, 1, 2 );
  paletteLayout->addWidget( _frameResult, 1, 0, 1, 2 );
  groupPaletteSelection->setLayout( paletteLayout );

  if( dialog )
  {
    // Confirmation buttons widget
    QHBoxLayout* confirmLayout = new QHBoxLayout( );
    confirmLayout->addWidget( _buttonCancel );
    confirmLayout->addWidget( _buttonApply );
    confirmationButtonsContainer->setLayout( confirmLayout );
  }

  paletteLayout->addWidget( _rangeFilterSlider, 2, 0, 1, 2 );
  paletteLayout->addWidget( _labelActualRange, 3, 0, 1, 2 );
  paletteLayout->addWidget( _labelTotalRange, 4, 0, 1, 2 );
  paletteLayout->addWidget( _checkFilterActive, 5, 0, 1, 2 );
  paletteLayout->addWidget( _checkInvertPalette, 6, 0, 1, 2 );

  uppestLayout->addWidget( groupPaletteType );
  uppestLayout->addWidget( groupPaletteSelection );
  uppestLayout->addWidget( confirmationButtonsContainer );

  this->setLayout( uppestLayout );

  connect( _comboPalettes, SIGNAL( currentIndexChanged( int )),
           this, SLOT( paletteSelectionChanged( int )));

  connect( _checkInvertPalette, SIGNAL( stateChanged( int )),
           this, SLOT( checkInvertColorsToggled( int )));

  if( dialog )
  {
    connect( _buttonApply, SIGNAL( clicked( void )),
             this, SLOT( buttonAcceptClicked( void )));

    connect( _buttonCancel, SIGNAL( clicked( void )),
             this, SLOT( buttonCancelClicked( void )));
  }

  connect( _checkFilterActive, SIGNAL( toggled( bool )),
           this, SLOT( setFilterActive( bool )));

  connect( _rangeFilterSlider, SIGNAL( maximumPositionChanged( int )),
           this, SLOT( filterSliderChanged()));

  connect( _rangeFilterSlider, SIGNAL( minimumValueChanged( int )),
           this, SLOT( filterSliderChanged()));

  paletteTypeChanged(0);
}

tQColorVec PaletteColorWidget::getColors( void ) const
{
  return _currentColors;
}

const QGradientStops& PaletteColorWidget::getGradientStops( void ) const
{
  return _frameResult->getGradientStops( );
}

void PaletteColorWidget::paletteTypeChanged( int index )
{
  _selectionState = static_cast<tColorType>(index);

  _currentPalette = 0;

  _fillPaletteNames( );
  _fillColors( );
}

void PaletteColorWidget::_fillPaletteNames( void )
{
  const std::vector< std::string >* paletteNames = nullptr;

  switch(static_cast<tColorType>( _selectionState ))
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

  switch( static_cast<tColorType>( _selectionState ))
  {
    case PALETTE_SEQUENTIAL:

      colors = tscoop::colorBrewerSequential(static_cast<tpSeq>( _currentPalette),
                                       _paletteSize, _invertPaletteColors );
      break;

    case PALETTE_CATEGORICAL:

      colors = tscoop::colorBrewerQualitative(static_cast<tpCat>( _currentPalette),
                                      _paletteSize, _invertPaletteColors );
      break;

    case PALETTE_DIVERGING:

      colors = tscoop::colorBrewerDiverging(static_cast<tpDiv>( _currentPalette),
                                      _paletteSize, _invertPaletteColors );
      break;

    case PALETTE_UNIFORM:

      colors = tscoop::matplotlibPerceptualUniform(static_cast<tpUni>(_currentPalette ),
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

  emit filterPaletteChanged();;
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

  const float range = ( _maxRange - _minRange );
  _currentMinValue = _currentLowerLimit * range + _minRange;
  _currentMaxValue = _currentUpperLimit * range + _minRange;

  QString currentRangetext = QString( "Current: [" ) +
      QString::number( _currentMinValue, 'f', 3 ) +
      QString( ", " ) +
      QString::number( _currentMaxValue, 'f', 3 ) + QString( "]");


  _labelTotalRange->setText( rangetext );
  _labelActualRange->setText( currentRangetext );
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
  _filtering = active;

  _rangeFilterSlider->setEnabled( active );

  _frameResult->showLimits( active );
  _frameResult->update( );

  emit filterStateChanged( );
}

void PaletteColorWidget::filterSliderChanged( )
{
  int min = _rangeFilterSlider->minimumPosition();
  int max = _rangeFilterSlider->maximumPosition();

  _currentLowerLimit = std::min( std::max( ( min - MIN_SLIDER ) * INV_RANGE, 0.0f ), 1.0f );

  _currentUpperLimit = std::min( std::max( ( max - MIN_SLIDER ) * INV_RANGE, 0.0f ), 1.0f );

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
