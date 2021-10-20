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
#include "EVTargetDef.h"
#include "EVTargetDefine.h"
#include "EVMsgDefine.h"
#include "EVWorkItem.h"

#define tr  

EVTargetDefine::EVTargetDefine(EVWorkItem* pWorkItem)
{
  m_pWorkItem = pWorkItem;
  m_pTargetCurrent = NULL;
 
  viableTargetsCount = 0;
  definingRootFlag = true;
  editingSegmentation = false;
}

EVTargetDefine::~EVTargetDefine()
{
}

ebiVesselPipeline* EVTargetDefine::GetVesselPipeine()
{
  EVWorkItem* pWorkItem = GetWorkItem();
  ebAssert(pWorkItem);
  return pWorkItem->GetVesselPipeline();
}

vtkSmartPointer<EVLinkedViewers> EVTargetDefine::GetEVLinkedViewers()
{
  EVWorkItem* pWorkItem = GetWorkItem();
  ebAssert(pWorkItem);
  return pWorkItem->GetEVLinkerViewers();
}

EVImageSeries* EVTargetDefine::GetImageSeries()
{
  EVWorkItem* pWorkItem = GetWorkItem();
  ebAssert(pWorkItem);
  return pWorkItem->GetImageSeries();
}

ebID EVTargetDefine::GetImage4ID()
{
  EVImageSeries* pImageSeries = GetImageSeries();
  ebAssert(pImageSeries);
  return pImageSeries->image4IDdefine;
}

QList<EVTargetDef>* EVTargetDefine::GetTargets()
{
  EVWorkItem* pWorkItem = GetWorkItem();
  ebAssert(pWorkItem);
  return pWorkItem->GetTargetDefs();
}

void EVTargetDefine::preloadDefinePre(QString product)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  setDefiningRootFlag(false);

  //client should update UI states
  PromptMessage(EV_MSG_PRELOAD_DEFINE_PRE_FINISHED);
}

void EVTargetDefine::processingParametersSettingsChanged()
{
  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);

  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  clearTargetBeyondCurrentStage(currentTarget);
  currentTarget->unsavedChanges = true;
  logUpdateWithStageSettings("EVTargetDefine::processingParametersSettingsChanged", currentTarget, currentTarget->getID());
  pEVLineedViewers->SyncViewersToScene();
  pEVLineedViewers->Render();
}

void EVTargetDefine::presentTarget(EVTargetDef *def)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers)

  // work forward, as deep as can go based on what is in place (except not as far as patientAnalyze goes)
  if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization()) {
    def->initializerIDdefine = pEVLineedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization(), "initializer");
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation()) { 
      def->lumenSegIDdefine = pEVLineedViewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation(), 0, "lumenSegmentation");
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath()) {
        pEVLineedViewers->RemoveVesselTarget(def->initializerIDdefine); // if have a path, don't want to display the initializer too
        def->initializerIDdefine = null_ebID;
        def->pathIDdefine = pEVLineedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath(), "path");
        if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenPartition()) {
          pEVLineedViewers->RemoveSegmentation4(def->lumenSegIDdefine); // if have partitions, don't want to display the segmentation too
          def->lumenSegIDdefine = null_ebID;
          def->lumenPartIDsDefine.clear();
          int i = 0;
          for (auto partition : *pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenPartition())
            def->lumenPartIDsDefine.insert(pEVLineedViewers->AddSegmentation4(partition.GetPointer(), 0, "lumenPartition"+std::to_string(i++)));
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallSegmentation()) {
            def->wallSegIDdefine = pEVLineedViewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallSegmentation(), 0, "wallSegmentation");
            pEVLineedViewers->GetScene()->GetSegmentation4(def->wallSegIDdefine)->GetInteriorProperty()->SetOpacity(0);
            if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallPartition()) {
              pEVLineedViewers->RemoveSegmentation4(def->wallSegIDdefine); // if have partitions, don't want to display the segmentation too
              def->wallSegIDdefine = null_ebID;
              def->wallPartIDsDefine.clear();
              int i = 0;
              for (auto partition : *pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallPartition()) {
                ebID partID = pEVLineedViewers->AddSegmentation4(partition.GetPointer(), 0, "wallPartition"+std::to_string(i++));
                def->wallPartIDsDefine.insert(partID);
                pEVLineedViewers->GetScene()->GetSegmentation4(partID)->GetInteriorProperty()->SetOpacity(0);
              }
            }
          }
        }
      }
    }
  } 
  pEVLineedViewers->SyncViewersToScene();
  pEVLineedViewers->Render();
}

void EVTargetDefine::logUpdateWithStageSettings(QString step, EVTargetDef *def, QString ID)
{
}


void EVTargetDefine::UpdateSenes()
{
  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);
  pEVLineedViewers->SyncViewersToScene();
  pEVLineedViewers->Render();
}

int EVTargetDefine::PromptMessage(int iIDMsg)
{
  // client respond this message ID

  return 0;
}

void EVTargetDefine::selectCurrentTarget(EVTargetDef *newCurrentDef)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  std::cerr << GetEVLinkedViewers() << std::endl;

  SetCurrentTarget(newCurrentDef);

  EVTargetDef* currentTarget = GetCurrentTarget();
  //ebAssert(currentTarget);

  QList<EVTargetDef>* targets = GetTargets();
  ebAssert(targets);

 
  QString label;
  if (currentTarget != NULL) {
    if ((currentTarget->lesionReadingsIDdefine != null_ebID) || (currentTarget->readingsIDdefine != null_ebID) ||
              (currentTarget->pathIDdefine != null_ebID) || (currentTarget->initializerIDdefine != null_ebID)) {
      ebID id = null_ebID;
      if (currentTarget->lesionReadingsIDdefine != null_ebID)
        id = currentTarget->lesionReadingsIDdefine;
      else if (currentTarget->readingsIDdefine != null_ebID)
        id = currentTarget->readingsIDdefine;
      else if (currentTarget->pathIDdefine != null_ebID)
        id = currentTarget->pathIDdefine;
      else if (currentTarget->initializerIDdefine != null_ebID)
        id = currentTarget->initializerIDdefine;
      GetEVLinkedViewers()->GetScene()->SetSelectedVesselTarget(id);
      ebAssert(GetEVLinkedViewers() && GetEVLinkedViewers()->GetScene() && GetEVLinkedViewers()->GetScene()->GetSelectedVesselTarget());
      GetEVLinkedViewers()->GetScene()->GetSelectedVesselTarget()->GetVesselTarget()->SetName(currentTarget->getID().toStdString());
    }
    label = "Current target being defined: ";

    QString displayName = currentTarget->getID();
    // override the display name if it is a coronary target (because the greater branching complexity makes their screen labelling confusing if we don't do this)
    if (displayName.contains("oronary") && currentTarget->rootVesselName != "") {
      displayName = currentTarget->rootVesselName;
    }
    label.append(displayName);
    bool atLeastOneAlreadyInTheParens = false;
    if (targets->size() > 1) {
      label.append(" (other available targets include: ");
      for (int k=0; k < targets->size(); k++) {
        EVTargetDef *def = &((*targets)[k]);
        if (def->getID() != currentTarget->getID()) {
          QString otherTargetDisplayName = def->getID();
          // override the display name if it is a coronary target (because the greater branching complexity makes their screen labelling confusing if we don't do this)
          if (otherTargetDisplayName.contains("oronary") && def->rootVesselName != "") {
            otherTargetDisplayName = def->rootVesselName;
          }
          if (atLeastOneAlreadyInTheParens && (k < targets->size()))
            label.append(", ");
          label.append(otherTargetDisplayName);
          atLeastOneAlreadyInTheParens = true;
        } // end-if this isn't the current one  
      } // end-for each target
      label.append(")");
    } // end-if there are more targets defined
    else {
      label.append(" (no other targets have as yet been defined)");
    }
  }
  else { // the selected target is NULL, compose label accordingly
    label = "No current target focus, press ";
    if (targets->size() > 0) {
      label.append("Switch Target button to select from among ");
      bool atLeastOneAlreadyInTheParens = false;
      for (int k=0; k < targets->size(); k++) {
        EVTargetDef *def = &((*targets)[k]);
        QString otherTargetDisplayName = def->getID();
        // override the display name if it is a coronary target (because the greater branching complexity makes their screen labelling confusing if we don't do this)
        if (otherTargetDisplayName.contains("oronary") && def->rootVesselName != "") {
          otherTargetDisplayName = def->rootVesselName;
        }
        if (atLeastOneAlreadyInTheParens && (k < targets->size()))
          label.append(", ");
        label.append(otherTargetDisplayName);
        atLeastOneAlreadyInTheParens = true;
      } // end-for each target
      label.append(" (or press Create Target to add an additional one)");

    }
    else {
      label.append("Create Target button to make one");
    }
 
  }

  UpdateSenes();

  //this is on patient analyse
  //setCurrentTarget(currentTarget);

  GetEVLinkedViewers()->Render();
}

//void EVTargetDefine::acceptScreenControlFromAnalyze(QStackedWidget *seriesSelectionArea, EVImageSeries *series, TargetDef *def)
//{
//  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
//  selectCurrentTarget(def);
// // acceptScreenControlCommon(seriesSelectionArea, series);
//}
//
//void EVTargetDefine::acceptScreenControlFromSurvey(QStackedWidget *seriesSelectionArea, EVImageSeries *series)
//{
//  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
//  //acceptScreenControlCommon(seriesSelectionArea, series);
//}

void EVTargetDefine::CreateTargetPre(const QString& bodySite)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // compose a menu to get the root vessel name
  QList<QString> allowableVesselsGivenBodySite;

  if (bodySite == "Aorta") {
    allowableVesselsGivenBodySite = QList<QString>()
       << tr("Aorta")
       << tr("ThoracicAorta")
       << tr("AbdominalAorta")
       << tr("LeftCommonIliacArtery")
       << tr("RightCommonIliacArtery")
       ;
  } 
  else if (bodySite == "LeftCoronary") {
    allowableVesselsGivenBodySite = QList<QString>()
       << tr("MainStem")
       << tr("LeftAnteriorDescending")
       << tr("Circumflex")
       << tr("LeftCoronaryArtery")
       ;
  } 
  else if (bodySite == "RightCoronary") {
    allowableVesselsGivenBodySite = QList<QString>()
       << tr("RightCoronaryArtery")
       ;
  } 
  else if (bodySite == "LeftCarotid") {
    allowableVesselsGivenBodySite = QList<QString>()
       << tr("CommonCarotidArtery")
       << tr("InternalCarotidArtery")
       << tr("ExternalCarotidArtery")
       << tr("CarotidArtery")
       ;
  } 
  else if (bodySite == "RightCarotid") {
    allowableVesselsGivenBodySite = QList<QString>()
       << tr("CommonCarotidArtery")
       << tr("InternalCarotidArtery")
       << tr("ExternalCarotidArtery")
       << tr("CarotidArtery")
       ;
  } 
  else if ((bodySite == "LeftVertebral") || (bodySite == "RightVertebral")) {
    addVessel(tr("VertebralArtery"));
    return;
  } 
  else if ((bodySite == "LeftFemoral") || (bodySite == "RightFemoral")) {
    addVessel(tr("FemoralArtery"));
    return;
  } 
  else {
    addVessel(tr("NotSpecified"));
    return;
  }

  foreach (QString vessel, allowableVesselsGivenBodySite) {
    addRootVessel(vessel);
  }
}

void EVTargetDefine::addRootVessel(const QString& rootVesselName)
{
  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);

  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  currentTarget->rootVesselName = rootVesselName;
  addVessel(rootVesselName);
}

void EVTargetDefine::EstablishNewTarget(const QString& bodySite)
{
  ebLog eblog(Q_FUNC_INFO); eblog << bodySite.toStdString() << std::endl;

  QList<EVTargetDef>* targets = GetTargets();
  ebAssert(targets);
 

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (coronalType == ebvViewer::CPR) {
    // can't define a target when in oblique, need to get back to axial
    toggleOblique(false);
  }

  // allocate the new target def
  EVTargetDef *newTarget = new EVTargetDef();
  newTarget->setBodySite(bodySite); // bodySite stored by createVesselPre to pass to createVesselAction
  newTarget->rootVesselName = ""; // will be set later but need empty string now for display logic

  // we establish the ID of the target to be the body site, or if this same body site has already been seen we also append an index count.  
  QString newID(bodySite);
  int countOfTargetsWithThisBodySite = 1;
  for (int i=0; i < targets->size(); i++) {
    EVTargetDef *def = &((*targets)[i]);
    if (def->getBodySite() == bodySite)
      countOfTargetsWithThisBodySite++;
  }
  if (countOfTargetsWithThisBodySite > 1)
    newID.append(QString::number(countOfTargetsWithThisBodySite)); // this will make the IDs be assigned for example as LeftCarotid, LeftCarotid2, LeftCarotid3, etc., which should work for the typical cases where a given body site isn't usually re-sued (as in LeftCarotid) but also works when it is (e.g., PulmonaryLesion)
  newTarget->setID(newID);

  // now proceed to the rest of the fields
  newTarget->setTargetReadingsFileName(""); 
  newTarget->setLesionReadingsFileName(""); 
  newTarget->setTargetPathFileName("");
  newTarget->setTargetRegistrationTransforms("");
  newTarget->setParentPipeline(pipeline);

  newTarget->targetPipelineID = pipeline->AddVesselTargetPipeline();
  pipeline->GetVesselTargetPipeline(newTarget->targetPipelineID)->SetBodySite(bodySite.toStdString());
  //newTarget->parameters = new processingParameters(this, owner->systemPreferencesObject, owner->clinicalJurisdiction, pipeline, newTarget->getBodySite(), newTarget->targetPipelineID);
  //connect(newTarget->parameters, SIGNAL(processingParametersSettingsChanged()), this, SLOT(processingParametersSettingsChanged()));
  newTarget->pushTargetParametersToPipeline(pipeline->GetVesselTargetPipeline(newTarget->targetPipelineID));
  if (pipeline->GetVesselTargetPipeline(newTarget->targetPipelineID)->InitializationPreConditions()) {
    newTarget->unsavedChanges = true;

    //// setting the targetFolder is more involved due to the need to carefully construct it
    //workItemListEntry *wi = owner->getWorkItem(sessionItemIndex)->selectedWorkItemListEntryPtr;
    //if (!wi->setWorkItemFolder()) { // this may be the first time an operation that requires the folder has been done
    //  message->showMessage(tr("Error: Cannot write work item folder."));
    //  return;
    //}
    //if (!newTarget->setTargetFolder(wi->getWorkItemFolder())) {
    //  message->showMessage(tr("Error: Cannot write target folder."));
    //  return;
    //}

    targets->append(*newTarget);


    // here we make sure we have the screen, as the create may have originated from analyze.  That would be simple enough but for a catch: a similar 
    // sequence is used when the request is to modify (rather than create) the target; and in that case, we want analyze to set the current target as 
    // what is to be modified. But this doesn't work for create, since analyze doesn't have the newly created one at this stage of the sequence yet. So
    // need to select the current target as the new one after the transition.
   // owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
    selectCurrentTarget(&targets->last()); // the one we just appended
   // owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
   // qApp->processEvents();

    EVTargetDef* currentTarget = GetCurrentTarget();
    ebAssert(currentTarget);

    // now set up to collect points
    pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CreateInitialization();
    currentTarget->initializerIDdefine = GetEVLinkedViewers()->AddVesselTarget(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization(), "initializer");
  }
  else {
    PromptMessage(EV_MSG_ERROR_NEW_TARGET_MAY_NOT_BE_INITIALIZED_SKIPPING);
  }
  
  SetCurrentTarget(newTarget);

  EVTargetDef* currentTarget = GetCurrentTarget();
  if(currentTarget);
 
  pipeline->GetVesselTargetPipeline(newTarget->targetPipelineID)->SetTargetFolder(currentTarget->getTargetFolder().toStdString());
}

void EVTargetDefine::addVessel(QString startFromName)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // enter gesture mode to collect points for root vessel
  GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowVolume(true);
  GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, false);
  GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, false);
  GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, false);
  GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, false);
  GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
  GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, false);
  GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP2, false);

  //tools->disableToolButtons(); // almost everything needs to be disabled during this operation
  //tools->VolumeRenderTool->setChecked(true);

  //ENABLECONTROL(tools->OptimizeSubvolumeTool, tools->OptimizeSubvolumeAction, tr("Press to toggle widgets allowing optimization of subvolume"));

  //ENABLECONTROL(tools->TargetCreateTool, tools->TargetCreateAction, tr("Press to stop collecting points and process them"));
  //tools->TargetCreateTool->setChecked(true);

  //DISABLECONTROL(tools->TargetModifyTool, tools->TargetModifyAction, tr("TargetModify is disabled"));

  //ENABLECONTROL(tools->ToggleCoronalTool, tools->ToggleCoronalAction, tr("Press to toggle coronal plane"));
  //tools->ToggleCoronalTool->setChecked(false);
  //  
  //ENABLECONTROL(tools->ToggleSagittalTool, tools->ToggleSagittalAction, tr("Press to toggle sagittal plane"));
  //tools->ToggleSagittalTool->setChecked(false);
  //  
  //ENABLECONTROL(tools->ToggleAxialTool, tools->ToggleAxialAction, tr("Press to toggle axila plane"));
  //tools->ToggleAxialTool->setChecked(false);

  //ui->continueWithAnalysisButton->setEnabled(false); 
  //owner->getPatientAnalyze(sessionItemIndex)->disableMenuActions(); // propagate it downstream

  GetEVLinkedViewers()->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Sub-volume ready"));
  GetEVLinkedViewers()->SyncViewersToScene();
  GetEVLinkedViewers()->Render();
  // when adding rather than creating, proximal vessel is determined by distance rather than being explicitly given
  GetEVLinkedViewers()->StartVesselTargetInitialization(startFromName.toStdString());
  //ui->backToSurveyButton->setEnabled(false); // can't go back when in middle of operation
}

void EVTargetDefine::addDistalVessel()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);


  //owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
  //owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
  // clear out any existing computations and displays
  // Keep only the initializer. This is accomplished by closing the lumen (if it exists) and then flushing.
  if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation())
    pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLumenSegmentation(); 
  clearTargetBeyondCurrentStage(currentTarget);
  addVessel("");
}

void EVTargetDefine::completeDistalVessel()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;


  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);

  // prompt user for distal vessel name
  // compose a menu to get the root vessel name
 // QMenu menu(this);
  QList<QString> allowableDistalVesselsGivenRoot;

  if (currentTarget->rootVesselName == "Aorta") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("LeftCoronaryCusp")
       << tr("LeftCoronaryArtery")
       << tr("MainStem")
       << tr("RightCoronaryCusp")
       << tr("RightCoronaryArtery")
       << tr("NonCoronaryCusp")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "LeftCoronaryArtery") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("LeftAnteriorDescending")
       << tr("Circumflex")
       << tr("Ramus")
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "MainStem") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("LeftAnteriorDescending")
       << tr("Circumflex")
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "LeftAnteriorDescending") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("Diagonal")
       << tr("Diagonal1")
       << tr("Diagonal2")
       << tr("SeptalPerforator")
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "Circumflex") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("LeftMarginal")
       << tr("LeftMarginal1")
       << tr("LeftMarginal2")
       << tr("PosterioLateral")
       << tr("SinoAtrialNode")
       << tr("PosteriorDescending")
       << tr("AtrioventricularNodal")
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "RightCoronaryArtery") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("AtrioventricularNodal")
       << tr("SinoAtrialNode")
       << tr("RightMarginal")
       << tr("PosteriorDescending")
       << tr("PosterioLateral")
       << tr("SeptalPerforator")
       << tr("ConusBranch")
       << tr("Ventricular")
       << tr("AcuteMarginal")
       << tr("RightVentricular")
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "CoronaryArtery") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "CommonCarotidArtery") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("InternalCarotidArtery")
       << tr("ExternalCarotidArtery")
       << tr("NotSpecified")
       ;
  }
  else 
  {
    labelDistalVessel(tr("NotSpecified"));
    return;
  }

  foreach (QString vessel, allowableDistalVesselsGivenRoot) 
  {
    labelDistalVessel(vessel);
  }
}

void EVTargetDefine::labelDistalVessel(const QString& distalVesselName)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;


  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);


  // turn off the gesture mode (with distal vessel parameter) which causes the target data structures to be established
  GetEVLinkedViewers()->StopVesselTargetInitialization(distalVesselName.toStdString());
  PromptMessage(EV_MSG_ON_LABEL_DISTAL_VESSEL);
  // log the update and set the flag
  logUpdateWithStageSettings("targetDefine::labelDistalVessel (initializer)", currentTarget, currentTarget->getID());
  currentTarget->unsavedChanges = true;

}

bool EVTargetDefine::switchTargetPath()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  QList<EVTargetDef>* targets = GetTargets();
  ebAssert(targets);


  GetEVLinkedViewers()->GetScene()->IncrCurrentPathSetCursor(1);
  if (GetEVLinkedViewers()->GetScene()->IsVesselTargetSelected()) {
    //ENABLECONTROL(tools->ToggleObliqueTool, tools->ToggleObliqueAction, tr("Press to toggle slice viewer alignment"));
    //ENABLECONTROL(tools->MoveProximalTool, tools->MoveProximalAction, tr("Press to move proximally"));
    //ENABLECONTROL(tools->MoveDistalTool, tools->MoveDistalAction, tr("Press to move distally"));
    ebID id = GetEVLinkedViewers()->GetScene()->GetSelectedVesselTargetID();

    if ((currentTarget == NULL) 
        || ((id != currentTarget->initializerIDdefine) || (id != currentTarget->pathIDdefine) || (id != currentTarget->readingsIDdefine))) {
      // we not only switched paths, but targets as well.  Find the new one and switch to it.
      int j;
      for (j=0; j < targets->size(); j++) {
        if ((id == targets->at(j).initializerIDdefine) || (id == targets->at(j).pathIDdefine) || (id == targets->at(j).readingsIDdefine))
          break;
      }
      if (j < targets->size()) {
        selectCurrentTarget(&((*targets)[j]));
              ebAssert(GetEVLinkedViewers() && GetEVLinkedViewers()->GetScene() && GetEVLinkedViewers()->GetScene()->GetSelectedVesselTarget());
              GetEVLinkedViewers()->GetScene()->GetSelectedVesselTarget()->GetVesselTarget()->SetName(currentTarget->getID().toStdString());

        // if the initializer has no points, then get them
        if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()->InitializationPointsEmpty()) {
          //owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
          //owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
          CreateTargetPre(currentTarget->getBodySite());
          return false; // signifies that there had to be an initializer created
        }
        else if (!currentTarget->isViable) {
          // similar to case above but without going into the initializer point collecting sequence
          //owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
          //owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
        }

        // update buttons buttons now that there is a specific target in focus
        //ENABLECONTROL(tools->TargetDeleteTool, tools->TargetDeleteAction, tr("Press to delete current target"));
        //ENABLECONTROL(tools->TargetModifyTool, tools->TargetModifyAction, tr("Press to modify current target"));
        //tools->TargetModifyTool->setChecked(false);
        return true;
      }
      else {
        qWarning() << "can't find target id" << id << ", skipping.";
      }
    } // end-if there had been no prior selected target or if selected target has changed
    else { // there is no change to the target (possibly the path within the target changed, but no actual target switch)
      return true;
    }
  } // end-if there is a selected target
  // get here either if there is no selected target or if can't find the one that supposedly is
  selectCurrentTarget(NULL);
 // DISABLECONTROL(tools->ToggleObliqueTool, tools->ToggleObliqueAction, tr("ToggleOblique is disabled (select a target with a path to enable)"));
  //DISABLECONTROL(tools->MoveProximalTool, tools->MoveProximalAction, tr("MoveProximal is disabled (select a target with a path to enable)"));
  //DISABLECONTROL(tools->MoveDistalTool, tools->MoveDistalAction, tr("MoveDistal is disabled (select a target to enable)"));
  return false;
}

void EVTargetDefine::resetAllTargetsDueToChangesInImages()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
 
  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);
  
  QList<EVTargetDef>* targets = GetTargets();
  ebAssert(targets);


  //this->setEnabled(false);
  for (int i=0; i < targets->size(); i++) {
    EVTargetDef *def = &((*targets)[i]);
    // Keep only the initializer. This is accomplished by closing the lumen (if it exists) and then flushing.
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation())
      pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CloseLumenSegmentation(); 
    clearTargetBeyondCurrentStage(def);
    def->unsavedChanges = true;
    logUpdateWithStageSettings("EVTargetDefine::resetAllTargetsDueToChangesInImages", def, def->getID());
  }
  GetEVLinkedViewers()->SyncViewersToScene();
  GetEVLinkedViewers()->Render();
  //this->setEnabled(true);
}

void EVTargetDefine::resetAllTargetsDueToDifferentImages()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  QList<EVTargetDef>* targets  = GetTargets();
  ebAssert(targets);

  //this->setEnabled(false);
  if (targets->size() > 0) {
    // start by clearing everything but the initializers
    resetAllTargetsDueToChangesInImages();

    // now prompt to see if want to save the initializers before scrubbing
    //QMessageBox msgBox(this);
    //msgBox.setText(tr("Resetting targets due to difference in images."));
    //msgBox.setInformativeText(tr("Do you want to save the target initializers, so they are available after the reset?"));
    //msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    //msgBox.setDefaultButton(QMessageBox::Yes);
    //msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    //int ret = msgBox.exec();
    for (int i=0; i < targets->size(); i++) {
      EVTargetDef *def = &((*targets)[i]);
    //  if (ret == QMessageBox::Yes) {
    //    owner->save();
    //  }
      // now close the initializers and delete the pipeline
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization())
        pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CloseInitialization();  
      clearTargetBeyondCurrentStage(def);
      pipeline->RemoveVesselTargetPipeline(def->targetPipelineID);
      def->targetPipelineID = null_ebID;
    }

    // finally, select images again and re-load
    pipeline->SelectImages();
    preloadDefinePost();
  }
  //this->setEnabled(true);
}

void EVTargetDefine::deleteTarget()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  
  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);
  
  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  QList<EVTargetDef>* targets = GetTargets();
  ebAssert(targets);

  // this->setEnabled(false);
  if (currentTarget != NULL) {
    // determine whether there are unsaved changes and seek user confirmation if so
    if (currentTarget->unsavedChanges) {
      //QMessageBox msgBox(this);
      //msgBox.setText(tr("There are unsaved changes which will be lost if the target is deleted."));
      //msgBox.setInformativeText(tr("Do you want to save these changes before deleting?"));
      //msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      //msgBox.setDefaultButton(QMessageBox::No);
      //msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
      //int ret = msgBox.exec();
      //if (ret == QMessageBox::Yes) {
      //  owner->save();
      //}
    }

    // start the deletion by removing all displays for the target. This is accomplished by closing the initializer and then flushing.
    //owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
    //owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
    //owner->getPatientAnalyze(sessionItemIndex)->removeAllTargetDisplays();
    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization())
      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseInitialization(); 
    clearTargetBeyondCurrentStage(currentTarget);
    pipeline->RemoveVesselTargetPipeline(currentTarget->targetPipelineID);
    currentTarget->targetPipelineID = null_ebID;
    logUpdateWithStageSettings("EVTargetDefine::deleteTarget", currentTarget, currentTarget->getID());
    GetEVLinkedViewers()->SyncViewersToScene();
    if (currentTarget->isViable)
      --viableTargetsCount;
    // now that it is out of the displays, remove it from the targets list
    for (int i=0; i < targets->size(); i++) {
      if (&(targets->at(i)) == currentTarget) {
        targets->removeAt(i);
      }
    }
    currentTarget = NULL; // since we just deleted it!
    selectCurrentTarget(NULL); // to catch up all the labeling etc.

    // finally, perform a switch target path
    switchTargetPath();
    GetEVLinkedViewers()->SyncViewersToScene();
    GetEVLinkedViewers()->Render();
  }
  else
  {
    //message->showMessage(tr("Program logic error; can't remove a target unless it is current, yet current is NULL"));

  }
}

/*void EVTargetDefine::renameTarget()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  bool ok;
  QString name = QInputDialog::getText(this, tr("Rename Current Target"), tr("ID:"), QLineEdit::Normal, currentTarget->getID(), &ok);
  if (ok) {
    currentTarget->setID(name);
    pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetPath()->SetName(name.toStdString()); 
    currentTarget->unsavedChanges = true;
    QString step = "EVTargetDefine::renameTarget"; // log the update
    emit logUpdate(step, sessionItemIndex);
  }
}*/

void EVTargetDefine::resetWall()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  
  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->RegistrationPreConditions()) {
    // first reset the pipeline if necessary, i.e., if a prior lumen had existed
    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetResampledRegisteredImages()) {
      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseRegistration();
    }
    clearTargetBeyondCurrentStage(currentTarget);
  }
  GetEVLinkedViewers()->SyncViewersToScene();
  GetEVLinkedViewers()->Render();
}

void EVTargetDefine::resetTarget()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  EVTargetDef* currentTarget = GetCurrentTarget();
  ebAssert(currentTarget);

  ebiVesselPipeline* pipeline = GetVesselPipeine();
  ebAssert(pipeline);

  if (currentTarget != NULL) {
    //this->setEnabled(false);
    // determine whether there are unsaved changes and ask whether user wants to save them if so if so
    if (currentTarget->unsavedChanges) {
      //QMessageBox msgBox(this);
      //msgBox.setText(tr("There are unsaved changes which will be lost if the target is reset."));
      //msgBox.setInformativeText(tr("Do you want to save these changes before deleting?"));
      //msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      //msgBox.setDefaultButton(QMessageBox::No);
      //msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
      //int ret = msgBox.exec();
      //if (ret == QMessageBox::Yes) {
      //  owner->save();
      //}
    }

    // close the old initializer
    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()) {
      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseInitialization();  
    }
    clearTargetBeyondCurrentStage(currentTarget);
    GetEVLinkedViewers()->SyncViewersToScene();
    GetEVLinkedViewers()->Render();

    // now start the sequence for the new initializer
    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->InitializationPreConditions()) {
      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CreateInitialization();
      currentTarget->initializerIDdefine = GetEVLinkedViewers()->AddVesselTarget(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization(), "initializer");
      CreateTargetPre(currentTarget->getBodySite());
    }
    else
    {
      PromptMessage(EV_MSG_PIPELN_STATE_OUT_OF_SYNC);
    }

    logUpdateWithStageSettings("EVTargetDefine::resetTarget", currentTarget, currentTarget->getID());
    //this->setEnabled(true);
  }
  else
  {
    // message->showMessage(tr("Program logic error; can't remove a target unless it is current, yet current is NULL"));
  }
}

void EVTargetDefine::toggleOblique(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << (checked ? "OBLIQUE/CPR" : "AXIS-ALIGNED") << std::endl;
 // if (owner->getWorkItemProcess(sessionItemIndex)->currentIndex() != TARGETDEFINE) {
 //   return;
 // }
  GetEVLinkedViewers()->RemoveViewer(ebvIdAxial);
  GetEVLinkedViewers()->RemoveViewer(ebvIdCoronal);
  if (checked) {
    // make it oblique
    axialType = ebvViewer::OBLIQUE;
    //ebvIdAxial = viewers->AddViewer(axialType,ui->axialRender->GetRenderWindow());
    GetEVLinkedViewers()->GetViewer(ebvIdAxial)->SetShowLogo(true);
    coronalType = ebvViewer::CPR;
    ///ebvIdCoronal = viewers->AddViewer(coronalType,ui->coronalRender->GetRenderWindow());
    ebAssert(GetEVLinkedViewers()->GetViewerCPR(ebvIdCoronal));
    GetEVLinkedViewers()->GetViewer(ebvIdCoronal)->SetShowLogo(true);
    GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, false);
    GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, false);
    GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, false);

    //if (tools->ToggleAxialTool->isChecked())
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, tools->ToggleAxialTool->isChecked());
    //else
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, false);
    //if (tools->ToggleCoronalTool->isChecked())
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, tools->ToggleCoronalTool->isChecked());
    //else
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
    //if (tools->ToggleSagittalTool->isChecked())
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, tools->ToggleSagittalTool->isChecked());
    //else
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, false);
  }
  else {
    // make it aligned
    axialType = ebvViewer::AXIAL;
   // ebvIdAxial = viewers->AddViewer(axialType,ui->axialRender->GetRenderWindow());
    GetEVLinkedViewers()->GetViewer(ebvIdAxial)->SetShowLogo(true);
    coronalType = ebvViewer::CORONAL;
    //ebvIdCoronal = viewers->AddViewer(coronalType,ui->coronalRender->GetRenderWindow());
    GetEVLinkedViewers()->GetViewer(ebvIdCoronal)->SetShowLogo(true);
    //if (tools->ToggleAxialTool->isChecked())
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, tools->ToggleAxialTool->isChecked());
    //else
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, false);
    //if (tools->ToggleCoronalTool->isChecked())
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, tools->ToggleCoronalTool->isChecked());
    //else
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, false);
    //if (tools->ToggleSagittalTool->isChecked())
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, tools->ToggleSagittalTool->isChecked());
    //else
    //  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, false);
    GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, false);
    GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
    GetEVLinkedViewers()->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, false);
  }
  //tools->ToggleObliqueTool->setChecked(checked);
  GetEVLinkedViewers()->SyncViewersToScene();
  GetEVLinkedViewers()->Render();
}

void EVTargetDefine::giveToAnalyzeIfNotAlreadyThere()
{
  ////ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  //if (ui->seriesSelectionArea->count() != 0) // if not 0, then the screen is currently on EVTargetDefine, so must give it to patientAnalyze
  //  on_continueWithAnalysisButton_clicked();

}

void EVTargetDefine::toggleSegmentation(QString regName, bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QList<EVTargetDef>* targets = GetTargets();
  ebAssert(targets);

  // loop through targets, because the segmentation toggles effect all of them
  for (int i=0; i < targets->size(); i++) {
    // go through multiple levels of check to see if the specification is complete enough to add
    EVTargetDef *def = &((*targets)[i]);
    // have to do this in layers, since partitions override segmentations
    if (regName.contains("lumen")) {
      if (def->lumenSegIDdefine != null_ebID)
        GetEVLinkedViewers()->GetScene()->GetSegmentation4(def->lumenSegIDdefine)->SetVisibility(checked);
      if (!def->lumenPartIDsDefine.empty()) {
        for (auto partitionID : def->lumenPartIDsDefine) {
          GetEVLinkedViewers()->GetScene()->GetSegmentation4(partitionID)->SetVisibility(checked);
        }
      }
    }
    if (regName.contains("wall")) {
      if (def->wallSegIDdefine != null_ebID) 
        GetEVLinkedViewers()->GetScene()->GetSegmentation4(def->wallSegIDdefine)->SetVisibility(checked);
      if (!def->wallPartIDsDefine.empty()) {
        for (auto partitionID : def->wallPartIDsDefine) {
          GetEVLinkedViewers()->GetScene()->GetSegmentation4(partitionID)->SetVisibility(checked);
        }
      }
    }
  } // end-for each listed target
  GetEVLinkedViewers()->SyncViewersToScene();
  GetEVLinkedViewers()->Render();
}
