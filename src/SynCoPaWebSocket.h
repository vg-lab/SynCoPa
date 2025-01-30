//
// Created by gaeqs on 7/10/24.
//

#ifndef SYNCOPAWEBSOCKET_H
#define SYNCOPAWEBSOCKET_H

#include <cstdint>
#include <QWebSocket>
#include <QWebSocketServer>
#include <map>

class SynCoPaWebSocket : public QObject {
    Q_OBJECT

    struct Listener {
        const QObject* parent;
        std::function<void(const QJsonObject&)> callback;
    };

    struct ConnectionListener {
        const QObject* parent;
        std::function<void(QWebSocket*)> callback;
    };

    QWebSocketServer* _server;
    QList<QWebSocket*> _sockets;

    std::map<QString, QList<Listener>> _listeners;

    QList<ConnectionListener> _connectionListeners;

public:
    SynCoPaWebSocket(uint16_t port, QObject* parent);

    ~SynCoPaWebSocket() override;

    void sendMessage(const QString& string);

    void sendCommand(const QString& type, const QJsonObject& data);

    void addListener(const QString& type,
                     const QObject* parent,
                     std::function<void(const QJsonObject&)> callback);

    void removeListeners(const QString& type, const QObject* parent);

    void addConnectionListener(const QObject* parent,
                               std::function<void(QWebSocket*)> callback);

    void removeConnectionListeners(const QObject* parent);

private slots:
    void onNewConnection();

    void onSocketDisconnect();

    void onClose();

    void onReceiveText(QString message);
};


#endif //SYNCOPAWEBSOCKET_H
