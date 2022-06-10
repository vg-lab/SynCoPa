//
// Created by gaeqs on 4/05/22.
//

#ifndef SYNCOPA_NEURONSELECTIONVIEW_H
#define SYNCOPA_NEURONSELECTIONVIEW_H


#include <QWidget>
#include "NeuronMetadata.h"
#include "TripleStateButton.h"

class NeuronSelectionView : public QWidget
{

Q_OBJECT

  QString _name;
  syncopa::NeuronClusterMetadata& _clusterMetadata;
  syncopa::NeuronSelectionMetadata& _metadata;

  QLabel* _label;
  TripleStateButton* _visibilityButton;
  QPushButton* _colorButton;
  TripleStateButton* _morphologyButton;
  TripleStateButton* _synapsesButton;
  TripleStateButton* _pathsButton;
  TripleStateButton* _pathTypesButton;

private slots:

  void colorSelectionClicked( );

  void manageVisibilityChange( TripleStateButton::State visibility );

  void togglePartVisibilities( TripleStateButton::State mode );

  void toggleSynapsesVisibilities( TripleStateButton::State mode );

  void togglePathsVisibilities( TripleStateButton::State mode );

  void togglePathTypes( TripleStateButton::State mode );

public:

  explicit NeuronSelectionView(
    const QString& name ,
    syncopa::NeuronClusterMetadata& clusterMetadata ,
    syncopa::NeuronSelectionMetadata& metadata ,
    QWidget* parent = nullptr );

public slots:

  void manageExternalFocusChange( );

signals:

  void onLocalFocusRefreshRequest( );

  void onRequestBroadcastMetadataModification( );

};


#endif //SYNCOPA_NEURONSELECTIONVIEW_H
