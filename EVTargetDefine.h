#ifndef _EV_TARGETDEFINE_H_
#define _EV_TARGETDEFINE_H_


#include "EVFirst.h"
#include "manifest.h"
#include "EVWorkItemListFileToken.h"
#include "EVImageSeries.h"
#include "EVMap.h"
#include "EVTargetDef.h"
#include "EVLinkedViewers.h"
#include "EVWorkItem.h"
#include "EVVesselPipeline.h"


class EVWorkItem;
class EVLinkedViewers;

typedef std::pair<ebiMultiImageReader*, ebID> MultiReaderIDPair;


class EVTargetDefine
{
public:
  EVTargetDefine(EVWorkItem* pWorkItem);
  virtual ~EVTargetDefine();

public:
  inline EVWorkItem*   GetWorkItem() { return m_pWorkItem; }
  inline EVTargetDef*  GetCurrentTarget() { return m_pTargetCurrent; }

  vtkSmartPointer<EVLinkedViewers> GetEVLinkedViewers();
  ebiVesselPipeline* GetVesselPipeine();

  void SetCurrentTarget(EVTargetDef* pTarget) { m_pTargetCurrent = pTarget; }

  EVTargetDefine(const EVTargetDefine&) = delete;
  EVTargetDefine& operator=(const EVTargetDefine&) = delete;

public:
  /* interface functions */
  int  PromptMessage(int iIDMsg); // feedback: NONE YES NO IGNOR CANCEL CLOSE ????
  void preloadDefinePre(QString product);

  int GetStageOfClearTargetBeyondCurrentStage(EVTargetDef *def);

  void clearTargetBeyondCurrentStage(EVTargetDef *def);

  void ClearTargetBeyondStageInitialization(EVTargetDef *def, int currentStage);
  void ClearTargetBeyondStageLumenSegmentation(EVTargetDef *def, int currentStage);
  void ClearTargetBeyondStagePath(EVTargetDef *def, int currentStage);
  void ClearTargetBeyondStageLumenPartition(EVTargetDef *def, int currentStage);
  void ClearTargetBeyondStageResampledRegisteredImages(EVTargetDef *def, int currentStage);
  void ClearTargetBeyondStageLumenAndWallSegmentation(EVTargetDef *def, int currentStage);
  void ClearTargetBeyondStageLumenAndWallPartition(EVTargetDef *def, int currentStage);
  void ClearTargetBeyondStageWallThickness(EVTargetDef *def, int currentStage);
  void ClearTargetBeyondStagePerivascularRegion(EVTargetDef *def, int currentStage);
  void ClearTargetBeyondStagePerivascularRegionPartition(EVTargetDef *def, int currentStage);
  void ClearTargetBeyondStageComposition(EVTargetDef *def, int currentStage);
  void ClearTargetBeyondStageCapThickness(EVTargetDef *def, int currentStage);
  void ClearTargetBeyondStageReadings(EVTargetDef *def, int currentStage);
  void ClearTargetBeyondStageLesionLumenPartition(EVTargetDef *def, int currentStage);
  void ClearTargetBeyondStageLesionLumenAndWallPartition(EVTargetDef *def, int currentStage);
  void ClearTargetBeyondStageLesionPerivascularRegionPartition(EVTargetDef *def, int currentStage);
  void ClearTargetBeyondStageLesionReadings(EVTargetDef *def, int currentStage);
  
  void preloadDefinePost();
  void CreateTargetPre(const QString& bodySite);
  void EstablishNewTarget(const QString& bodySite);
    
  bool switchTargetPath();
  void toggleOblique(bool checked);

  void labelDistalVessel(const QString& distalVesselName);
  void completeDistalVessel();
  void addRootVessel(const QString& rootVesselName);
  void addDistalVessel();
  void addVessel(QString startFromName);

  void resetWall();

  void resetTarget();
  void deleteTarget();
  void presentTarget(EVTargetDef *def);
  void selectCurrentTarget(EVTargetDef *Def);

  bool computeRemainingStages(EVTargetDef *def);

  void evaluateLumen();
  void evaluateWall();

  void editSegmentation(bool checked);
  void toggleSegmentation(QString regName, bool checked);

  void giveToAnalyzeIfNotAlreadyThere();
    
  void setDefiningRootFlag(bool flag) { definingRootFlag = flag; }
  bool definingRoot() const { return definingRootFlag; }

  void UpdateSenes();
  void logUpdateWithStageSettings(QString step, EVTargetDef *def, QString ID);


  void resetAllTargetsDueToChangesInImages();
  void resetAllTargetsDueToDifferentImages();

  void processingParametersSettingsChanged();

private:
  QList<EVTargetDef>*          GetTargets();
  EVImageSeries*               GetImageSeries();

private:
  EVTargetDef*                 m_pTargetCurrent; // various methods need to know what target to effect 
  EVWorkItem*                  m_pWorkItem;

  int                          viableTargetsCount;
  int                          savedParallelScale;
  int                          savedPerspectiveDistance;
  int                          savedSlabThickness;

  ebID                         GetImage4ID();
  ebID                         ebvIdVolume;
  ebID                         ebvIdCoronal;
  ebID                         ebvIdSagittal;
  ebID                         ebvIdAxial; // viewer identifiers on the EVTargetDefine screen itself

  ebvViewer::ViewerType        axialType;
  ebvViewer::ViewerType        coronalType;
  ebvViewer::ViewerType        sagittalType;
   
  bool                         editingSegmentation; 
  bool                         definingRootFlag;
  bool                         editingWall;
};

#endif //!_EV_TARGETDEFINE_H_
