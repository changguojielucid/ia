
// myserver.cpp

#include "EVSocketServer.h"

EVSocketServer::EVSocketServer(QObject *parent) :
  QTcpServer(parent)
{
}

void EVSocketServer::startServer()
{
  if (listen(QHostAddress::Any, 1234))
  {
    qDebug() << "Server: started";
  }
  else
  {
    qDebug() << "Server: not started!";
  }
}

void EVSocketServer::incomingConnection(qintptr socketDescriptor)
{
  // At the incoming connection, make a client
  // and set the socket
  EVSocketClient *client = new EVSocketClient(this);
  client->setSocket(socketDescriptor);
}

