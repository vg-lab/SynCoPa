//
// Created by gaeqs on 20/04/22.
//

#ifndef SYNCOPA_SYNCOPAWEBAPI_H
#define SYNCOPA_SYNCOPAWEBAPI_H

#include <QObject>
#include <QJsonArray>

#include "SynCoPaWebSocket.h"

class MainWindow;

class SynCoPaWebAPI : public QObject {
    Q_OBJECT

public:
    explicit SynCoPaWebAPI(MainWindow* window, QObject* parent = nullptr);

    ~SynCoPaWebAPI() override = default;

    Q_INVOKABLE void selection(const QJsonObject& object);

    Q_INVOKABLE void synapsesModeSelection(
        const QJsonArray& array);

    Q_INVOKABLE void pathsModeSelection(const QJsonObject& object);

    void neuronCluster(const QJsonObject& object);

    bool isSynchronizedMode();

    void setSynchronizedMode(bool synchronized);

    void callSynapsesModeSelectionEvent(const QJsonArray& array);

    void callPathsModeSelectionEvent(const QJsonArray& pre, const QJsonArray& post);

    void registerListener(SynCoPaWebSocket* socket);

signals:
    void onSynapsesModeSelection(
        const QJsonArray& array);

    void onPathsModeSelection(
        const QJsonArray& pre, const QJsonArray& post);

private:
    MainWindow* _window;
    bool _synchronized;
};


#endif //SYNCOPA_SYNCOPAWEBAPI_H
