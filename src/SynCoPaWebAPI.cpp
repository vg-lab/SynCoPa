//
// Created by gaeqs on 20/04/22.
//

#include <QJsonObject>

#include "SynCoPaWebAPI.h"
#include "MainWindow.h"
#include "NeuronCluster.h"

SynCoPaWebAPI::SynCoPaWebAPI(MainWindow *window, QObject *parent)
: QObject(parent)
, _window(window)
, _synchronized(false)
{
}

void SynCoPaWebAPI::selection(const QJsonObject &object)
{
  // {"type": "synapse_selection", "data": {"selection": [1, 2, 3, 4]}}
  if (!_synchronized)
    return;

  QJsonValue array = object["selection"];
  if (!array.isArray())
  {
    // qDebug() << "Invalid array";
    return;
  }

  std::vector<unsigned int> selection;
  for (const auto &item : array.toArray())
  {
    if (!item.isDouble())
    {
      // qDebug() << "Invalid item " << item;
      continue;
    }
    selection.push_back(item.toInt());
  }
  _window->manageSynapsesSelectionEvent(selection);
}

void SynCoPaWebAPI::synapsesModeSelection(const QJsonArray &array)
{
  if (!_synchronized)
    return;
  std::vector<unsigned int> selection;
  for (const auto &item : array)
  {
    if (!item.isDouble())
    {
      // qDebug() << "Invalid item " << item;
      continue;
    }
    selection.push_back(item.toInt());
  }
  _window->manageSynapsesSelectionEvent(selection);
}

void SynCoPaWebAPI::pathsModeSelection(const QJsonObject &object)
{
  // {"type": "path_selection", "data": {"pre": 1, "post": [1, 2, 3, 4]}}
  if (!_synchronized)
    return;

  QJsonValue rawPre = object["pre"];
  QJsonValue rawPost = object["post"];
  if (!rawPre.isDouble())
  {
    // qDebug() << "Invalid pre item";
    return;
  }
  if (!rawPost.isArray())
  {
    // qDebug() << "Invalid post item";
    return;
  }

  std::vector<unsigned int> selection;
  for (const auto &item : rawPost.toArray())
  {
    if (!item.isDouble())
    {
      // qDebug() << "Invalid item " << item;
      continue;
    }
    selection.push_back(item.toInt());
  }
  _window->managePathsSelectionEvent(rawPre.toInt(), selection);
}

void SynCoPaWebAPI::neuronCluster(const QJsonObject &object)
{
  // {"type": "cluster", "data": {
  //      "name": "patata",
  //      "color": 0x445566 // OPTIONAL
  //      "selections": [
  //          {
  //              "name": "selection",
  //              "selection": [1, 2, 3, 4],
  //              "morphology_visibility": "all"|"soma_only"|"morphology_only" // OPTIONAL
  //              "synapses_visibility": "hidden"|"connected_only"|"all" // OPTIONAL
  //              "paths_visibility": "hidden"|"connected_only"|"all" // OPTIONAL
  //              "path_type": "pre_only"|"post_only"|"all" // OPTIONAL
  //          }
  //
  //      ]
  // }}

  // qDebug() << object;
  if (!_synchronized)
    return;
  const QJsonValue nameRaw = object["name"];
  if (!nameRaw.isString())
  {
    // qDebug() << "Invalid name: " << nameRaw;
    return;
  }

  const QJsonValue selectionsRaw = object["selections"];
  if (!selectionsRaw.isArray())
  {
    // qDebug() << "Invalid selections: " << nameRaw;
    return;
  }

  std::map<QString, std::unordered_set<unsigned int>> selectionMap;
  for (const auto &item : selectionsRaw.toArray())
  {
    if (!item.isObject())
    {
      // qDebug() << "Invalid item: " << item;
      continue;
    }

    const auto selectionObject = item.toObject();
    const auto selectionName = selectionObject["name"];

    if (!selectionName.isString())
    {
      // qDebug() << "Invalid selection name: " << selectionName;
      continue;
    }

    // endregion

    std::unordered_set<unsigned int> selection;

    const auto selectionIds = selectionObject["selection"];

    if (!selectionIds.isArray())
    {
      // qDebug() << "Invalid selection: " << selectionIds;
      continue;
    }

    for (const auto &gid : selectionIds.toArray())
    {
      if (!gid.isDouble())
      {
        // qDebug() << "Invalid id: " << gid;
        continue;
      }
      selection.insert(gid.toInt());
    }
    selectionMap[selectionName.toString()] = selection;
  }

  syncopa::NeuronCluster cluster(nameRaw.toString(), selectionMap);
  _window->getNeuronClusterManager()->addCluster(cluster);

  // region META OVERRIDE VALUES
  auto &metadata = _window->getNeuronClusterManager()->getMetadata(
      nameRaw.toString());

  const QJsonValue clusterColor = object["color"];
  if (clusterColor.isString())
  {
    metadata.color = QColor(clusterColor.toString());
  }
  // endregion

  for (const auto &item : selectionsRaw.toArray())
  {
    if (!item.isObject())
    {
      // qDebug() << "Invalid item: " << item;
      continue;
    }

    const auto selectionObject = item.toObject();
    const auto selectionName = selectionObject["name"];

    if (!selectionName.isString())
    {
      // qDebug() << "Invalid selection name: " << selectionName;
      continue;
    }

    auto &selectionMeta = metadata.selection[selectionName.toString()];

    // region META OVERRIDE VALUES
    const QJsonValue selectionColor = selectionObject["color"];
    if (selectionColor.isString())
    {
      selectionMeta.color = QColor(selectionColor.toString());
    }

    const QJsonValue morphologyVisibility = selectionObject["morphology_visibility"];
    if (morphologyVisibility.isString())
    {
      auto key = morphologyVisibility.toString().toLower();
      if (syncopa::NEURON_METADATA_SHOW_PARTS.count(key) > 0)
      {
        selectionMeta.partsToShow = syncopa::NEURON_METADATA_SHOW_PARTS.at(key);
      }
    }

    const QJsonValue synapsesVisibility = selectionObject["synapses_visibility"];
    if (synapsesVisibility.isString())
    {
      auto key = synapsesVisibility.toString().toLower();
      if (syncopa::NEURON_METADATA_SYNAPSES_VISIBILITY.count(key) > 0)
      {
        selectionMeta.synapsesVisibility = syncopa::NEURON_METADATA_SYNAPSES_VISIBILITY.at(key);
      }
    }

    const QJsonValue pathsVisibility = selectionObject["paths_visibility"];
    if (pathsVisibility.isString())
    {
      auto key = pathsVisibility.toString().toLower();
      if (syncopa::NEURON_METADATA_PATHS_VISIBILITY.count(key) > 0)
      {
        selectionMeta.pathsVisibility = syncopa::NEURON_METADATA_PATHS_VISIBILITY.at(key);
      }
    }

    const QJsonValue pathsType = selectionObject["paths_type"];
    if (pathsType.isString())
    {
      auto key = pathsType.toString().toLower();
      if (syncopa::NEURON_METADATA_PATH_TYPES.count(key) > 0)
      {
        selectionMeta.pathTypes = syncopa::NEURON_METADATA_PATH_TYPES.at(key);
      }
    }
  }
}

bool SynCoPaWebAPI::isSynchronizedMode()
{
  return _synchronized;
}

void SynCoPaWebAPI::setSynchronizedMode(bool synchronized)
{
  _synchronized = synchronized;
}

void SynCoPaWebAPI::callSynapsesModeSelectionEvent(const QJsonArray &array)
{
  if (_synchronized)
    emit onSynapsesModeSelection(array);
}

void SynCoPaWebAPI::callPathsModeSelectionEvent(const QJsonArray &pre,
                                                const QJsonArray &post)
{
  if (_synchronized)
    emit onPathsModeSelection(pre, post);
}

void SynCoPaWebAPI::registerListener(SynCoPaWebSocket *socket)
{
  socket->addListener("cluster", this, [this](const QJsonObject &obj)
                     { neuronCluster(obj); });

  socket->addListener("synapse_selection", this, [this](const QJsonObject &obj)
                     { selection(obj); });

  socket->addListener("path_selection", this, [this](const QJsonObject &obj)
                     { pathsModeSelection(obj); });
}
