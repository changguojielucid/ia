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

#include "EVFirst.h"
#include "EVWebSocketServer.h"
#include "QtWebSockets/QWebSocketServer"
#include "QtWebSockets/QWebSocket"
#include <QMetaEnum>
#include <QtCore/QFile>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>
#include "EVWebSocketClient.h"
#include "EVMacros.h"
#include "EVWorkClient.h"
#include "EVWorkClientManager.h"

EVWebSocketServer::EVWebSocketServer(QObject *parent) : 
  QObject(parent), 
  m_pWebSocketServer(nullptr)
{

}

EVWebSocketServer::~EVWebSocketServer()
{
  QWebSocketServer* pWebSocketServer = GetWebSocketServer();
  ebAssert(pWebSocketServer);

  pWebSocketServer->close();

  SAFE_REMOVAL(pWebSocketServer)
}


bool EVWebSocketServer::Run()
{
  if (!SslConfiguration())
  {
    ebAssert(false);
    return false;
  }

  int port = GetPortNumber();

  QWebSocketServer* pWebSocketServer = GetWebSocketServer();
  ebAssert(pWebSocketServer);

  if (pWebSocketServer->listen(QHostAddress::Any, port))
  {
    qDebug() << (QString("WebSocket listening on wss://localhost:") + QString::number(port));

    connect(pWebSocketServer, &QWebSocketServer::newConnection, this, &EVWebSocketServer::OnNewConnection);
    connect(pWebSocketServer, &QWebSocketServer::acceptError, this, &EVWebSocketServer::onAcceptError);
    connect(pWebSocketServer, &QWebSocketServer::peerVerifyError, this, &EVWebSocketServer::onPeerVerifyError);
    connect(pWebSocketServer, &QWebSocketServer::serverError, this, &EVWebSocketServer::onServerError);
    connect(pWebSocketServer, &QWebSocketServer::sslErrors, this, &EVWebSocketServer::onSslErrors);

    return true;
  }

  return false;
}

bool EVWebSocketServer::SslConfiguration()
{
  QString strCertFile = GetCertificateFullPath();
  QFile certFile(strCertFile);

  if (!certFile.open(QIODevice::ReadOnly))
  {
    ebAssert(false);
    return false;
  }

  QString strKeyFile = GetKeyFileFullPath();
  QFile keyFile(strKeyFile);

  if (!keyFile.open(QIODevice::ReadOnly))
  {
    ebAssert(false);
    return false;
  }

  QSslCertificate certificate(&certFile, QSsl::Pem);
  QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);

  certFile.close();
  keyFile.close();

  QSslConfiguration sslConfiguration;
  sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
  sslConfiguration.setLocalCertificate(certificate);
  sslConfiguration.setPrivateKey(sslKey);
  sslConfiguration.setProtocol(QSsl::TlsV1SslV3);

  QWebSocketServer* pWebSocketServer = GetWebSocketServer();
  ebAssert(pWebSocketServer);
  pWebSocketServer->setSslConfiguration(sslConfiguration);

  return true;
}

QWebSocketServer* EVWebSocketServer::GetWebSocketServer()
{
  if(nullptr == m_pWebSocketServer)
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("EVProServer"), QWebSocketServer::SecureMode, this);

  return m_pWebSocketServer;
}


void EVWebSocketServer::onAcceptError(QAbstractSocket::SocketError socketError)
{
  QMetaEnum metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketError>();
  qDebug() << (QString("WebSocket Accept Error:") + metaEnum.valueToKey(socketError));
}

void EVWebSocketServer::onPeerVerifyError(QSslError socketError)
{
  qDebug() << (QString("WebSocket Peer Verify Error:") + socketError.errorString());
}

void EVWebSocketServer::onServerError(QWebSocketProtocol::CloseCode closeCode) {
  qDebug() << (QString("WebSocket Server Error:") + QString::number((int)closeCode));
}

void EVWebSocketServer::OnNewConnection()
{
  qDebug() << ("OnNewConnection not handled");
}

void EVWebSocketServer::ProcessTextMessage(const QString& message)
{
  qDebug() << ("Text WebSocket message not handled");
}

void EVWebSocketServer::ProcessBinaryMessage(const QByteArray& message)
{
  qDebug() << ("Binary WebSocket message not handled");
}


void EVWebSocketServer::onSslErrors(const QList<QSslError> &)
{
  qDebug() << ("SSL errors occurred");
}