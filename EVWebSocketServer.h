/*
*             dP                   oo       dP          oo
*             88                            88
*    .d8888b. 88 dP    dP .d8888b. dP .d888b88 dP   .dP dP dP   .dP .d8888b.
*    88ooood8 88 88    88 88'  `"" 88 88'  `88 88   d8' 88 88   d8' 88'  `88
*    88.  ... 88 88.  .88 88.  ... 88 88.  .88 88 .88'  88 88 .88'  88.  .88
*    `88888P' dP `88888P' `88888P' dP `88888P8 8888P'   dP 8888P'   `88888P'
*    oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
*

*
*    Created:
*
*    Modified:
*
*
*
*
*
*                              Copyright (c) Elucid Bioimaging
*/

#ifndef __EV_WEBSOCKET_SERVER_H__
#define __EV_WEBSOCKET_SERVER_H__

#include "EVFirst.h"
#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtCore/QFile>
#include <QtNetwork/QSslError>
#include <QAbstractSocket>
#include <QtWebSockets/qwebsocketprotocol.h>
#include <QSslPreSharedKeyAuthenticator>
#include "EVWebSocketClient.h"
#include "EVWorkClient.h"


class QWebSocket; 
class QWebSocketServer;
class EVWorkClientManager;

class EVWebSocketServer : public QObject
{
  Q_OBJECT
public:
  EVWebSocketServer(QObject *parent = nullptr);
  virtual  ~EVWebSocketServer();

public:
  QWebSocketServer* GetWebSocketServer();
  bool Run();

Q_SIGNALS:
  void closed();

protected:
  virtual QString GetCertificateFullPath() = 0;
  virtual QString GetKeyFileFullPath() = 0;
  virtual int  GetPortNumber() = 0;

  virtual void OnNewConnection();
  virtual void ProcessTextMessage(const QString& message);
  virtual void ProcessBinaryMessage(const QByteArray& message);

  bool SslConfiguration();

private Q_SLOTS:
  void onAcceptError(QAbstractSocket::SocketError socketError);
  void onPeerVerifyError(QSslError socketError);
  void onServerError(QWebSocketProtocol::CloseCode closeCode);
  void onSslErrors(const QList<QSslError> &errors);

signals:

public slots :

private:
  QWebSocketServer*  m_pWebSocketServer;
  
};

#endif //__EV_WEBSOCKET_SERVER_H__