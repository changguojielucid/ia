#pragma once

#include "EVFirst.h"
#include "EVTargetDef.h"
#include "EVWorkItem.h"
#include "EVMsgDefine.h"


class EVTargetDefine;
class EVWorkItem;

class EVLinkedViewers :
  public ebvLinkedViewers2
{
public:
  EVLinkedViewers(EVWorkItem* pWorkItem);
  ~EVLinkedViewers();

public:
  inline  EVWorkItem* GetWorkItem() { return m_pWorkItem;  }

  EVTargetDefine* GetTargetDefine();
  void SetupLinkedViewers(const std::string& layout);
  void RenderImage();
  bool AddImageFromMultiReader(ebiMultiImageReader::Pointer pMultiReader, ebID imageID, const std::string& seriesType);

public:
  vtkSmartPointer<vtkRenderWindow> m_pRenwin;


protected:
  EVWorkItem* m_pWorkItem;

};




