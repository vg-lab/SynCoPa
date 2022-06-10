//
// Created by gaeqs on 26/05/22.
//

#include "NeuronMetadata.h"

namespace syncopa
{

  const std::map< QString , NeuronMetadataShowPart > NEURON_METADATA_SHOW_PARTS = {
    { "all" ,             NeuronMetadataShowPart::ALL } ,
    { "soma_only" ,       NeuronMetadataShowPart::SOMA_ONLY } ,
    { "morphology_only" , NeuronMetadataShowPart::MORPHOLOGY_ONLY }
  };

  const std::map< QString , SynapsesVisibility > NEURON_METADATA_SYNAPSES_VISIBILITY = {
    { "hidden" ,         SynapsesVisibility::HIDDEN } ,
    { "connected_only" , SynapsesVisibility::CONNECTED_ONLY } ,
    { "all" ,            SynapsesVisibility::ALL }
  };

  const std::map< QString , PathsVisibility > NEURON_METADATA_PATHS_VISIBILITY = {
    { "hidden" ,         PathsVisibility::HIDDEN } ,
    { "connected_only" , PathsVisibility::CONNECTED_ONLY } ,
    { "all" ,            PathsVisibility::ALL }
  };

  const std::map< QString , PathTypes > NEURON_METADATA_PATH_TYPES = {
    { "pre_only" ,  PathTypes::PRE_ONLY } ,
    { "post_only" , PathTypes::POST_ONLY } ,
    { "all" ,       PathTypes::ALL }
  };

}
