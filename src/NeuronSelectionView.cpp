//
// Created by gaeqs on 4/05/22.
//

#include <QHBoxLayout>
#include <QIcon>
#include <QPushButton>
#include <QLabel>
#include <QColorDialog>
#include <QDebug>

#include "NeuronSelectionView.h"

void NeuronSelectionView::colorSelectionClicked( )
{

  const QColor result = QColorDialog::getColor(
    _metadata.color , this , tr( "Select a new cluster color" ) ,
    QColorDialog::DontUseNativeDialog );

  if ( result.isValid( ))
  {
    _metadata.color = result;
    auto titlePalette = QPalette( palette( ));
    titlePalette.setColor( QPalette::Background , result );
    titlePalette.setColor( QPalette::Button , result );
    _colorButton->setPalette( titlePalette );
    emit onRequestBroadcastMetadataModification( );
  }
}

void NeuronSelectionView::manageVisibilityChange(
  TripleStateButton::State visibility )
{
  switch ( visibility )
  {
    case TripleStateButton::Off:
      _metadata.enabled = false;

      if ( _clusterMetadata.focusedSelection == _name )
      {
        _clusterMetadata.focusedSelection = "";
        emit onLocalFocusRefreshRequest( );
      }
      break;
    case TripleStateButton::On:
      _metadata.enabled = true;

      if ( _clusterMetadata.focusedSelection == _name )
      {
        _clusterMetadata.focusedSelection = "";
        emit onLocalFocusRefreshRequest( );
      }
      break;
    case TripleStateButton::Focus:
      _metadata.enabled = true;
      _clusterMetadata.focusedSelection = _name;
      emit onLocalFocusRefreshRequest( );
      break;
  }
  emit onRequestBroadcastMetadataModification( );
}

void NeuronSelectionView::togglePartVisibilities(
  TripleStateButton::State mode )
{
  switch ( mode )
  {
    case TripleStateButton::Focus:
      _metadata.partsToShow = syncopa::NeuronMetadataShowPart::ALL;
      break;
    case TripleStateButton::On:
      _metadata.partsToShow = syncopa::NeuronMetadataShowPart::SOMA_ONLY;
      break;
    case TripleStateButton::Off:
      _metadata.partsToShow = syncopa::NeuronMetadataShowPart::MORPHOLOGY_ONLY;
      break;
  }
  emit onRequestBroadcastMetadataModification( );
}

void NeuronSelectionView::toggleSynapsesVisibilities(
  TripleStateButton::State mode )
{
  switch ( mode )
  {
    case TripleStateButton::Focus:
      _metadata.synapsesVisibility = syncopa::SynapsesVisibility::ALL;
      break;
    case TripleStateButton::On:
      _metadata.synapsesVisibility = syncopa::SynapsesVisibility::CONNECTED_ONLY;
      break;
    case TripleStateButton::Off:
      _metadata.synapsesVisibility = syncopa::SynapsesVisibility::HIDDEN;
      break;
  }
  emit onRequestBroadcastMetadataModification( );
}

void NeuronSelectionView::togglePathsVisibilities(
  TripleStateButton::State mode )
{
  switch ( mode )
  {
    case TripleStateButton::Focus:
      _metadata.pathsVisibility = syncopa::PathsVisibility::ALL;
      break;
    case TripleStateButton::On:
      _metadata.pathsVisibility = syncopa::PathsVisibility::CONNECTED_ONLY;
      break;
    case TripleStateButton::Off:
      _metadata.pathsVisibility = syncopa::PathsVisibility::HIDDEN;
      break;
  }
  emit onRequestBroadcastMetadataModification( );
}

void NeuronSelectionView::togglePathTypes(
  TripleStateButton::State mode )
{
  switch ( mode )
  {
    case TripleStateButton::Focus:
      _metadata.pathTypes = syncopa::PathTypes::ALL;
      break;
    case TripleStateButton::On:
      _metadata.pathTypes = syncopa::PathTypes::POST_ONLY;
      break;
    case TripleStateButton::Off:
      _metadata.pathTypes = syncopa::PathTypes::PRE_ONLY;
      break;
  }
  emit onRequestBroadcastMetadataModification( );
}


NeuronSelectionView::NeuronSelectionView(
  const QString& name ,
  syncopa::NeuronClusterMetadata& clusterMetadata ,
  syncopa::NeuronSelectionMetadata& metadata ,
  QWidget* parent )
  : QWidget( parent )
  , _name( name )
  , _clusterMetadata( clusterMetadata )
  , _metadata( metadata )
{
  auto layout = new QHBoxLayout( this );
  layout->setAlignment( Qt::AlignLeft );
  setLayout( layout );

  _label = new QLabel( name );

  if ( _clusterMetadata.focusedSelection.isEmpty( ))
  {
    _visibilityButton = new TripleStateButton( metadata.enabled
                                               ? TripleStateButton::On
                                               : TripleStateButton::Off ,
                                               this );

  }
  else if ( _clusterMetadata.focusedSelection == name )
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


  _colorButton = new QPushButton( this );
  _colorButton->setFixedSize( 20 , 20 );
  auto colorButtonPalette = QPalette( palette( ));
  colorButtonPalette.setColor( QPalette::Button , metadata.color );
  _colorButton->setPalette( colorButtonPalette );


  auto morphButtonState = TripleStateButton::Focus;
  switch ( _metadata.partsToShow )
  {
    case syncopa::NeuronMetadataShowPart::ALL:
      morphButtonState = TripleStateButton::Focus;
      break;
    case syncopa::NeuronMetadataShowPart::SOMA_ONLY:
      morphButtonState = TripleStateButton::On;
      break;
    case syncopa::NeuronMetadataShowPart::MORPHOLOGY_ONLY:
      morphButtonState = TripleStateButton::Off;
      break;
  }

  auto synapsesVisibilityState = TripleStateButton::On;
  switch ( _metadata.synapsesVisibility )
  {
    case syncopa::SynapsesVisibility::ALL:
      synapsesVisibilityState = TripleStateButton::Focus;
      break;
    case syncopa::SynapsesVisibility::CONNECTED_ONLY:
      synapsesVisibilityState = TripleStateButton::On;
      break;
    case syncopa::SynapsesVisibility::HIDDEN:
      synapsesVisibilityState = TripleStateButton::Off;
      break;
  }

  auto pathsVisibilityState = TripleStateButton::On;
  switch ( _metadata.pathsVisibility )
  {
    case syncopa::PathsVisibility::ALL:
      pathsVisibilityState = TripleStateButton::Focus;
      break;
    case syncopa::PathsVisibility::CONNECTED_ONLY:
      pathsVisibilityState = TripleStateButton::On;
      break;
    case syncopa::PathsVisibility::HIDDEN:
      pathsVisibilityState = TripleStateButton::Off;
      break;
  }

  auto pathTypesState = TripleStateButton::On;
  switch ( _metadata.pathTypes )
  {
    case syncopa::PathTypes::ALL:
      pathTypesState = TripleStateButton::Focus;
      break;
    case syncopa::PathTypes::POST_ONLY:
      pathTypesState = TripleStateButton::On;
      break;
    case syncopa::PathTypes::PRE_ONLY:
      pathTypesState = TripleStateButton::Off;
      break;
  }

  _morphologyButton = new TripleStateButton(
    morphButtonState ,
    this ,
    ":/icons/neuron_paths.png" ,
    ":/icons/neuron_soma.png" ,
    ":/icons/neuron_all.png",
    "Showing neuron paths only",
    "Showing neuron somas only",
    "Showing full neuron morphologies"
  );

  _synapsesButton = new TripleStateButton(
    synapsesVisibilityState ,
    this,
    ":/icons/synapses_hidden.svg" ,
    ":/icons/synapses_connected.svg" ,
    ":/icons/synapses_all.svg",
    "Hiding synapses",
    "Showing connected synapses",
    "Showing all synapses"
  );

  _pathsButton = new TripleStateButton(
    pathsVisibilityState ,
    this,
    ":/icons/paths_hidden.svg" ,
    ":/icons/paths_connected.svg" ,
    ":/icons/paths_all.svg",
    "Hiding paths",
    "Showing connected paths",
    "Showing all paths"
  );

  _pathTypesButton = new TripleStateButton(
    pathTypesState ,
    this,
    ":/icons/paths_pre.svg" ,
    ":/icons/paths_post.svg" ,
    ":/icons/paths_pre_post.svg",
    "Showing pre-synaptic path sections",
    "Showing post-synaptic path sections",
    "Showing pre-synaptic and post-synaptic path sections"
  );

  layout->addWidget( _visibilityButton );
  layout->addWidget( _colorButton );
  layout->addWidget( _morphologyButton );
  layout->addWidget( _synapsesButton );
  layout->addWidget( _pathsButton );
  layout->addWidget( _pathTypesButton );
  layout->addWidget( _label );

  connect(
    _colorButton , SIGNAL( clicked( bool )) ,
    this , SLOT( colorSelectionClicked( ))
  );

  connect(
    _morphologyButton , SIGNAL( onStateChange( TripleStateButton::State )) ,
    this , SLOT( togglePartVisibilities( TripleStateButton::State ))
  );

  connect(
    _synapsesButton , SIGNAL( onStateChange( TripleStateButton::State )) ,
    this , SLOT( toggleSynapsesVisibilities( TripleStateButton::State ))
  );

  connect(
    _pathsButton , SIGNAL( onStateChange( TripleStateButton::State )) ,
    this , SLOT( togglePathsVisibilities( TripleStateButton::State ))
  );

  connect(
    _pathTypesButton , SIGNAL( onStateChange( TripleStateButton::State )) ,
    this , SLOT( togglePathTypes( TripleStateButton::State ))
  );

  connect(
    _visibilityButton ,
    SIGNAL( onStateChange( TripleStateButton::State )) ,
    this , SLOT( manageVisibilityChange( TripleStateButton::State ))
  );

}

void NeuronSelectionView::manageExternalFocusChange( )
{
  if ( _clusterMetadata.focusedSelection.isEmpty( ))
  {
    _label->setEnabled( true );
  }
  else
  {
    bool enabled = _clusterMetadata.focusedSelection == _name;
    _label->setEnabled( enabled );
    if ( !enabled &&
         _visibilityButton->getState( ) == TripleStateButton::Focus )
    {
      _visibilityButton->setState( TripleStateButton::On );
    }
  }
}
