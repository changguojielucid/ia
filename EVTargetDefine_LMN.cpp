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


void EVTargetDefine::evaluateLumen()
{
	ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

	EVTargetDef* currentTarget = GetCurrentTarget();
	ebAssert(currentTarget);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	// proceed if there is a current target
	if ((currentTarget != NULL)
		&& (!pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()->InitializationPointsEmpty())) {
		//this->setEnabled(false);
		if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenSegmentationPreConditions()) {
			// first reset the pipeline if necessary, i.e., if a prior lumen had existed
			if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation())
				pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLumenSegmentation();
			clearTargetBeyondCurrentStage(currentTarget);

			//QITKProgressDialog progressIndicator(0,0);
			//progressIndicator.setWindowModality(Qt::NonModal);
			//progressIndicator.setMinimumDuration(10);
			//progressIndicator.setWindowFlags(progressIndicator.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::WindowStaysOnTopHint);
			//progressIndicator.setCancelButton(nullptr);   // no cancel button on dialog
			//progressIndicator.show();
			//progressIndicator.AddFilter(2,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentationFilter(),2);
			//progressIndicator.AddFilter(3,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetPathFilter(),1);
			//progressIndicator.AddFilter(4,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenPartitionFilter(),0.5);
			//this->repaint(); // ensure progress is shown
			//qApp->processEvents();


			UpdateSenes();
			// and proceed step-wise with the task
			GetEVLinkedViewers()->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Delineating Luminal Surface"));
			GetEVLinkedViewers()->SyncViewersToScene();
			GetEVLinkedViewers()->Render();
			try {
				pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenSegmentation();
			}
			catch (std::exception& e) {
				eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
				return;
			}
			logUpdateWithStageSettings("EVTargetDefine::ComputeLumenSegmentation", currentTarget, currentTarget->getID());
			currentTarget->unsavedChanges = true;
			currentTarget->lumenSegIDdefine = GetEVLinkedViewers()->AddSegmentation4(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation(), 0, "lumenSegmentation");
			clearTargetBeyondCurrentStage(currentTarget);
			GetEVLinkedViewers()->SyncViewersToScene();
			GetEVLinkedViewers()->Render();

			if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->PathPreConditions()) {
				GetEVLinkedViewers()->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Computing Detailed Cross Section Positions"));
				GetEVLinkedViewers()->SyncViewersToScene();
				GetEVLinkedViewers()->Render();
				try {
					pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputePath();
				}
				catch (std::exception& e) {
					eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
					return;
				}

				logUpdateWithStageSettings("EVTargetDefine::ComputeVesselTargetPath", currentTarget, currentTarget->getID());
				currentTarget->unsavedChanges = true;
				GetEVLinkedViewers()->RemoveVesselTarget(currentTarget->initializerIDdefine); // now that we have a path, don't want to display the initializaer too
				currentTarget->initializerIDdefine = null_ebID;
				currentTarget->pathIDdefine = GetEVLinkedViewers()->AddVesselTarget(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetPath(), "path");
				clearTargetBeyondCurrentStage(currentTarget);
				GetEVLinkedViewers()->SyncViewersToScene();
				GetEVLinkedViewers()->Render();
				if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenPartitionPreConditions()) {
					GetEVLinkedViewers()->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Partitioning Lumen into Vessels"));
					GetEVLinkedViewers()->SyncViewersToScene();
					GetEVLinkedViewers()->Render();
					try {
						pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenPartition();
					}
					catch (std::exception& e) {
						eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;

						return;
					}
					logUpdateWithStageSettings("EVTargetDefine::ComputeLumenPartition", currentTarget, currentTarget->getID());
					currentTarget->unsavedChanges = true;
					GetEVLinkedViewers()->RemoveSegmentation4(currentTarget->lumenSegIDdefine); // if have partitions, don't want to display the segmentation too
					currentTarget->lumenSegIDdefine = null_ebID;
					int i = 0;
					for (auto partition : *pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenPartition()) {
						currentTarget->lumenPartIDsDefine.insert(GetEVLinkedViewers()->AddSegmentation4(partition.GetPointer(), 0, "lumenPartition" + std::to_string(i++)));
					}
					clearTargetBeyondCurrentStage(currentTarget);
					GetEVLinkedViewers()->SyncViewersToScene();
					GetEVLinkedViewers()->Render();
				} // end-if can can compute global lumen
				else
					qWarning() << "evaluate lumen was not able to make it all the way through to the partitioned lumen.";
			} // end-if can compute vessel target path 

			GetEVLinkedViewers()->GetViewer(ebvIdVolume)->SetTemporaryText("");
			GetEVLinkedViewers()->SyncViewersToScene();
			GetEVLinkedViewers()->Render();
			currentTarget->isViable = true; // now that there is a lumen the target may be considered viable :-)
			viableTargetsCount++;

			//progressIndicator.setMaximum(100); // any value will take it down
		} // end-if can compute local lumen
		else
		{
		}
	}
	else
	{
	}
}

