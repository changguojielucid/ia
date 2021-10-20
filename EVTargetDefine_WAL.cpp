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

void EVTargetDefine::evaluateWall()
{
	ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

	EVTargetDef* currentTarget = GetCurrentTarget();
	ebAssert(currentTarget);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

  EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
  ebAssert(pEVLineedViewers);

	if ((currentTarget != NULL)
		&& (!pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()->InitializationPointsEmpty())) {
		//this->setEnabled(false);
		// start by seeing if have to do lumen first
		if ((pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenSegmentationPreConditions())
			&& (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation() == NULL)) {
			//QMessageBox msgBox(this);
			//msgBox.setText(tr("A lumen is needed before computing the wall, and preconditions to compute it are met."));
			//msgBox.setInformativeText(tr("Do you want to compute the lumen now and go straight through to the wall?"));
			//msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
			//msgBox.setDefaultButton(QMessageBox::Yes);
			//msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
			//int ret = msgBox.exec();
			//if (ret == QMessageBox::Yes) 
			//  evaluateLumen();
			//else {
			//  //this->setEnabled(true);
			//  return;
			//}
		}

		if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->RegistrationPreConditions()) {
			// first reset the pipeline if necessary, i.e., if a prior lumen had existed
			if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetResampledRegisteredImages())
				pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseRegistration();
			clearTargetBeyondCurrentStage(currentTarget);

			// display a dialog for progress
			//QITKProgressDialog progressIndicator(0,0);
			//progressIndicator.setWindowModality(Qt::NonModal);
			//progressIndicator.setMinimumDuration(10);
			//progressIndicator.setWindowFlags(progressIndicator.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::WindowStaysOnTopHint);
			//progressIndicator.setCancelButton(nullptr);   // no cancel button on dialog
			//progressIndicator.show();
			//progressIndicator.AddFilter(5,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetRegistrationFilter(),0.01);
			//progressIndicator.AddFilter(6,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentationFilter(),3);
			//progressIndicator.AddFilter(7,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallPartitionFilter(),0.5);
			//this->repaint(); // ensure progress is shown
			//qApp->processEvents();

			UpdateSenes();
			// and proceed step-wise with the task
      pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Registering Images"));
      pEVLineedViewers->SyncViewersToScene();
      pEVLineedViewers->Render();
			try {
				pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeRegistration();
			}
			catch (std::exception& e) {
				eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
				//QMessageBox::warning(this, tr("ebException computing"), QString(tr("object, suggest resetting target")));
				//this->setEnabled(true);
				return;
			}
			logUpdateWithStageSettings("EVTargetDefine::ComputeRegistration", currentTarget, currentTarget->getID());
			currentTarget->unsavedChanges = true;

			if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenAndWallSegmentationPreConditions()) {
        pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Delineating Outer Wall Surface"));
        pEVLineedViewers->SyncViewersToScene();
        pEVLineedViewers->Render();
				try {
					pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenAndWallSegmentation();
				}
				catch (std::exception& e) {
					eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
					PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
					//this->setEnabled(true);
					return;
				}
				logUpdateWithStageSettings("EVTargetDefine::ComputeLumenAndWallSegmentation", currentTarget, currentTarget->getID());
				currentTarget->unsavedChanges = true;
				currentTarget->wallSegIDdefine = pEVLineedViewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation(), 0, "wallSegmentation");
        pEVLineedViewers->GetScene()->GetSegmentation4(currentTarget->wallSegIDdefine)->GetInteriorProperty()->SetOpacity(0);
				clearTargetBeyondCurrentStage(currentTarget);
        pEVLineedViewers->SyncViewersToScene();
        pEVLineedViewers->Render();

				if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenAndWallPartitionPreConditions()) {
          pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Partitioning Wall For Specific Vessels"));
          pEVLineedViewers->SyncViewersToScene();
          pEVLineedViewers->Render();
					try {
						pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenAndWallPartition();
					}
					catch (std::exception& e) {
						eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
						/* message->showMessage(tr("Pipeline state does not allow registration, skipping.")); this, tr("Error computing"), QString(tr(PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);)));
				   this->setEnabled(true);*/
						return;
					}
					logUpdateWithStageSettings("EVTargetDefine::ComputeLumenAndWallPartition", currentTarget, currentTarget->getID());
					currentTarget->unsavedChanges = true;
          pEVLineedViewers->RemoveSegmentation4(currentTarget->wallSegIDdefine); // if have partitions, don't want to display the segmentation too
					currentTarget->wallSegIDdefine = null_ebID;
					currentTarget->wallPartIDsDefine.clear();
					int i = 0;
					for (auto partition : *pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallPartition()) {
						ebID partID = pEVLineedViewers->AddSegmentation4(partition.GetPointer(), 0, "wallPartition" + std::to_string(i++));
						currentTarget->wallPartIDsDefine.insert(partID);
            pEVLineedViewers->GetScene()->GetSegmentation4(partID)->GetInteriorProperty()->SetOpacity(0);
					}
					clearTargetBeyondCurrentStage(currentTarget);
          pEVLineedViewers->SyncViewersToScene();
          pEVLineedViewers->Render();
				} // end-if can can compute partitions
				else
					qWarning() << "evaluate wall was not able to make it all the way through to the local wall.";

        pEVLineedViewers->GetViewer(ebvIdVolume)->SetTemporaryText("");
        pEVLineedViewers->SyncViewersToScene();
        pEVLineedViewers->Render();

				currentTarget->unsavedChanges = true;

				////ENABLECONTROL(tools->ToggleWallTool, tools->ToggleWallAction, tr("Press to toggle wall display"));
				//tools->ToggleWallTool->setChecked(true);
				//tools->editSegmentationEnableState = true;
				//tools->editSegmentationActionLabel = tr("Edit Wall");

				//ui->continueWithAnalysisButton->setEnabled(true); 
				//owner->getPatientAnalyze(sessionItemIndex)->enableMenuActions(); // propagate it downstream

				//progressIndicator.setMaximum(100); // any value will take it down
			} // end-if can compute wall  
			else
			{
				//message->showMessage(tr("Pipeline state does not allow to compute wall, skipping."));
			}
		} // end-if can do registration
		else
		{
			//message->showMessage(tr("Pipeline state does not allow registration, skipping."));
		}

	}
	else
	{
		//message->showMessage(tr("Warning: Need a current target focus with completed initializer, skipping."));
	}
}
