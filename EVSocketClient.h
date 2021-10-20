// myclient.h

#ifndef __EV_SOCKETCLINENT_H__
#define __EV_SOCKETCLINENT_H__

#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include <QThreadPool>
#include "EVTask.h"

class EVSocketClient : public QObject
{
  Q_OBJECT
public:
  explicit EVSocketClient(QObject *parent = 0);
  void setSocket(qintptr Descriptor);

signals:

  public slots :
    void connected();
    void disconnected();
    void readyRead();

  // make the server fully ascynchronous
  // by doing time consuming task
  void TaskResult(int Number);

private:
  QTcpSocket *socket;

};

#endif // __EV_SOCKETCLINENT_H__