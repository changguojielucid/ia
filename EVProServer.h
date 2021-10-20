#pragma once

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtNetwork/QSslError>
#include <QAbstractSocket>
#include <QtWebSockets/qwebsocketprotocol.h>
#include <QSslPreSharedKeyAuthenticator>

#include "EVWebSocketServer.h"
#include "EVWorkClient.h"

class EVWorkClient;
class EVWorkClientManager;

class EVProServer : public EVWebSocketServer
{
public:
  EVProServer(QObject *parent = nullptr);
  ~EVProServer();

  inline EVWorkClientManager* GetWorkClientManager() { return m_pWorkClientManager; }
  EVWorkClient* FindClientForSocket(QWebSocket* pSocket);

  QString GetCertificateFullPath();
  QString GetKeyFileFullPath();
  int  GetPortNumber();


protected:
  void SocketDisconnected();
  void OnNewConnection();
  void ProcessTextMessage(const QString& message);
  void ProcessBinaryMessage(const QByteArray& message);


private:
  QList<EVWorkClient*> m_webSocketClients;
  EVWorkClientManager* m_pWorkClientManager;
};

