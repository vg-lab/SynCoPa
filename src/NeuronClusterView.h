//
// Created by gaeqs on 4/05/22.
//

#ifndef SYNCOPA_NEURONCLUSTERVIEW_H
#define SYNCOPA_NEURONCLUSTERVIEW_H


#include <QWidget>
#include <QLabel>
#include <memory>

#include "NeuronCluster.h"
#include "NeuronMetadata.h"
#include "TripleStateButton.h"
#include "NeuronClusterManager.h"
#include "NeuronSelectionView.h"
#include "NeuronClusterViewTitle.h"

class QFrame;

class NeuronClusterView : public QWidget
{

Q_OBJECT

  static constexpr double DARK_THEME_THRESHOLD = 0.3;

  std::shared_ptr< syncopa::NeuronClusterManager > _manager;
  const syncopa::NeuronCluster& _cluster;
  syncopa::NeuronClusterMetadata& _metadata;

  NeuronClusterViewTitle* _titleBar;
  QWidget* _container;

  std::vector< NeuronSelectionView* > _selectionViews;

public:

  NeuronClusterView(
    std::shared_ptr< syncopa::NeuronClusterManager > manager ,
    const syncopa::NeuronCluster& cluster ,
    syncopa::NeuronClusterMetadata& metadata ,
    QWidget* parent = nullptr );


public slots:

  void toggleContainerVisibility( );

  void refreshLocalFocus( );

};


#endif //SYNCOPA_NEURONCLUSTERVIEW_H
