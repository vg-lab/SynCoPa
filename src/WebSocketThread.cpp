/*
 * @file  WebSocketThread.cpp
 * @brief
 * @author Gael Rial Costas <g.rial.2018@alumnos.urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

// Project
#include "WebSocketThread.h"
#include "Events.h"

// Qt
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

WebSocketThread::WebSocketThread( const QUrl& url , QObject* parent )
  : QThread( parent )
  , m_socket( nullptr )
  , m_url( url )
{
  qRegisterMetaType< QAbstractSocket::SocketError >( "WebSocketThread::Error" );
  qRegisterMetaType< QAbstractSocket::SocketState >( "WebSocketThread::State" );
}

void WebSocketThread::run( )
{
  m_socket = std::make_shared<QWebSocket>();

  connect(m_socket.get(), SIGNAL(connected()),    this, SIGNAL(connected()));
  connect(m_socket.get(), SIGNAL(disconnected()), this, SIGNAL(disconnected()));
  connect(m_socket.get(), SIGNAL(error(QAbstractSocket::SocketError)),
          this,           SLOT(error( QAbstractSocket::SocketError)));
  connect(m_socket.get(), SIGNAL(textMessageReceived(const QString &)),
          this,           SLOT(onMessageReceived(const QString &)));

  m_socket->open(m_url);

  exec();

  m_socket->abort();
  m_socket = nullptr;
}

void WebSocketThread::stop()
{
  exit();
}

Message WebSocketThread::getMessage( )
{
  QMutexLocker lock( &m_mutex );

  Message message;

  if ( !m_messages.isEmpty( ))
    message = m_messages.takeFirst( );

  return message;
}

bool WebSocketThread::pendingMessages()
{
  QMutexLocker lock(&m_mutex);

  return !m_messages.isEmpty();
}

void WebSocketThread::error( QAbstractSocket::SocketError code )
{
  switch(code)
  {
    case QAbstractSocket::ConnectionRefusedError:
      m_errorMessage = "Connection refused.";
      break;
    case QAbstractSocket::RemoteHostClosedError:
      m_errorMessage = "Connection closed.";
      break;
    case QAbstractSocket::HostNotFoundError:
      m_errorMessage = "Connection refused or host not found.";
      break;
    case QAbstractSocket::SocketAccessError:
    case QAbstractSocket::SocketResourceError:
    case QAbstractSocket::SocketTimeoutError:
      m_errorMessage = "Socket error.";
      break;
    default:
      m_errorMessage = QString("Socket Wrapper error, code: %1").arg(static_cast<int>(code));
      break;
  }

  emit error();
}

void WebSocketThread::sendMessage(const QString &text)
{
  if(isConnected())
  {
    m_socket->sendTextMessage(text);
    m_socket->flush();
  }
}

void WebSocketThread::onMessageReceived(const QString &text)
{
  {
    const QJsonDocument document = QJsonDocument::fromJson( text.toUtf8( ));
    const QJsonObject object = document.object( );
    const QJsonValue id = object[ "id" ];
    if ( !id.isDouble( )) return;
    Message message;
    message.id = id.toInt( );
    switch ( message.id )
    {
      // SELECT
      case 0:
        message.data = std::make_shared< SynapsesModeSelection >( object );
        break;
      case 1:
        message.data = std::make_shared< PathsModeSelection >( object );
        break;
      default:
        break;
    }

    QMutexLocker lock( &m_mutex );
    m_messages << message;
  }

  emit messageReceived();
}
