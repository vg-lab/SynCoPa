//
// Created by gaeqs on 4/05/22.
//

#ifndef SYNCOPA_NEURONCLUSTER_H
#define SYNCOPA_NEURONCLUSTER_H

#include <map>
#include <unordered_set>

#include <QString>

namespace syncopa
{

  /**
   * Represents a group of selections, where each selection
   * has a name and a set of neuron GIDs.
   * <p>
   * This class is <strong>immutable</strong>.
   */
  class NeuronCluster
  {

    QString _name;
    std::map< QString , std::unordered_set< unsigned int>> _selections;

  public:

    /**
     * Creates a new neuron cluster.
     * @param name the name of the cluster. This name is visible to the user.
     * @param selections the selections of this cluster.
     */
    NeuronCluster(
      const QString& name ,
      const std::map< QString , std::unordered_set< unsigned int>>& selections );

    /**
     * Returns this cluster's name.
     * @return the name.
     */
    const QString& getName( ) const;

    /**
     * Returns this cluster's selections.
     * @return the selections.
     */
    const std::map< QString , std::unordered_set< unsigned int>>&
    getSelections( ) const;

  };

}


#endif //SYNCOPA_NEURONCLUSTER_H
