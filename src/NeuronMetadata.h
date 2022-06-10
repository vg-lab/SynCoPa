//
// Created by gaeqs on 4/05/22.
//

#ifndef SYNCOPA_NEURONMETADATA_H
#define SYNCOPA_NEURONMETADATA_H


#include <QColor>
#include <utility>
#include <map>

namespace syncopa
{

  enum class NeuronMetadataShowPart
  {
    ALL = 0 ,
    SOMA_ONLY ,
    MORPHOLOGY_ONLY
  };

  enum class SynapsesVisibility
  {
    HIDDEN = 0 ,
    CONNECTED_ONLY ,
    ALL
  };

  enum class PathsVisibility
  {
    HIDDEN = 0 ,
    CONNECTED_ONLY ,
    ALL
  };

  enum class PathTypes
  {
    PRE_ONLY = 0 ,
    POST_ONLY ,
    ALL
  };

  extern const std::map< QString , NeuronMetadataShowPart > NEURON_METADATA_SHOW_PARTS;
  extern const std::map< QString , SynapsesVisibility > NEURON_METADATA_SYNAPSES_VISIBILITY;
  extern const std::map< QString , PathsVisibility > NEURON_METADATA_PATHS_VISIBILITY;
  extern const std::map< QString , PathTypes > NEURON_METADATA_PATH_TYPES;

  /**
   * Represents the mutable information of a cluster's selection.
   */
  struct NeuronSelectionMetadata
  {
    bool enabled;
    NeuronMetadataShowPart partsToShow;
    SynapsesVisibility synapsesVisibility;
    PathsVisibility pathsVisibility;
    PathTypes pathTypes;
    QColor color;

    NeuronSelectionMetadata( )
      : enabled( true )
      , partsToShow( NeuronMetadataShowPart::SOMA_ONLY )
      , synapsesVisibility( SynapsesVisibility::CONNECTED_ONLY )
      , pathsVisibility( PathsVisibility::HIDDEN )
      , pathTypes( PathTypes::ALL )
      , color( 255 , 255 , 255 )
    { }

    NeuronSelectionMetadata( bool enabled_ ,
                             NeuronMetadataShowPart partsToShow_ ,
                             SynapsesVisibility synapsesVisibility_ ,
                             PathsVisibility pathsVisibility_ ,
                             PathTypes pathTypes_ ,
                             QColor color_ )
      : enabled( enabled_ )
      , partsToShow( partsToShow_ )
      , synapsesVisibility( synapsesVisibility_ )
      , pathsVisibility( pathsVisibility_ )
      , pathTypes( pathTypes_ )
      , color( std::move( color_ ))
    { }
  };

  /**
   * Represents a cluster's mutable information.
   */
  struct NeuronClusterMetadata
  {

    bool enabled = true;
    bool closeable = true;
    QColor color = QColor( 127 , 127 , 0 );
    std::map< QString , NeuronSelectionMetadata > selection;

    bool viewCollapsed = false;
    QString focusedSelection = "";
  };


}


#endif //SYNCOPA_NEURONMETADATA_H
