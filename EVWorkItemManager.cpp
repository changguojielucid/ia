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



#include "EVWorkItemManager.h"
#include "EVMacros.h"


EVWorkItemManager::EVWorkItemManager()
{
}


EVWorkItemManager::~EVWorkItemManager()
{
  EVWorkItemThreadMap::const_iterator item = m_mapWorkItemThread.constBegin();
  while (item != m_mapWorkItemThread.constEnd())
  {
    EVWorkItemThread* pWorkerThread = item.value();

    ebAssert(pWorkerThread);
    SAFE_REMOVAL(pWorkerThread)
  }
}

EVWorkItemThread* EVWorkItemManager::CreateWorkItemThread(const std::string& seriesPath)
{
  EVWorkItemThread* pWorkItemThread = new EVWorkItemThread();
  m_mapWorkItemThread.insert(m_mapWorkItemThread.size(), pWorkItemThread);

  EVWorkItem* pWorkItem = pWorkItemThread->GetWorkItem();
  ebAssert(pWorkItem);

  QThread* pThread = pWorkItemThread->GetThread();
  ebAssert(pThread);

  pWorkItem->moveToThread(pThread);
  pWorkItem->LoadImageFromSeriesPath(seriesPath);
  pWorkItem->RenderImage();

  pThread->start();

  return pWorkItemThread;
}



