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
#include "EVWorkClient.h"
#include "EVLinkedViewers.h"
#include "EVTargetDefine.h"
#include "EVWorkItem.h"
#include "EVMacros.h"
#include "EVWorkItemThread.h"
#include "EVWorkItemManager.h"
#include "EVWebSocketClient.h"


#ifdef __EV_SERVER_USE_CWEBSOCKET__
  EVWorkClient::EVWorkClient(QObject *parent) : EVWebSocketClient(parent)
#else
  EVWorkClient::EVWorkClient(QObject *parent)
#endif //!__EV_SERVER_USE_CWEBSOCKET__
{
  m_pWorkItemManager = new EVWorkItemManager();
}

bool EVWorkClient::CreateWorkItem(const std::string& seriesPath)
{

  EVWorkItemManager* pWorkItemManager = GetWorkItemManger();
  ebAssert(pWorkItemManager);

  EVWorkItemThread* pWorkItemThread = pWorkItemManager->CreateWorkItemThread(seriesPath);
  ebAssert(pWorkItemThread);

  EVWorkItem* pWorkItem = pWorkItemThread->GetWorkItem();
  ebAssert(pWorkItem);

  QThread* pThread = pWorkItemThread->GetThread();
  ebAssert(pThread);

  connect(pThread, &QThread::finished, pWorkItem, &QObject::deleteLater);
  connect(this, &EVWorkClient::operate, pWorkItem, &EVWorkItem::doWork);
  connect(pWorkItem, &EVWorkItem::resultReady, this, &EVWorkClient::handleResults);

  return true;
}

EVWorkClient::~EVWorkClient()
{
  SAFE_REMOVAL(m_pWorkItemManager)
}

void EVWorkClient::handleResults(const QString &)
{
  // remove this later
}

void EVWorkClient::Receive(const QByteArray& message)
{

}

void EVWorkClient::Receive(const QString& message)
{

}

void EVWorkClient::Receive(const QJsonObject& json)
{

}

void EVWorkClient::Send(const QByteArray& message)
{

}

void EVWorkClient::Send(const QString& message)
{

}

void EVWorkClient::Send(const QJsonObject& json)
{

}
