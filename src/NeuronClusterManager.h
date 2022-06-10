//
// Created by gaeqs on 4/05/22.
//

#ifndef SYNCOPA_NEURONCLUSTERMANAGER_H
#define SYNCOPA_NEURONCLUSTERMANAGER_H

#include <vector>
#include <QObject>

#include "NeuronCluster.h"
#include "NeuronMetadata.h"

namespace syncopa
{

  /**
   * This class manages the NeuronClusters of the scene.
   */
  class NeuronClusterManager : public QObject
  {

  Q_OBJECT

    std::vector< NeuronCluster > _clusters;
    std::map< QString , NeuronClusterMetadata > _metadata;
    QString _focused;

    std::vector< NeuronCluster >::iterator
    findCluster( const QString& name );

  public:

    /**
     * Creates the manager.
     */
    NeuronClusterManager( );

    /**
     * Returns a constant view of all clusters of the scene.
     *
     * @return the vector.
     */
    const std::vector< NeuronCluster >& getClusters( );

    /**
     * Returns the map with the metadata of all loaded clusters.
     *
     * You can modify contents of each metadata from this map, and it
     * is possible to modify the contents of the map itself.
     * Once you're done modifying the contents, invoke a
     * <i>onMetadataModification()</i> event using the
     * <i>broadcastMetadataModification()</i> method.
     *
     * It is not recommended to add new metadata elements from this
     * map: metadata instances are created when a new cluster is added
     * to the manager automatically.
     *
     * @return the map.
     */
    std::map< QString , NeuronClusterMetadata >& getMetadata( );

    /**
     * Returns the name of the focused cluster.
     *
     * @return the name of the focused cluster.
     */
    const QString& getFocused( ) const;

    /**
     * Returns the cluster that matches the given name.
     * @param name the name.
     * @return the cluster.
     * @throw std::out_of_range whether a cluster that matches
     * the given name is not found.
     */
    NeuronCluster& getCluster( const QString& name );

    /**
     * Returns the metadata of the cluster that matches the given name.
     * @param name the name.
     * @return the metadata.
     * @throw std::out_of_range whether a cluster metadata that matches
     * the given name is not found.
     */
    NeuronClusterMetadata& getMetadata( const QString& name );

  public slots:

    /**
     * Focuses the cluster that matches the given name.
     * @param cluster the name.
     * @return whether the operation was successful or not.
     */
    bool focus( const QString& cluster );

    /**
     * Clears the focus state of the focused cluster.
     */
    void removeFocus( );

    /**
     * Clears the focus state of the focused cluster only
     * if the focused cluster matches the given name.
     * @param cluster the name.
     */
    void removeFocusIfFocused( const QString& cluster );

    /**
     * Adds a new cluster to this manager.
     * @param cluster the cluster.
     */
    void addCluster( const syncopa::NeuronCluster& cluster );

    /**
     * Removes the cluster that matches the given name.
     *
     * This method doesn't remove the cluster's metadata.
     *
     * @param name the name.
     * @return whether the operation was successful or not.
     */
    bool removeCluster( const QString& name );

    /**
     * Moves the cluster that matches the first given name
     * to the position before the cluster that matches the second given name.
     * @param clusterName the name of the cluster to move.
     * @param nextElementName the name of the cluster that will be next to the moved cluster.
     * @return whether the operation was successful or not.
     */
    bool moveBeforeElement( const QString& clusterName ,
                            const QString& nextElementName );

    /**
     * Removes all clusters from this manager.
     * This method doesn't clears the clusters' metadata.
     */
    void clear( );

    /**
     * Emits a new <i>onMetadataModification()</i> signal.
     */
    void broadcastMetadataModification( );

  signals:

    /**
     * Event invoked when a cluster is focused.
     * @param focused the focused cluster.
     */
    void onFocus( syncopa::NeuronCluster* focused );

    /**
     * Event invoked when a cluster is added.
     * @param cluster the cluster.
     * @param index the index of thee cluster.
     */
    void onAddition( syncopa::NeuronCluster& cluster , int index );

    /**
     * Event invoked when a cluster is removed.
     *
     * This event is not invoked when the manager is cleared.
     *
     * @param cluster the cluster.
     */
    void onRemoval( syncopa::NeuronCluster& cluster );

    /**
     * Event invoked when the manager is cleared.
     */
    void onClear( );

    /**
     * Event invoked when a cluster is moved to a new index.
     * @param cluster the cluster.
     * @param newIndex the new index.
     */
    void onMovement( syncopa::NeuronCluster& cluster , int newIndex );

    /**
     * Event invoked when the structure of the clusters was modified.
     *
     * A structure modification may be an addition, a removal or a movement.
     *
     */
    void onStructureModification( );

    /**
     * Event invoked when a cluster's metadata is modified.
     */
    void onMetadataModification( );

  };
}


#endif //SYNCOPA_NEURONCLUSTERMANAGER_H
