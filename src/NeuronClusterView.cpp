//
// Created by gaeqs on 4/05/22.
//

#include <QHBoxLayout>
#include <QDebug>
#include <QColorDialog>
#include <utility>

#include "NeuronClusterView.h"
#include "NeuronSelectionView.h"

NeuronClusterView::NeuronClusterView(
  std::shared_ptr< syncopa::NeuronClusterManager > manager ,
  const syncopa::NeuronCluster& cluster ,
  syncopa::NeuronClusterMetadata& metadata ,
  QWidget* widget )
  : QWidget( widget )
  , _manager( std::move( manager ))
  , _cluster( cluster )
  , _metadata( metadata )
{
  auto layout = new QVBoxLayout( );
  layout->setMargin( 0 );
  setLayout( layout );

  // Title

  _titleBar = new NeuronClusterViewTitle(
    _manager , cluster , metadata , this , this );

  // Container

  _container = new QWidget( this );
  auto containerLayout = new QVBoxLayout( _container );
  containerLayout->setMargin( 0 );
  _container->setLayout( containerLayout );

  for ( const auto& selection: cluster.getSelections( ))
  {
    auto view = new NeuronSelectionView(
      selection.first , _metadata ,
      _metadata.selection[ selection.first ] , this );

    _selectionViews.push_back( view );
    containerLayout->addWidget( view );

    connect(
      view , SIGNAL( onLocalFocusRefreshRequest( )) ,
      this , SLOT( refreshLocalFocus( ))
    );

    connect(
      view , SIGNAL( onRequestBroadcastMetadataModification( )) ,
      _manager.get( ) , SLOT( broadcastMetadataModification( ))
    );
  }

  layout->addWidget( _titleBar );
  layout->addWidget( _container );

  _container->setHidden( metadata.viewCollapsed );
}

void NeuronClusterView::toggleContainerVisibility( )
{
  _metadata.viewCollapsed = !_metadata.viewCollapsed;
  _container->setHidden( _metadata.viewCollapsed );
}

void NeuronClusterView::refreshLocalFocus( )
{
  for ( const auto& item: _selectionViews )
  {
    item->manageExternalFocusChange( );
  }
}
