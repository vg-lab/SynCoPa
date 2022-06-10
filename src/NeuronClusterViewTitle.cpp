//
// Created by gaeqs on 5/05/22.
//

#include "NeuronClusterViewTitle.h"
#include "TripleStateButton.h"

#include <utility>
#include <QHBoxLayout>
#include <QPushButton>
#include <QColorDialog>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDebug>
#include <QDrag>

#include "NeuronClusterView.h"

void NeuronClusterViewTitle::colorSelectionClicked( )
{

  const QColor result = QColorDialog::getColor(
    _metadata.color , this , tr( "Select a new cluster color" ) ,
    QColorDialog::DontUseNativeDialog );

  if ( result.isValid( ))
  {
    _metadata.color = result;
    auto titlePalette = QPalette( palette( ));
    titlePalette.setColor( QPalette::Background , result );
    titlePalette.setColor( QPalette::Button , result.darker( ));
    if ( result.darker( ).lightnessF( ) < DARK_THEME_THRESHOLD )
    {
      titlePalette.setColor( QPalette::ButtonText , Qt::white );
    }
    else
    {
      titlePalette.setColor( QPalette::ButtonText , Qt::black );
    }

    if ( result.lightnessF( ) < DARK_THEME_THRESHOLD )
    {
      titlePalette.setColor( QPalette::WindowText , Qt::white );
    }
    else
    {
      titlePalette.setColor( QPalette::WindowText , Qt::black );
    }

    setPalette( titlePalette );
  }
}

void NeuronClusterViewTitle::closeClicked( )
{
  _manager->removeCluster( _cluster.getName( ));
}

void NeuronClusterViewTitle::manageVisibilityChange(
  TripleStateButton::State visibility )
{
  switch ( visibility )
  {
    case TripleStateButton::Off:
      _metadata.enabled = false;
      _manager->removeFocusIfFocused( _cluster.getName( ));
      break;
    case TripleStateButton::On:
      _metadata.enabled = true;
      _manager->removeFocusIfFocused( _cluster.getName( ));
      break;
    case TripleStateButton::Focus:
      _metadata.enabled = true;
      _manager->focus( _cluster.getName( ));
      break;
  }
  _manager->broadcastMetadataModification( );
}


void NeuronClusterViewTitle::managerExternalFocusChange(
  syncopa::NeuronCluster* cluster )
{
  if ( cluster == nullptr )
  {
    _label->setEnabled( true );
  }
  else
  {
    bool enabled = cluster->getName( ) == _cluster.getName( );
    _label->setEnabled( enabled );
    if ( !enabled &&
         _visibilityButton->getState( ) == TripleStateButton::Focus )
    {
      _visibilityButton->setState( TripleStateButton::On );
    }
  }
}

NeuronClusterViewTitle::NeuronClusterViewTitle(
  std::shared_ptr< syncopa::NeuronClusterManager > manager ,
  const syncopa::NeuronCluster& cluster ,
  syncopa::NeuronClusterMetadata& metadata ,
  NeuronClusterView* title ,
  QWidget* parent )
  : QFrame( parent )
  , _manager( std::move( manager ))
  , _cluster( cluster )
  , _metadata( metadata )
{
  setAcceptDrops( true );

  auto titleLayout = new QHBoxLayout( this );
  setLayout( titleLayout );

  setFrameShadow( QFrame::Raised );
  setFrameShape( QFrame::StyledPanel );

  auto titlePalette = QPalette( palette( ));
  titlePalette.setColor( QPalette::Background , metadata.color );
  titlePalette.setColor( QPalette::Button , metadata.color.darker( ));
  if ( metadata.color.darker( ).lightnessF( ) < DARK_THEME_THRESHOLD )
  {
    titlePalette.setColor( QPalette::ButtonText , Qt::white );
  }
  if ( metadata.color.lightnessF( ) < DARK_THEME_THRESHOLD )
  {
    titlePalette.setColor( QPalette::WindowText , Qt::white );
  }

  _label = new QLabel( cluster.getName( ) , this );

  auto focused = _manager->getFocused( );
  if ( focused.isEmpty( ))
  {
    _visibilityButton = new TripleStateButton( metadata.enabled
                                               ? TripleStateButton::On
                                               : TripleStateButton::Off ,
                                               this );
  }
  else if ( focused == cluster.getName( ))
  {
    _visibilityButton = new TripleStateButton( TripleStateButton::Focus ,
                                               this );
  }
  else
  {
    _visibilityButton = new TripleStateButton( metadata.enabled
                                               ? TripleStateButton::On
                                               : TripleStateButton::Off ,
                                               this );
    _label->setEnabled( false );
  }


  auto colorButton = new QPushButton( this );
  auto collapseButton = new QPushButton( "-" , this );

  colorButton->setFixedSize( 20 , 20 );
  collapseButton->setFixedSize( 20 , 20 );

  titleLayout->addWidget( _label );
  titleLayout->addStretch( );
  titleLayout->addWidget( _visibilityButton );
  titleLayout->addWidget( colorButton );
  titleLayout->addWidget( collapseButton );

  if ( metadata.closeable )
  {
    auto closeButton = new QPushButton( "x" , this );
    closeButton->setFixedSize( 20 , 20 );
    titleLayout->addWidget( closeButton );

    connect(
      closeButton , SIGNAL( clicked( )) ,
      this , SLOT( closeClicked( ))
    );
  }

  setAutoFillBackground( true );
  setPalette( titlePalette );

  connect(
    _visibilityButton ,
    SIGNAL( onStateChange( TripleStateButton::State )) ,
    this , SLOT( manageVisibilityChange( TripleStateButton::State ))
  );

  connect(
    colorButton , SIGNAL( clicked( )) ,
    this , SLOT( colorSelectionClicked( ))
  );

  connect(
    collapseButton , SIGNAL( clicked( )) ,
    title , SLOT( toggleContainerVisibility( ))
  );

  connect(
    _manager.get( ) , SIGNAL( onFocus( syncopa::NeuronCluster * )) ,
    this , SLOT( managerExternalFocusChange( syncopa::NeuronCluster * ))
  );
}

void NeuronClusterViewTitle::mouseMoveEvent( QMouseEvent* event )
{
  if ( event->buttons( ).testFlag( Qt::LeftButton ))
  {
    auto* drag = new QDrag( this );
    auto* mimeData = new QMimeData;
    mimeData->setText( "neuron_cluster:" + _cluster.getName( ));
    drag->setMimeData( mimeData );
    drag->setPixmap( QPixmap( ":/icons/neurolots" ));
    drag->exec( );
  }
}

void NeuronClusterViewTitle::dragEnterEvent( QDragEnterEvent* event )
{
  if ( event->mimeData( )->hasFormat( "text/plain" ))
  {
    QString text = event->mimeData( )->text( );
    if ( text.startsWith( "neuron_cluster:" ))
    {
      event->acceptProposedAction( );
    }
  }
}

void NeuronClusterViewTitle::dropEvent( QDropEvent* event )
{
  QString text = event->mimeData( )->text( );
  QString name = text.right( text.length( ) - 15 );
  _manager->moveBeforeElement( name , _cluster.getName( ));
}
