#pragma once

#include <QMap>
#include "EVWorkClient.h"

typedef QMap<int, EVWorkClient*> EVWorkClientMap;

class EVWorkClient;

class EVWorkClientManager
{
public:
  EVWorkClientManager();
  ~EVWorkClientManager();

  EVWorkClient* CreateWorkClient();

private:
  EVWorkClientMap m_mapWorkClient;

};

