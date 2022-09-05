//
// Created by gaeqs on 20/04/22.
//

#ifndef SYNCOPA_SYNCOPAWEBAPI_H
#define SYNCOPA_SYNCOPAWEBAPI_H

#include <QObject>
#include <QJsonArray>

class MainWindow;

class SynCoPaWebAPI : public QObject
{
Q_OBJECT
public:
  explicit SynCoPaWebAPI( MainWindow* window , QObject* parent = nullptr );

  virtual ~SynCoPaWebAPI( )
  { };

  Q_INVOKABLE void selection( const QJsonArray& array );

  Q_INVOKABLE void synapsesModeSelection(
    const QJsonArray& array );

  Q_INVOKABLE void pathsModeSelection(
    const unsigned int pre , const QJsonArray& post );

  Q_INVOKABLE void neuronCluster( const QJsonObject& object );

  bool isSynchronizedMode( );

  void setSynchronizedMode( bool synchronized );

  void callSynapsesModeSelectionEvent( const QJsonArray& array );

  void callPathsModeSelectionEvent( unsigned int pre , const QJsonArray& post );

  void
  callSceneSyncEvent( const QString& hierarchy , const QString& connections );

signals:

  void onSynapsesModeSelection(
    const QJsonArray& array );

  void onPathsModeSelection(
    unsigned int pre , const QJsonArray& post );

  void onSceneSync( const QString& hierarchy , const QString& connections );

private:
  MainWindow* _window;
  bool _synchronized;
};


#endif //SYNCOPA_SYNCOPAWEBAPI_H
