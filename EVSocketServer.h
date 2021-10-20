

// EVServer.h

#ifndef __EV_SOCKETSERVER_H__
#define __EV_SOCKETSERVER_H__

#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>
#include "EVSocketClient.h"

class EVSocketServer : public QTcpServer
{
  Q_OBJECT
public:
  explicit EVSocketServer(QObject *parent = 0);
  void startServer();

protected:
  void incomingConnection(qintptr socketDescriptor);

signals:

  public slots :

};

#endif // __EV_SOCKETSERVER_H__


