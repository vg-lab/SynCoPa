//
// Created by gaeqs on 5/05/22.
//

#ifndef SYNCOPA_NEURONCLUSTERVIEWTITLE_H
#define SYNCOPA_NEURONCLUSTERVIEWTITLE_H


#include <memory>

#include <QFrame>
#include <QLabel>

#include "NeuronClusterManager.h"
#include "NeuronCluster.h"
#include "NeuronMetadata.h"
#include "TripleStateButton.h"

class NeuronClusterView;

class NeuronClusterViewTitle : public QFrame
{

  static constexpr double DARK_THEME_THRESHOLD = 0.3;

Q_OBJECT

  std::shared_ptr< syncopa::NeuronClusterManager > _manager;
  const syncopa::NeuronCluster& _cluster;
  syncopa::NeuronClusterMetadata& _metadata;

  QLabel* _label;
  TripleStateButton* _visibilityButton;

private slots:

  void colorSelectionClicked( );

  void closeClicked( );

  void manageVisibilityChange( TripleStateButton::State visibility );

  void managerExternalFocusChange( syncopa::NeuronCluster* cluster );

public:

  NeuronClusterViewTitle(
    std::shared_ptr< syncopa::NeuronClusterManager > manager ,
    const syncopa::NeuronCluster& cluster ,
    syncopa::NeuronClusterMetadata& metadata ,
    NeuronClusterView* view ,
    QWidget* parent = nullptr );

protected:

  void mouseMoveEvent( QMouseEvent* event ) override;

  void dragEnterEvent( QDragEnterEvent* event ) override;

  void dropEvent( QDropEvent* event ) override;

};


#endif //SYNCOPA_NEURONCLUSTERVIEWTITLE_H
