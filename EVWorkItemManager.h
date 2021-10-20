#pragma once

#include <QMap>
#include "EVWorkItemThread.h"

typedef QMap<int, EVWorkItemThread*> EVWorkItemThreadMap;

class EVWorkItemManager
{
public:
  EVWorkItemManager();
  ~EVWorkItemManager();

  EVWorkItemThread* CreateWorkItemThread(const std::string& seriesPath);
  EVWorkItemThreadMap* GetWorkItemThreadMap() { return &m_mapWorkItemThread;  }

private:
  EVWorkItemThreadMap m_mapWorkItemThread;

};

