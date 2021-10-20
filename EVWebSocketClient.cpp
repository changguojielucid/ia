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
#include "EVWebSocketClient.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "QtWebSockets/QWebSocket"
#include "EVMacros.h"

EVWebSocketClient::EVWebSocketClient(QObject *parent) : QObject(parent)
{
  m_pWebSocket = nullptr;
}

EVWebSocketClient::~EVWebSocketClient()
{
  if (m_pWebSocket)
    m_pWebSocket->deleteLater();
  
  SAFE_REMOVAL(m_pWebSocket);
}

void EVWebSocketClient::Receive(const QByteArray& message)
{
   qDebug() << ("Binary message not handled");
}


void EVWebSocketClient::Receive(const QString& message) // data received from remote computer
{
  QJsonObject json = QJsonDocument::fromJson(message.toUtf8()).object();
  
  if(json["type"].isNull()) 
  {
    return;
  }
}

void EVWebSocketClient::Receive(const QJsonObject& json)    // data received from remote client
{
  if (json["type"].isNull())
  {
    return;
  }
}

void EVWebSocketClient::Send(const QByteArray& message)
{

}

void EVWebSocketClient::Send(const QString& message) // send to remote computer
{
  QWebSocket* pWebSocket = GetWebSocket();
  ebAssert(pWebSocket);

  if (pWebSocket)
    pWebSocket->sendTextMessage(message);
}

void EVWebSocketClient::Send(const QJsonObject& json) // send to remote computer
{
  QJsonDocument jsondoc(json);
  Send(QString::fromUtf8(jsondoc.toJson()));
}
