//
// Created by gaeqs on 17/3/22.
//

#ifndef SYNCOPA_EVENTS_H
#define SYNCOPA_EVENTS_H

#include <memory>
#include <utility>
#include <vector>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

struct MessageData
{
  virtual ~MessageData( ) = default;

  virtual QString toJSON( ) = 0;
};

struct Message
{
  unsigned short id;
  std::shared_ptr< MessageData > data;
};

struct SynapsesModeSelection : public MessageData
{
  static constexpr int id = 0;
  std::vector< unsigned int > selection;

  SynapsesModeSelection( ) = default;

  explicit SynapsesModeSelection( const QJsonObject& object )
  {
    auto arrayRaw = object[ "selection" ];
    if ( !arrayRaw.isArray( ))
    {
      qDebug( ) << "Couldn't find selection parameter as an array.";
      return;
    }
    auto array = arrayRaw.toArray( );
    for ( const auto& item: array )
    {
      if ( !item.isDouble( ))
      {
        qDebug( ) << "Invalid item " << item;
        continue;
      }
      selection.push_back( item.toInt( ));
    }
  }

  explicit SynapsesModeSelection( std::vector< unsigned int > sel )
  {
    selection = std::move( sel );
  }

  QString toJSON( ) override
  {
    QJsonObject object;
    QJsonArray array;

    for ( const auto& item: selection )
    {
      array.push_back( QJsonValue( static_cast<int>(item)));
    }

    object[ "id" ] = id;
    object[ "selection" ] = array;

    return QJsonDocument( object ).toJson( QJsonDocument::Compact );
  }
};

struct PathsModeSelection : public MessageData
{
  static constexpr int id = 1;
  unsigned int preSelection;
  std::vector< unsigned int > postSelection;

  PathsModeSelection( ) = default;

  explicit PathsModeSelection( const QJsonObject& object )
  {

    auto preRaw = object[ "pre_selection" ];
    if ( !preRaw.isDouble( ))
    {
      qDebug( ) << "Couldn't find pre_selection parameter as an int.";
      return;
    }

    preSelection = preRaw.toInt( );

    auto arrayRaw = object[ "post_selection" ];
    if ( !arrayRaw.isArray( ))
    {
      qDebug( ) << "Couldn't find post_selection parameter as an array.";
      return;
    }

    auto array = arrayRaw.toArray( );
    for ( const auto& item: array )
    {
      if ( !item.isDouble( ))
      {
        qDebug( ) << "Invalid item " << item;
        continue;
      }
      postSelection.push_back( item.toInt( ));
    }
  }

  explicit PathsModeSelection( unsigned int pre ,
                               std::vector< unsigned int > post )
  {
    preSelection = pre;
    postSelection = std::move( post );
  }

  QString toJSON( ) override
  {
    QJsonObject object;
    QJsonArray array;

    for ( const auto& item: postSelection )
    {
      array.push_back( QJsonValue( static_cast<int>(item)));
    }

    object[ "id" ] = id;
    object[ "pre_selection" ] = static_cast<int>(preSelection);
    object[ "post_selection" ] = array;

    return QJsonDocument( object ).toJson( QJsonDocument::Compact );
  }
};

#endif //SYNCOPA_EVENTS_H
