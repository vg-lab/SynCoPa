//
// Created by gaeqs on 7/10/24.
//

#include "SynCoPaWebSocket.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QWebSocket>
#include <utility>
#include <sys/socket.h>

SynCoPaWebSocket::SynCoPaWebSocket(uint16_t port, QObject *parent)
    : _server(new QWebSocketServer("SynCoPa", QWebSocketServer::NonSecureMode, parent))
{
    if (_server->listen(QHostAddress::Any, port))
    {
        connect(
            _server,
            &QWebSocketServer::newConnection,
            this,
            &SynCoPaWebSocket::onNewConnection);

        connect(
            _server,
            &QWebSocketServer::closed,
            this,
            &SynCoPaWebSocket::onClose);
    }
}

SynCoPaWebSocket::~SynCoPaWebSocket()
{
    _server->close();
}

void SynCoPaWebSocket::sendMessage(const QString &string)
{
    for (auto socket : _sockets)
    {
        socket->sendTextMessage(string);
    }
}

void SynCoPaWebSocket::sendCommand(const QString &type,
                                   const QJsonObject &data)
{
    QJsonObject root;
    root.insert("type", type);
    root.insert("data", data);

    QJsonDocument doc(root);
    QString string(doc.toJson(QJsonDocument::Compact));
    sendMessage(string);
}

void SynCoPaWebSocket::addListener(
    const QString &type,
    const QObject *parent,
    std::function<void(const QJsonObject &)> callback)
{
    auto &listeners = _listeners[type];
    listeners.push_back({parent, std::move(callback)});
}

void SynCoPaWebSocket::removeListeners(const QString &type,
                                       const QObject *parent)
{
    auto find = _listeners.find(type);
    if (find == _listeners.end())
        return;
    auto &list = find->second;

    QMutableListIterator<Listener> it(list);
    while (it.hasNext())
    {
        if (it.next().parent == parent)
        {
            it.remove();
        }
    }

    if (list.isEmpty())
    {
        _listeners.erase(find);
    }
}

void SynCoPaWebSocket::addConnectionListener(
    const QObject *parent,
    std::function<void(QWebSocket *)> callback)
{
    _connectionListeners.push_back({parent, std::move(callback)});
}

void SynCoPaWebSocket::removeConnectionListeners(const QObject *parent)
{
    QMutableListIterator<ConnectionListener> it(_connectionListeners);
    while (it.hasNext())
    {
        if (it.next().parent == parent)
        {
            it.remove();
        }
    }
}

void SynCoPaWebSocket::onNewConnection()
{
    QWebSocket *socket = _server->nextPendingConnection();
    if (socket == nullptr)
        return;
    _sockets.push_back(socket);
    connect(socket, &QWebSocket::textMessageReceived, this, &SynCoPaWebSocket::onReceiveText);

    std::for_each(_connectionListeners.cbegin(), _connectionListeners.cend(), 
    [&socket](const ConnectionListener &c){ c.callback(socket); });
}

void SynCoPaWebSocket::onSocketDisconnect()
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    if (!client) return;

    _sockets.removeOne(client);
}

void SynCoPaWebSocket::onClose()
{}

void SynCoPaWebSocket::onReceiveText(QString message)
{
    QJsonDocument json = QJsonDocument::fromJson(message.toUtf8());
    if (!json.isObject())
        return;

    QJsonObject root = json.object();

    QJsonValue type = root.value("type");
    if (!type.isString())
        return;
    auto it = _listeners.find(type.toString());
    if (it == _listeners.end())
        return;

    QJsonValue dataRaw = root.value("data");
    QJsonObject data = dataRaw.isObject() ? dataRaw.toObject() : QJsonObject();

    const auto &listeners = it->second;
    std::for_each(listeners.begin(), listeners.end(), [&data](const Listener &l)
                  { l.callback(data); });
}
