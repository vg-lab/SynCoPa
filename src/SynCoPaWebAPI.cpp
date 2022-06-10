//
// Created by gaeqs on 20/04/22.
//

#include <QJsonObject>

#include "SynCoPaWebAPI.h"
#include "MainWindow.h"
#include "NeuronCluster.h"

SynCoPaWebAPI::SynCoPaWebAPI( MainWindow* window , QObject* parent )
  : QObject( parent )
  , _window( window )
{
}

void SynCoPaWebAPI::selection( const QJsonArray& array )
{
  std::vector< unsigned int > selection;
  for ( const auto& item: array )
  {
    if ( !item.isDouble( ))
    {
      qDebug( ) << "Invalid item " << item;
      continue;
    }
    selection.push_back( item.toInt( ));
  }
  _window->manageSynapsesSelectionEvent( selection );
}

void SynCoPaWebAPI::synapsesModeSelection( const QJsonArray& array )
{
  std::vector< unsigned int > selection;
  for ( const auto& item: array )
  {
    if ( !item.isDouble( ))
    {
      qDebug( ) << "Invalid item " << item;
      continue;
    }
    selection.push_back( item.toInt( ));
  }
  _window->manageSynapsesSelectionEvent( selection );
}

void
SynCoPaWebAPI::pathsModeSelection( const unsigned int pre ,
                                   const QJsonArray& post )
{
  std::vector< unsigned int > selection;
  for ( const auto& item: post )
  {
    if ( !item.isDouble( ))
    {
      qDebug( ) << "Invalid item " << item;
      continue;
    }
    selection.push_back( item.toInt( ));
  }
  _window->managePathsSelectionEvent( pre , selection );
}

void SynCoPaWebAPI::neuronCluster( const QJsonObject& object )
{

  const QJsonValue nameRaw = object[ "name" ];
  if ( !nameRaw.isString( ))
  {
    qDebug( ) << "Invalid name: " << nameRaw;
    return;
  }

  const QJsonValue selectionsRaw = object[ "selections" ];
  if ( !selectionsRaw.isArray( ))
  {
    qDebug( ) << "Invalid selections: " << nameRaw;
    return;
  }

  std::map< QString , std::unordered_set< unsigned int > > selectionMap;
  for ( const auto& item: selectionsRaw.toArray( ))
  {
    if ( !item.isObject( ))
    {
      qDebug( ) << "Invalid item: " << item;
      continue;
    }

    const auto selectionObject = item.toObject( );
    const auto selectionName = selectionObject[ "name" ];

    if ( !selectionName.isString( ))
    {
      qDebug( ) << "Invalid selection name: " << selectionName;
      continue;
    }

    // endregion

    std::unordered_set< unsigned int > selection;

    const auto selectionIds = selectionObject[ "selection" ];

    if ( !selectionIds.isArray( ))
    {
      qDebug( ) << "Invalid selection: " << selectionIds;
      continue;
    }

    for ( const auto& gid: selectionIds.toArray( ))
    {
      if ( !gid.isDouble( ))
      {
        qDebug( ) << "Invalid id: " << gid;
        continue;
      }
      selection.insert( gid.toInt( ));
    }
    selectionMap[ selectionName.toString( ) ] = selection;
  }

  syncopa::NeuronCluster cluster( nameRaw.toString( ) , selectionMap );
  _window->getNeuronClusterManager( )->addCluster( cluster );


  // region META OVERRIDE VALUES
  auto& metadata = _window->getNeuronClusterManager( )->getMetadata(
    nameRaw.toString( ));

  const QJsonValue clusterColor = object[ "color" ];
  if ( clusterColor.isString( ))
  {
    metadata.color = QColor( clusterColor.toString( ));
  }
  // endregion

  for ( const auto& item: selectionsRaw.toArray( ))
  {
    if ( !item.isObject( ))
    {
      qDebug( ) << "Invalid item: " << item;
      continue;
    }

    const auto selectionObject = item.toObject( );
    const auto selectionName = selectionObject[ "name" ];

    if ( !selectionName.isString( ))
    {
      qDebug( ) << "Invalid selection name: " << selectionName;
      continue;
    }

    auto& selectionMeta = metadata.selection[ selectionName.toString( ) ];

    // region META OVERRIDE VALUES
    const QJsonValue selectionColor = selectionObject[ "color" ];
    if ( selectionColor.isString( ))
    {
      selectionMeta.color = QColor( selectionColor.toString( ));
    }

    const QJsonValue morphologyVisibility = selectionObject[ "morphology_visibility" ];
    if ( morphologyVisibility.isString( ))
    {
      auto key = morphologyVisibility.toString( ).toLower( );
      if ( syncopa::NEURON_METADATA_SHOW_PARTS.count( key ) > 0 )
      {
        selectionMeta.partsToShow = syncopa::NEURON_METADATA_SHOW_PARTS
          .at( key );
      }
    }

    const QJsonValue synapsesVisibility = selectionObject[ "synapses_visibility" ];
    if ( synapsesVisibility.isString( ))
    {
      auto key = synapsesVisibility.toString( ).toLower( );
      if ( syncopa::NEURON_METADATA_SYNAPSES_VISIBILITY.count( key ) > 0 )
      {
        selectionMeta.synapsesVisibility =
          syncopa::NEURON_METADATA_SYNAPSES_VISIBILITY.at( key );
      }
    }

    const QJsonValue pathsVisibility = selectionObject[ "paths_visibility" ];
    if ( pathsVisibility.isString( ))
    {
      auto key = pathsVisibility.toString( ).toLower( );
      if ( syncopa::NEURON_METADATA_PATHS_VISIBILITY.count( key ) > 0 )
      {
        selectionMeta.pathsVisibility =
          syncopa::NEURON_METADATA_PATHS_VISIBILITY.at( key );
      }
    }

    const QJsonValue pathsType = selectionObject[ "paths_type" ];
    if ( pathsType.isString( ))
    {
      auto key = pathsType.toString( ).toLower( );
      if ( syncopa::NEURON_METADATA_PATH_TYPES.count( key ) > 0 )
      {
        selectionMeta.pathTypes = syncopa::NEURON_METADATA_PATH_TYPES.at( key );
      }
    }
  }

}
