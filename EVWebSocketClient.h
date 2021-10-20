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

#ifndef __EV_WEBSOCKET_CLIENT_H__
#define __EV_WEBSOCKET_CLIENT_H__

#include <QObject>

class QWebSocket;
class QJsonObject;

class EVWebSocketClient : public QObject
{
  Q_OBJECT
public:
  explicit EVWebSocketClient(QObject *parent = nullptr);
  ~EVWebSocketClient();

  inline QWebSocket* GetWebSocket() { return m_pWebSocket; }

  virtual void Receive(const QByteArray& message); // data received from remote client
  virtual void Receive(const QString& message);    // data received from remote client
  virtual void Receive(const QJsonObject& json);    // data received from remote client

  virtual void Send(const QByteArray& message);    // send to remote client
  virtual void Send(const QString& message);       // send to remote client
  virtual void Send(const QJsonObject& json);      // send to remote client


public:


  QWebSocket* m_pWebSocket;

private:
  private slots:

  signals:
  public slots :
};


#endif //__EV_WEBSOCKET_CLIENT_H__