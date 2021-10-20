#pragma once
#include "ebiVesselPipeline.h"
#include "EVWorkItem.h"

class EVWorkItem;

class EVVesselPipeline : public ebiVesselPipeline
{
public:
  EVVesselPipeline(EVWorkItem* pWorkItem);
  ~EVVesselPipeline();

  void SetPrimaryImageID(ebID imageID) {  primaryImageID = imageID; }


};

