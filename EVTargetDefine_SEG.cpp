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

#ifndef tr
	#define tr  
#endif //!tr
 
void EVTargetDefine::editSegmentation(bool checked)
{
	//ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
	//if ((currentTarget != NULL) 
	//  && (!pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()->InitializationPointsEmpty())) {
	//   UpdateSenes();
	//  if (checked) {
	//    // start by swapping the volume and axial viewers, making the large view a slice view
	//    savedParallelScale = viewers->GetViewer2D(ebvIdAxial)->GetParallelScale();
	//    savedPerspectiveDistance = viewers->GetViewer3D(ebvIdVolume)->GetPerspectiveDistance();
	//    viewers->RemoveViewer(ebvIdAxial);
	//    viewers->RemoveViewer(ebvIdVolume);
	//    ebvIdVolume = viewers->AddViewer((tools->ToggleObliqueTool->isChecked() ? ebvViewer::OBLIQUE : ebvViewer::AXIAL),ui->volumeRender->GetRenderWindow());
	//    viewers->GetViewer(ebvIdVolume)->SetShowLogo(true);
	//    tools->resetViewer(ebvIdVolume, ebvViewer::AXIAL);
	//    ebvIdAxial = viewers->AddViewer(ebvViewer::THREED,ui->axialRender->GetRenderWindow());
	//    viewers->GetViewer(ebvIdAxial)->SetShowLogo(true);
	//    viewers->GetViewer3D(ebvIdAxial)->SetShowVolume(false);
	//    viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::AXIAL, false);
	//    viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::CORONAL, false);
	//    viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::SAGITTAL, false);
	//    viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::OBLIQUE, false);
	//    viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
	//    viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::CPR, tools->ToggleObliqueTool->isChecked());
	//    viewers->PanCamerasToCursor(true, true);
	//    savedSlabThickness = viewers->GetScene()->GetSlabThickness();
	//    viewers->GetScene()->SetSlabThickness(0);
	//    viewers->UpdateCameras();

	//    double window, level;
	//    ebAssert(viewers && viewers->GetScene() && viewers->GetScene()->GetSelectedImage4());
	//    viewers->GetScene()->GetSelectedImage4()->GetWindowLevel(window, level);
	//    viewers->GetViewer2D(ebvIdVolume)->SetParallelScale(.5*savedParallelScale);
	//    currentBackingSeries->window = window;
	//    currentBackingSeries->level = level;
	//    viewers->GetScene()->GetSelectedImage4()->SetWindowLevel(currentBackingSeries->window, currentBackingSeries->level);

	//    viewers->GetViewer3D(ebvIdAxial)->SetPerspectiveDistance(.3*savedPerspectiveDistance);
	//    ui->axialLabel->setText("Surface Rendering");
	//    // start by seeing if it is wall or lumen which will be edited
	//    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation() != NULL) {
	//      currentTarget->wallSegIDdefine = viewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation(), 0, "wallSegmentation");
	//            viewers->GetScene()->GetSegmentation4(currentTarget->wallSegIDdefine)->GetInteriorProperty()->SetOpacity(0);  
	//      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseCapThickness(); 
	//      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseComposition(); 
	//      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseWallThickness(); 
	//      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLumenAndWallPartition();
	//      clearTargetBeyondCurrentStage(currentTarget); // this will take down the partitions and anything else downstream
	//      logUpdateWithStageSettings("EVTargetDefine::EditLumenAndWallSegmentationStart", currentTarget, currentTarget->getID());
	//      viewers->SyncViewersToScene();
	//      viewers->Render();
	//      viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Editing mode on").toStdString());
	//            viewers->StartSegmentationEditor();
	//      editingSegmentation = true;
	//            segmentationEditor->show();
	//      tools->editSegmentationActionLabel = tr("Exit editing mode");
	//      editingWall = true;
	//      ui->backToSurveyButton->setEnabled(false); // can't go back when in middle of operation
	//    }
	//    else if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation() != NULL) {
	//      currentTarget->lumenSegIDdefine = viewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation(), 0, "lumenSegmentation");
	//      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLumenPartition();
	//      clearTargetBeyondCurrentStage(currentTarget); // this will take down the partitions and anything else downstream
	//      //pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()->SetVisibility(false); // since took off the path, the initializer would display but don't want it to obscure the edit
	//      if (currentTarget->initializerIDdefine) {
	//        viewers->RemoveVesselTarget(currentTarget->initializerIDdefine); // if have a path, don't want to display the initializer too
	//        currentTarget->initializerIDdefine = null_ebID;
	//      }
	//      logUpdateWithStageSettings("EVTargetDefine::EditLumenSegmentationStart", currentTarget, currentTarget->getID());
	//      viewers->SyncViewersToScene();
	//      viewers->Render();
	//      viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Editing mode on").toStdString());
	//            viewers->StartSegmentationEditor();
	//            segmentationEditor->show();
	//      tools->editSegmentationActionLabel = tr("Exit editing mode");
	//      editingWall = false;
	//      ui->backToSurveyButton->setEnabled(false); // can't go back when in middle of operation
	//    }
	//    else {
	//      qWarning() << "no lumen or wall to edit.";
	//    }
	//    //DISABLECONTROL(tools->ToggleCoronalTool, tools->ToggleCoronalAction, tr("ToggleCoronal is disabled during editing"));
	//    //DISABLECONTROL(tools->ToggleSagittalTool, tools->ToggleSagittalAction, tr("ToggleSagittal is disabled during editing"));
	//    //DISABLECONTROL(tools->ToggleAxialTool, tools->ToggleAxialAction, tr("ToggleAxial is disabled during editing"));
	//    //DISABLECONTROL(tools->ToggleLumenTool, tools->ToggleLumenAction, tr("Toggle lumen display is disabled during editing"));
	//    //DISABLECONTROL(tools->OptimizeSubvolumeTool, tools->OptimizeSubvolumeAction, tr("Optimization of subvolume is disabled during editing"));
	//    //DISABLECONTROL(tools->AnnotateTool, tools->AnnotateAction, tr("Annotation is disabled during editing"));
	//    //DISABLECONTROL(tools->TargetPathSwitchTool, tools->TargetPathSwitchAction, tr("Switch target path is disabled during editing"));
	//    //DISABLECONTROL(tools->TargetDeleteTool, tools->TargetDeleteAction, tr("Delete target is disabled during editing"));
	//    //DISABLECONTROL(tools->TargetCreateTool, tools->TargetCreateAction, tr("Create target is disabled during editing"));
	//    //DISABLECONTROL(tools->ToggleObliqueTool, tools->ToggleObliqueAction, tr("Toggle slice viewer alignment is disabled during editing"));
	//    //DISABLECONTROL(tools->VolumeRenderTool, tools->VolumeRenderAction, tr("Volume rendering is disabled during editing"));
	//    //DISABLECONTROL(tools->ShowAsMovieTool, tools->ShowAsMovieAction, tr("Show as movie is disabled during editing"));
	//    //DISABLECONTROL(tools->MoveProximalTool, tools->MoveProximalAction, tr("MoveProximal is disabled during editing"));
	//    //DISABLECONTROL(tools->MoveDistalTool, tools->MoveDistalAction, tr("MoveDistal is disabled during editing"));
	//    //ui->processingParametersButton->setEnabled(false); 
	//    // take down the parameter setting dialog if it may be up, because can't change paramters during editing
	//    //if (currentTarget->parameters != NULL)
	//    //  currentTarget->parameters->cancelParameterSetting();
	//  }
	//  else { // stop editing and re-compute partitions
	//    logUpdateWithStageSettings("EVTargetDefine::EditSegmentationStop", currentTarget, currentTarget->getID());
	//    viewers->StopSegmentationEditor();
	//    editingSegmentation = false;
	//    segmentationEditor->hide();
	//    currentTarget->unsavedChanges = true;
	//    this->setEnabled(false);
	//    ui->backToSurveyButton->setEnabled(true); // can go back now that operation is done
	//    // put viewers back to the default state
	//    viewers->RemoveViewer(ebvIdAxial);
	//    viewers->RemoveViewer(ebvIdVolume);
	//    ebvIdVolume = viewers->AddViewer(ebvViewer::THREED,ui->volumeRender->GetRenderWindow());
	//    viewers->GetViewer(ebvIdVolume)->SetShowLogo(true);
	//    tools->resetViewer(ebvIdVolume, ebvViewer::THREED);
	//    viewers->GetViewer3D(ebvIdVolume)->SetShowVolume(false);
	//    if (tools->ToggleObliqueTool->isChecked()) {
	//      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, false);
	//      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, false);
	//      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, false);
	//      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, tools->ToggleAxialTool->isChecked());
	//      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, tools->ToggleCoronalTool->isChecked());
	//      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, tools->ToggleSagittalTool->isChecked());
	//    }
	//    else {
	//      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, tools->ToggleAxialTool->isChecked());
	//      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, tools->ToggleCoronalTool->isChecked());
	//      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, tools->ToggleSagittalTool->isChecked());
	//      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, false);
	//      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
	//      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, false);
	//    }
	//    //ENABLECONTROL(tools->ToggleCoronalTool, tools->ToggleCoronalAction, tr("Press to toggle coronal plane"));
	//    //ENABLECONTROL(tools->ToggleSagittalTool, tools->ToggleSagittalAction, tr("Press to toggle sagittal plane"));
	//    //ENABLECONTROL(tools->ToggleAxialTool, tools->ToggleAxialAction, tr("Press to toggle axial plane"));
	//    //ENABLECONTROL(tools->ToggleLumenTool, tools->ToggleLumenAction, tr("Press to toggle lumen display"));
	//    //ENABLECONTROL(tools->OptimizeSubvolumeTool, tools->OptimizeSubvolumeAction, tr("Press to toggle widgets allowing optimization of subvolume"));
	//    //ENABLECONTROL(tools->AnnotateTool, tools->AnnotateAction, tr("Press to annotate"));
	//    //if (targets->size() >= 1) {
	//    //  ENABLECONTROL(tools->TargetPathSwitchTool, tools->TargetPathSwitchAction, tr("Press to switch target path"));
	//    //}
	//    //else {
	//    //  DISABLECONTROL(tools->TargetPathSwitchTool, tools->TargetPathSwitchAction, tr("Switch target path is disabled"));
	//    //}
	//    //ENABLECONTROL(tools->TargetCreateTool, tools->TargetCreateAction, tr("Press to create a new target"));
	//    //ENABLECONTROL(tools->TargetDeleteTool, tools->TargetDeleteAction, tr("Press to delete current target"));
	//    //ENABLECONTROL(tools->TargetModifyTool, tools->TargetModifyAction, tr("Press to modify current target"));
	//    //ENABLECONTROL(tools->ToggleObliqueTool, tools->ToggleObliqueAction, tr("Press to toggle slice viewer alignment"));
	//    //ENABLECONTROL(tools->VolumeRenderTool, tools->VolumeRenderAction, tr("Press to toggle volume rendering"));
	//    //ENABLECONTROL(tools->ShowAsMovieTool, tools->ShowAsMovieAction, tr("Press to show as movie"));
	//    //ENABLECONTROL(tools->MoveProximalTool, tools->MoveProximalAction, tr("Press to move proximally"));
	//    //ENABLECONTROL(tools->MoveDistalTool, tools->MoveDistalAction, tr("Press to move distally"));
	//    //ui->processingParametersButton->setEnabled(true); 
	//    //ebvIdAxial = viewers->AddViewer((tools->ToggleObliqueTool->isChecked() ? ebvViewer::OBLIQUE : ebvViewer::AXIAL),ui->axialRender->GetRenderWindow());
	//    //viewers->GetViewer(ebvIdAxial)->SetShowLogo(true);
	//    ////viewers->GetViewer2D(ebvIdAxial)->SetSlabLineVisible(0,false);
	//    viewers->GetScene()->SetImageSlabType(VTK_IMAGE_SLAB_MAX);
	//    viewers->GetScene()->SetSlabThickness(savedSlabThickness);
	//    viewers->UpdateCameras();
	//    ebAssert(viewers && viewers->GetScene() && viewers->GetScene()->GetSelectedImage4());
	//    viewers->GetScene()->GetSelectedImage4()->GetWindowLevel(currentBackingSeries->window, currentBackingSeries->level);
	//    viewers->GetScene()->GetSelectedImage4()->SetWindowLevel(currentBackingSeries->window, currentBackingSeries->level);
	//    viewers->GetViewer2D(ebvIdAxial)->SetParallelScale(savedParallelScale);
	//    viewers->GetViewer3D(ebvIdVolume)->SetPerspectiveDistance(savedPerspectiveDistance);
	//    ui->axialLabel->setText(tr("Maximum-Intensity Projection enabled (set slab thickness as desired)"));
	//    viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Editing mode off").toStdString());
	//    if (editingWall) {
	//      auto newLumenAndWall = LevelSetOr(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation(), pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation());
	//      ImageCopyPixels(newLumenAndWall.GetPointer(), pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation());
	//      LevelSetReinit(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation(),20);
	//      if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenAndWallPartitionPreConditions()) {
	//        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Paritioning Wall For Specific Vessels").toStdString());
	//        viewers->SyncViewersToScene();
	//        viewers->Render();
	//        try {
	//          pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenAndWallPartition();
	//        } catch (std::exception &e) {
	//          eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
	//          QMessageBox::warning(this, tr("Error computing"), QString(tr(PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);)));
	//          ui->continueWithAnalysisButton->setEnabled(true);
	//          ui->backToSurveyButton->setEnabled(true);
	//          this->setEnabled(true);
	//          return; 
	//        }
	//        logUpdateWithStageSettings("EVTargetDefine::ComputeLumenAndWallPartition", currentTarget, currentTarget->getID());
	//        currentTarget->unsavedChanges = true;
	//        if (currentTarget->wallSegIDdefine != null_ebID)
	//          viewers->RemoveSegmentation4(currentTarget->wallSegIDdefine); // if have partitions, don't want to display the segmentation too
	//        currentTarget->wallSegIDdefine = null_ebID;
	//        currentTarget->wallPartIDsDefine.clear();
	//        int i = 0;
	//        for (auto partition : *pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallPartition()) {
	//          ebID partID = viewers->AddSegmentation4(partition.GetPointer(),0,"wallPartition"+std::to_string(i++));
	//          currentTarget->wallPartIDsDefine.insert(partID);
	//          viewers->GetScene()->GetSegmentation4(partID)->GetInteriorProperty()->SetOpacity(0);   
	//        }
	//        clearTargetBeyondCurrentStage(currentTarget);
	//        tools->editSegmentationActionLabel = tr("Edit Wall");
	//        viewers->SyncViewersToScene();
	//        viewers->Render();
	//      } // end-if can can compute partitions
	//      else
	//        qWarning() << "evaluate wall was not able to re-partition.";
	//    }
	//    else {
	//      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ClosePath();
	//      if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->PathPreConditions()) {
	//        // re-compute new path
	//        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Computing Detailed Cross-section Positions").toStdString());
	//        viewers->SyncViewersToScene();
	//        viewers->Render();
	//        try {
	//          pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputePath();
	//        } catch (std::exception &e) {
	//          eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
	//          QMessageBox::warning(this, tr("Error computing"), QString(tr(PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);)));
	//          ui->continueWithAnalysisButton->setEnabled(true);
	//          ui->backToSurveyButton->setEnabled(true);
	//          this->setEnabled(true);
	//          return; 
	//        }
	//        logUpdateWithStageSettings("EVTargetDefine::ComputeVesselTargetPath", currentTarget, currentTarget->getID());
	//        currentTarget->unsavedChanges = true;
	//        if (currentTarget->initializerIDdefine) {
	//          viewers->RemoveVesselTarget(currentTarget->initializerIDdefine); // if have a path, don't want to display the initializer too
	//          currentTarget->initializerIDdefine = null_ebID;
	//        }
	//        currentTarget->pathIDdefine = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetPath(), "path"); 
	//        clearTargetBeyondCurrentStage(currentTarget);

	//        // re-compute new partition
	//        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Partitioning Lumen into Vessels").toStdString());
	//        viewers->SyncViewersToScene();
	//        viewers->Render();
	//        try {
	//          pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenPartition();
	//        } catch (std::exception &e) {
	//          eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
	//          QMessageBox::warning(this, tr("Error computing"), QString(tr(PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);)));
	//          ui->continueWithAnalysisButton->setEnabled(true);
	//          ui->backToSurveyButton->setEnabled(true);
	//          this->setEnabled(true);
	//          return; 
	//        }
	//        logUpdateWithStageSettings("EVTargetDefine::ComputeLumenPartition", currentTarget, currentTarget->getID());
	//        currentTarget->unsavedChanges = true;
	//        if (currentTarget->lumenSegIDdefine != null_ebID)
	//          viewers->RemoveSegmentation4(currentTarget->lumenSegIDdefine); // if have partitions, don't want to display the segmentation too
	//        currentTarget->lumenSegIDdefine = null_ebID;
	//        int i = 0;
	//        for (auto partition : *pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenPartition()) {
	//          currentTarget->lumenPartIDsDefine.insert(viewers->AddSegmentation4(partition.GetPointer(), 0, "lumenPartition"+std::to_string(i++)));
	//        }
	//        clearTargetBeyondCurrentStage(currentTarget);

	//        // sync up the display
	//        tools->editSegmentationActionLabel = tr("Edit Lumen");
	//        viewers->SyncViewersToScene();
	//        viewers->Render();
	//      } // end-if can can compute global lumen
	//      else {
	//        qWarning() << "evaluate lumen was not able to re-partition.";
	//      }
	//    }
	//    ui->continueWithAnalysisButton->setEnabled(true);
	//    ui->backToSurveyButton->setEnabled(true);
	//  }

	//  viewers->SyncViewersToScene();
	//  viewers->Render(); 
	//  this->setEnabled(true);
	//}
}
