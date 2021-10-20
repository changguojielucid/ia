#pragma once

#include "EVFirst.h"
#include <QMap>
#include "EVWorkItemManager.h"
#include "EVWebSocketClient.h"

#ifdef __EV_SERVER_USE_CWEBSOCKET__
  class EVWorkClient : public  EVWebSocketClient
#else
  class EVWorkClient : public  QObject
#endif //!__EV_SERVER_USE_CWEBSOCKET__
{
  Q_OBJECT
public:
  EVWorkClient(QObject *parent = nullptr);
  ~EVWorkClient();

  void Receive(const QByteArray& message); // data received from remote client
  void Receive(const QString& message);    // data received from remote client
  void Receive(const QJsonObject& json);    // data received from remote client

  void Send(const QByteArray& message);    // send to remote client  
  void Send(const QString& message);       // send to remote client
  void Send(const QJsonObject& json);      // send to remote client

  bool CreateWorkItem(const std::string& seriesPath);
  EVWorkItemManager* GetWorkItemManger() { return m_pWorkItemManager; }

public slots:
  void handleResults(const QString &);


signals:
  void operate(const QString &);

private:
  EVWorkItemManager* m_pWorkItemManager;

};

