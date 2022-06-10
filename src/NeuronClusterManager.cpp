//
// Created by gaeqs on 4/05/22.
//

#include <QDebug>

#include <functional>

#include "NeuronClusterManager.h"

namespace syncopa
{

  NeuronClusterManager::NeuronClusterManager( )
    :
    _clusters( )
    , _metadata( )
    , _focused( "" )
  {

    // DEFAULT COLORS
    NeuronClusterMetadata selection;
    selection.closeable = false;
    selection.selection[ "Presynaptic" ] = NeuronSelectionMetadata(
      true ,
      syncopa::NeuronMetadataShowPart::ALL ,
      SynapsesVisibility::HIDDEN ,
      PathsVisibility::HIDDEN ,
      PathTypes::PRE_ONLY ,
      QColor( 102 , 102 , 102 )
    );
    selection.selection[ "Postsynaptic" ] = NeuronSelectionMetadata(
      true ,
      syncopa::NeuronMetadataShowPart::ALL ,
      SynapsesVisibility::HIDDEN ,
      PathsVisibility::HIDDEN ,
      PathTypes::POST_ONLY ,
      QColor( 153 , 127 , 127 )
    );
    selection.selection[ "Connected" ] = NeuronSelectionMetadata(
      true ,
      syncopa::NeuronMetadataShowPart::SOMA_ONLY ,
      SynapsesVisibility::HIDDEN ,
      PathsVisibility::HIDDEN ,
      PathTypes::ALL ,
      QColor( 179 , 127 , 0 )
    );
    selection.selection[ "Other" ] = NeuronSelectionMetadata(
      true ,
      syncopa::NeuronMetadataShowPart::SOMA_ONLY ,
      SynapsesVisibility::ALL ,
      PathsVisibility::HIDDEN ,
      PathTypes::ALL ,
      QColor( 76 , 76 , 76 )
    );

    _metadata[ "Main Group" ] = selection;
  }

  std::vector< NeuronCluster >::iterator
  NeuronClusterManager::findCluster( const QString& name )
  {
    return std::find_if( _clusters.begin( ) , _clusters.end( ) ,
                         [ &name ]( const NeuronCluster& other )
                         {
                           return other.getName( ) == name;
                         } );
  }

  const QString& NeuronClusterManager::getFocused( ) const
  {
    return _focused;
  }

  NeuronCluster& NeuronClusterManager::getCluster( const QString& name )
  {
    auto index = findCluster( name );
    return _clusters.at( std::distance( _clusters.begin( ) , index ));
  }

  NeuronClusterMetadata&
  NeuronClusterManager::getMetadata( const QString& name )
  {
    return _metadata.at( name );
  }

  void NeuronClusterManager::addCluster( const NeuronCluster& cluster )
  {
    auto index = findCluster( cluster.getName( ));
    if ( index != _clusters.end( ))
    {
      // Override
      *index = cluster;
      auto position = std::distance( _clusters.begin( ) , index );
      emit onAddition( *index , static_cast<int>(position));
      emit onStructureModification( );
    }
    else
    {
      _clusters.push_back( cluster );

      // Create metadata if not found
      if ( !_metadata.count( cluster.getName( )))
      {
        auto clusterHash = std::hash< std::string >{ }(
          cluster.getName( ).toStdString( ));
        auto clusterMetadata = NeuronClusterMetadata( );

        clusterMetadata.color = QColor(
          static_cast<int>(clusterHash & 0xFF) ,
          static_cast<int>(( clusterHash >> 8 ) & 0xFF) ,
          static_cast<int>(( clusterHash >> 16 ) & 0xFF)
        );

        for ( const auto& item: cluster.getSelections( ))
        {
          auto selectionHash = std::hash< std::string >{ }(
            ( cluster.getName( ) + item.first ).toStdString( ));
          auto selectionMetadata = NeuronSelectionMetadata( );

          selectionMetadata.color = QColor(
            static_cast<int>(selectionHash & 0xFF) ,
            static_cast<int>(( selectionHash >> 8 ) & 0xFF) ,
            static_cast<int>(( selectionHash >> 16 ) & 0xFF)
          );
          clusterMetadata.selection[ item.first ] = selectionMetadata;
        }
        _metadata[ cluster.getName( ) ] = clusterMetadata;
      }
      emit onAddition( _clusters.back( ) , static_cast<int>(_clusters.size( )));
      emit onStructureModification( );
    }
  }

  bool NeuronClusterManager::removeCluster( const QString& name )
  {
    auto element = findCluster( name );
    if ( element == _clusters.end( )) return false;

    if ( name == _focused )
    {
      removeFocus( );
    }

    emit onRemoval( *element );
    _clusters.erase( element );

    emit onStructureModification( );
    return true;
  }

  const std::vector< NeuronCluster >& NeuronClusterManager::getClusters( )
  {
    return _clusters;
  }

  std::map< QString , NeuronClusterMetadata >&
  NeuronClusterManager::getMetadata( )
  {
    return _metadata;
  }

  bool NeuronClusterManager::moveBeforeElement( const QString& clusterName ,
                                                const QString& nextElementName )
  {
    auto nextElement = findCluster( nextElementName );
    if ( nextElement == _clusters.end( )) return false;

    auto element = findCluster( clusterName );
    if ( element == _clusters.end( ) || element == nextElement ) return false;

    NeuronCluster elementBase = *element.base( );
    _clusters.erase( element );

    nextElement = findCluster( nextElementName );
    auto nextElementIndex = std::distance( _clusters.begin( ) , nextElement );

    _clusters.insert( nextElement , elementBase );
    emit onMovement( elementBase , static_cast<int>(nextElementIndex));
    emit onStructureModification( );
    return true;
  }

  void NeuronClusterManager::clear( )
  {
    _focused = "";
    _clusters.clear( );
    emit onClear( );
    emit onStructureModification( );
  }

  bool NeuronClusterManager::focus( const QString& cluster )
  {
    auto element = findCluster( cluster );
    if ( element == _clusters.end( )) return false;
    auto focused = element.base( );
    _focused = focused->getName( );
    emit onFocus( focused );
    return true;
  }

  void NeuronClusterManager::removeFocus( )
  {
    _focused = "";
    emit onFocus( nullptr );
  }

  void NeuronClusterManager::removeFocusIfFocused( const QString& cluster )
  {
    if ( !_focused.isEmpty( ) && _focused == cluster )
    {
      removeFocus( );
    }
  }

  void NeuronClusterManager::broadcastMetadataModification( )
  {
    emit onMetadataModification( );
  }

}