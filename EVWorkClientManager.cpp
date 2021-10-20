#include "EVWorkClientManager.h"
#include "EVWorkClient.h"
#include "EVMacros.h"

EVWorkClientManager::EVWorkClientManager()
{
}

EVWorkClientManager::~EVWorkClientManager()
{
  EVWorkClientMap::const_iterator item = m_mapWorkClient.constBegin();
  while (item != m_mapWorkClient.constEnd())
  {
    EVWorkClient* pWorkClient = item.value();
    ebAssert(pWorkClient);

    SAFE_REMOVAL(pWorkClient)
  }
}

EVWorkClient* EVWorkClientManager::CreateWorkClient()
{
  EVWorkClient* pWorkClient = new EVWorkClient(nullptr);

  m_mapWorkClient.insert(m_mapWorkClient.size(), pWorkClient);

  return pWorkClient;
}