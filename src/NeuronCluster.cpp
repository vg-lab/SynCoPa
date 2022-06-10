//
// Created by gaeqs on 4/05/22.
//

#include "NeuronCluster.h"

namespace syncopa
{

  NeuronCluster::NeuronCluster(
    const QString& name ,
    const std::map< QString , std::unordered_set< unsigned int>>& selections )
    : _name( name )
    , _selections( selections )
  { }

  const QString& NeuronCluster::getName( ) const
  {
    return _name;
  }

  const std::map< QString , std::unordered_set< unsigned int>>&
  NeuronCluster::getSelections( ) const
  {
    return _selections;
  }

}
