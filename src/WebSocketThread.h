/*
 * @file  WebSocketThread.h
 * @brief
 * @author Gael Rial Costas <g.rial.2018@alumnos.urjc.es>
 * @date
 * @remarks Copyright (c) GMRV/URJC. All rights reserved.
 *          Do not distribute without further notice.
 */

#ifndef SYNCOPA_WEBSOCKETTHREAD_H
#define SYNCOPA_WEBSOCKETTHREAD_H

// Qt
#include <QThread>
#include <QMutex>
#include <QWebSocket>

// Project
#include "Events.h"

// C++
#include <memory>

/** \class WebSocketThread
 * \brief Implements a thread to send and receive messages using a websocket.
 *
 */
class WebSocketThread : public QThread
{
   Q_OBJECT
public:

  /** \brief WebSocketThread class constructor.
   * \param[in] url Url to connect.
   *
   */
  WebSocketThread( const QUrl& url, QObject* parent );

  /** \brief Sends the given text message.
   * \param[in] text Message to send.
   *
   */
  void sendMessage(const QString &text);

  /** \brief Returns the current error message or empty if none.
   *
   */
  QString errorMessage() const
  { return m_errorMessage; }

  /** \brief Returns true if connected and false otherwise.
   *
   */
  bool isConnected() const
  { return m_socket && m_socket->isValid(); }

  /** \brief Returns a pending message.
   *
   */
  Message getMessage( );

  /** \brief Returns true if there are pending messages.
   *
   */
  bool pendingMessages();

public slots:
  /** \brief Stops the thread and finishes.
   *
   */
  void stop();

signals:
  void connected();
  void disconnected();
  void messageReceived();
  void error();

protected:
  virtual void run( ) override;

private slots:
  /** \brief Processes the socket error and updates the thread error message.
   * \param[in] error Socket error code.
   *
   */
  void error( QAbstractSocket::SocketError error );

  /** \brief Adds the given message to the list of pending messages.
   * \param[in] text Message text.
   *
   */
  void onMessageReceived(const QString &text);

private:
  std::shared_ptr<QWebSocket> m_socket;       /** qt websocket implementation.                       */
  QUrl                        m_url;          /** url to connect.                                    */
  QList<Message>              m_messages;     /** list of received messages pending to be retrieved. */
  QMutex                      m_mutex;        /** message list mutex.                                */
  QString                     m_errorMessage; /** thread error message.                              */
};


#endif //SYNCOPA_WEBSOCKETTHREAD_H
