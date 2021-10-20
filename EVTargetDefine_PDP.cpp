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
 

void EVTargetDefine::preloadDefinePost()
{
	ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
	selectCurrentTarget(NULL); // start out with no target selected as current focus

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	QList<EVTargetDef>* targets = GetTargets();

	// loop through initial description to setup predefined targets each according to how well specified they are
	for (int i = 0; i < targets->size(); i++)
	{
		EVTargetDef* def = &((*targets)[i]);

		if ((def->getID() != "") && (def->getBodySite() != "") && (def->getTargetFolder() != ""))
		{
			qInfo() << "EVTargetDefine::preloadDefinePost: preloading a target of bodySite: " << def->getBodySite();
			def->unsavedChanges = false;
			def->setParentPipeline(pipeline);
			def->targetPipelineID = pipeline->AddVesselTargetPipeline();
			pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetBodySite(def->getBodySite().toStdString());
			pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetTargetFolder(def->getTargetFolder().toStdString());

			//def->parameters = new processingParameters(this, owner->systemPreferencesObject, owner->clinicalJurisdiction, pipeline, def->getBodySite(), def->targetPipelineID);

			PromptMessage(EV_MSG_PROCESSING_PARAMETERS_SETTINGS_CHANGED);
			def->pushTargetParametersToPipeline(pipeline->GetVesselTargetPipeline(def->targetPipelineID));



			if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->InitializationPreConditions()) {
				if (def->getTargetInitializerFileName() != "") {
					QFileInfo checkFile(def->getTargetInitializerFileName());
					if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
						try {
							pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenInitialization(def->getTargetInitializerFileName().toStdString());
						}
						catch (std::exception& e) {
							eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
							PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
							break;
						}
					}
					else {
						PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
						break;
					}
				}
				else {
					pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CreateInitialization();
				}





				// now we continue as deep as the target definition allows
				// start with lumenSegmentation
				int j;
				EVRegion* reg;
				for (j = 0; j < def->regions.size(); j++) {
					reg = &((def->regions)[j]);
					if (reg->regName.contains("lumenSegmentation")) {
						break;
					}
				}
				if (j < def->regions.size()) {
					if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenSegmentationPreConditions()) {
						if (reg->regFileName != "") {
							QFileInfo checkFile(reg->regFileName);
							if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
								try {
									pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLumenSegmentation(reg->regFileName.toStdString());
									qInfo() << "...OpenLumenSegmentation done";
								}
								catch (std::exception& e) {
									eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
									PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
									break;
								}
							}
							else {
								PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
								break;
							}
						}
					}
					else if (reg->regFileName != "") {
						qWarning() << "target in list position" << i << "has a lumen segmentation EVRegion, but pipeline state doesn't allow it.";
						return;
					}
				}




				// proceed to path
				if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PathPreConditions()) {
					if (def->getTargetPathFileName() != "") {
						QFileInfo checkFile(def->getTargetPathFileName());
						if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
							if (false/* replace with test for whether the object shoudl be recomputed because runnign a later version from that which created the object */) {
								try {
									pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputePath();
								}
								catch (std::exception& e) {
									eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
									PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
									break;
								}
								logUpdateWithStageSettings("EVTargetDefine::ComputeVesselTargetPath", def, def->getID());
								def->unsavedChanges = true;
							}
							else {
								try {
									pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenPath(def->getTargetPathFileName().toStdString());
									qInfo() << "...OpenPath done";
								}
								catch (std::exception& e) {
									eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
									PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
									break;
								}
							}
						else {
							PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
							break;
						}
					}
				}
				else if (def->getTargetPathFileName() != "") {
					qWarning() << "target in list position" << i << "has a path, but pipeline state doesn't allow it.";
					return;
				}




				// lumenPartition
				for (j = 0; j < def->regions.size(); j++) {
					reg = &((def->regions)[j]);
					if (reg->regName.contains("lumenPartition"))
						break;
				}
				if (j < def->regions.size()) {
					if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenPartitionPreConditions()) {
						if (reg->regFileName != "") {
							QFileInfo checkFile(reg->regFileName);
							if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
								if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) {
									try {
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenPartition();
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
										PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
										break;
									}
									logUpdateWithStageSettings("EVTargetDefine::ComputeLumenPartition", def, def->getID());
									def->unsavedChanges = true;
								}
								else {
									try {
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLumenPartition(reg->regFileName.toStdString());
										qInfo() << "...OpenLumenPartition done";
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
										PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
										break;
									}
								}
								// given that we have a partitioned lumen, the target is considered viable.
								def->isViable = true;
								viableTargetsCount++;
							}
							else {
								PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
								break;
							}
						}
					}
					else if (reg->regFileName != "") {
						qWarning() << "target in list position" << i << "has a partitioned lumen, but pipeline state doesn't allow it.";
						return;
					}
				}





				// registration
				if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->RegistrationPreConditions()) {
					if (def->getTargetRegistrationTransforms() != "") {
						QFileInfo checkFile(def->getTargetRegistrationTransforms());
						if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
							if (false/* replace with test for whether the object shoudl be recomputed because runnign a later version from that which created the object */) {
								try {
									pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeRegistration();
								}
								catch (std::exception& e) {
									eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
									PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
									break;
								}
								logUpdateWithStageSettings("EVTargetDefine::ComputeRegistration", def, def->getID());
								def->unsavedChanges = true;
							}
							else {
								try {
									pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenRegistration(def->getTargetRegistrationTransforms().toStdString());
									qInfo() << "...OpenRegistration done";
								}
								catch (std::exception& e) {
									eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
									PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
									break;
								}
							}
						else {
							PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
							break;
						}
					}
				}
				else if (def->getTargetRegistrationTransforms() != "") {
					qWarning() << "target in list position" << i << "has registration transforms, but pipeline state doesn't allow it.";
					return;
				}





				// wallSegmentation
				for (j = 0; j < def->regions.size(); j++) {
					reg = &((def->regions)[j]);
					if (reg->regName.contains("wallSegmentation"))
						break;
				}
				if (j < def->regions.size()) {
					if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenAndWallSegmentationPreConditions()) {
						if (reg->regFileName != "") {
							QFileInfo checkFile(reg->regFileName);
							if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
								try {
									pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLumenAndWallSegmentation(reg->regFileName.toStdString());
									qInfo() << "...OpenLumenAndWallSegmentation done";
								}
								catch (std::exception& e) {
									eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
									PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
									break;
								}
							}
							else {
								PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
								break;
							}
						}
					}
					else if (reg->regFileName != "") {
						qWarning() << "target in list position" << i << "has a wall segmentation EVRegion, but pipeline state doesn't allow it.";
						return;
					}
				}






				// wallPartition
				for (j = 0; j < def->regions.size(); j++) {
					reg = &((def->regions)[j]);
					if (reg->regName.contains("wallPartition"))
						break;
				}
				if (j < def->regions.size()) {
					if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenAndWallPartitionPreConditions()) {
						if (reg->regFileName != "") {
							QFileInfo checkFile(reg->regFileName);
							if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
								if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) {
									try {
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenAndWallPartition();
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
										PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
										break;
									}
									logUpdateWithStageSettings("EVTargetDefine::ComputeLumenAndWallPartition", def, def->getID());
									def->unsavedChanges = true;
								}
								else {
									try {
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLumenAndWallPartition(reg->regFileName.toStdString());
										qInfo() << "...OpenLumenAndWallPartition done";
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
										PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
										break;
									}
								}
							}
							else {
								PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
								break;
							}
						}
					}
					else if (reg->regFileName != "") {
						qWarning() << "target in list position" << i << "has a partitioned wall, but pipeline state doesn't allow it.";
						return;
					}
				}





				// wallThickness
				EVValueMap* WTmap;
				for (j = 0; j < def->valueMaps.size(); j++) {
					WTmap = &((def->valueMaps)[j]);
					if (WTmap->valueName.contains("wallThickness"))
						break;
				}
				if (j < def->valueMaps.size()) {
					if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->WallThicknessPreConditions()) {
						if (WTmap->valueMapFileName != "") {
							QFileInfo checkFile(WTmap->valueMapFileName);
							if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
								if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) {
									try {
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeWallThickness();
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;

										PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
										break;
									}
									logUpdateWithStageSettings("EVTargetDefine::ComputeWallThickness", def, def->getID());
									def->unsavedChanges = true;
								}
								else {
									try {
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenWallThickness(WTmap->valueMapFileName.toStdString());
										qInfo() << "...OpenWallThickness done";
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
										PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
										break;
									}
								}
							else {
								PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
								break;
							}
						}
					}
					else if (WTmap->valueMapFileName != "") {
						qWarning() << "target in list position" << i << "has a wall thickness value map, but pipeline state doesn't allow it.";
						return;
					}
				}





				// perivascularRegion
				for (j = 0; j < def->regions.size(); j++) {
					reg = &((def->regions)[j]);
					if (reg->regName.contains("perivascularRegion"))
						break;
				}
				if (j < def->regions.size()) {
					if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PerivascularRegionPreConditions()) {
						if (reg->regFileName != "") {
							QFileInfo checkFile(reg->regFileName);
							if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
								try {
									pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenPerivascularRegion(reg->regFileName.toStdString());
									qInfo() << "...OpenPerivascularRegion done";
								}
								catch (std::exception& e) {
									eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
									PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
									break;
								}
							}
							else {
								PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
								break;
							}
						}
					}
					else if (reg->regFileName != "") {
						qWarning() << "target in list position" << i << "has a perivascular EVRegion, but pipeline state doesn't allow it.";
						return;
					}
				}





				// periRegPartition
				for (j = 0; j < def->regions.size(); j++) {
					reg = &((def->regions)[j]);
					if (reg->regName.contains("periRegPartition"))
						break;
				}
				if (j < def->regions.size()) {
					if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PerivascularRegionPartitionPreConditions()) {
						if (reg->regFileName != "") {
							QFileInfo checkFile(reg->regFileName);
							if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
								if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) {
									try {
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputePerivascularRegionPartition();
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
										PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
										break;
									}
									logUpdateWithStageSettings("EVTargetDefine::ComputePerivascularRegionPartition", def, def->getID());
									def->unsavedChanges = true;
								}
								else {
									try {
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenPerivascularRegionPartition(reg->regFileName.toStdString());
										qInfo() << "...OpenPerivascularRegionPartition done";
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
										PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
										break;
									}
								}
							}
							else {
								PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
								break;
							}
						}
					}
					else if (reg->regFileName != "") {
						qWarning() << "target in list position" << i << "has a partitioned perivascular EVRegion, but pipeline state doesn't allow it.";
						return;
					}
				}





				// composition
				EVProbabilityMap* pmap;
				for (j = 0; j < def->probabilityMaps.size(); j++) {
					pmap = &((def->probabilityMaps)[j]);
					if (pmap->probabilityName.contains("composition"))
						break;
				}
				if (j < def->probabilityMaps.size()) {
					if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CompositionPreConditions()) {
						if (pmap->probabilityMapFileName != "") {
							QFileInfo checkFile(pmap->probabilityMapFileName);
							if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
								if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) {
									try {
										/*NOT NEEDED NOW BUT LEFT FOR THE PATTERNif (owner->clinicalJurisdiction != "") // any of the clinical editions
										  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetCompositionAnalytes(true,true,false,true,false);
										else*/
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetCompositionAnalytes(true, true, true, true, true);
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeComposition();
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
										PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
										break;
									}
									logUpdateWithStageSettings("EVTargetDefine::ComputeComposition", def, def->getID());
									def->unsavedChanges = true;
								}
								else {
									try {
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenComposition(pmap->probabilityMapFileName.toStdString());
										qInfo() << "...OpenComposition done";
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
										PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
										break;
									}
								}
							else {
								PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
								break;
							}
						}
					}
					else if (pmap->probabilityMapFileName != "") {
						qWarning() << "target in list position" << i << "has a composition probability map, but pipeline state doesn't allow it.";
						//return;
					}
				}






				// capThickness
				EVValueMap* CTmap;
				for (j = 0; j < def->valueMaps.size(); j++) {
					CTmap = &((def->valueMaps)[j]);
					if (CTmap->valueName.contains("capThickness"))
						break;
				}
				if (j < def->valueMaps.size()) {
					if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CapThicknessPreConditions()) {
						if (CTmap->valueMapFileName != "") {
							QFileInfo checkFile(CTmap->valueMapFileName);
							if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
								if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) {
									try {
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeCapThickness();
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
										PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
										break;
									}
									logUpdateWithStageSettings("EVTargetDefine::ComputeCapThickness", def, def->getID());
									def->unsavedChanges = true;
								}
								else {
									try {
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenCapThickness(CTmap->valueMapFileName.toStdString());
										qInfo() << "...OpenCapThickness done";
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
										PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
										break;
									}
								}
							else {
								PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
								break;
							}
						}
					}
					else if (CTmap->valueMapFileName != "") {
						qWarning() << "target in list position" << i << "has a cap thickness value map, but pipeline state doesn't allow it.";
						//return;
					}
				}








				// readings
				if (def->getTargetReadingsFileName() != "") {
					if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ReadingsPreConditions()) {
						QFileInfo checkFile(def->getTargetReadingsFileName());
						if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
							if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) {
								try {
									pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeReadings();
								}
								catch (std::exception& e) {
									eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
									PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
									break;
								}
								logUpdateWithStageSettings("EVTargetDefine::ComputeVesselTargetReadings", def, def->getID());
								def->unsavedChanges = true;
							}
							else {
								try {
									pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenReadings(def->getTargetReadingsFileName().toStdString());
									qInfo() << "...OpenReadings done";
								}
								catch (std::exception& e) {
									eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
									PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
									break;
								}
							}
						else {
							PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
							break;
						}
					}
					else {
						qWarning() << "target in list position" << i << "has readings, but pipeline state doesn't allow it.";
						//return;
					}
				} // end-if there are readings








				// lesionLumenPartition
				for (j = 0; j < def->regions.size(); j++) {
					reg = &((def->regions)[j]);
					if (reg->regName.contains("lesionLumenPartition"))
						break;
				}
				if (j < def->regions.size()) {
					if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionLumenPartitionPreConditions()) {
						if (reg->regFileName != "") {
							QFileInfo checkFile(reg->regFileName);
							if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
								if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) {
									try {
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionLumenPartition();
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
										PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
										break;
									}
									logUpdateWithStageSettings("EVTargetDefine::ComputeLesionLumenPartition", def, def->getID());
									def->unsavedChanges = true;
								}
								else {
									try {
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLesionLumenPartition(reg->regFileName.toStdString());
										qInfo() << "...OpenLesionLumenPartition done";
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
										PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
										break;
									}
								}
							}
							else {
								PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
								break;
							}
						}
					}
					else if (reg->regFileName != "") {
						qWarning() << "target in list position" << i << "has a partitioned lumen for lesions, but pipeline state doesn't allow it.";
						//return;
					}
				}






				// lesionLumenAndWallPartition
				for (j = 0; j < def->regions.size(); j++) {
					reg = &((def->regions)[j]);
					if (reg->regName.contains("lesionLumenAndWallPartition"))
						break;
				}
				if (j < def->regions.size()) {
					if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionLumenAndWallPartitionPreConditions()) {
						if (reg->regFileName != "") {
							QFileInfo checkFile(reg->regFileName);
							if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
								if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) {
									try {
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionLumenAndWallPartition();
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
										PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
										break;
									}
									logUpdateWithStageSettings("EVTargetDefine::ComputeLesionLumenAndWallPartition", def, def->getID());
									def->unsavedChanges = true;
								}
								else {
									try {
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLesionLumenAndWallPartition(reg->regFileName.toStdString());
										qInfo() << "...OpenLesionLumenAndWallPartition done";
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
										PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
										break;
									}
								}
							}
							else {
								PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
								break;
							}
						}
					}
					else if (reg->regFileName != "") {
						qWarning() << "target in list position" << i << "has a partitioned lumenAndWall for lesions, but pipeline state doesn't allow it.";
						//return;
					}
				}







				// lesionPeriRegPartition
				for (j = 0; j < def->regions.size(); j++) {
					reg = &((def->regions)[j]);
					if (reg->regName.contains("lesionPeriRegPartition"))
						break;
				}
				if (j < def->regions.size()) {
					if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionPerivascularRegionPartitionPreConditions()) {
						if (reg->regFileName != "") {
							QFileInfo checkFile(reg->regFileName);
							if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
								if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) {
									try {
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionPerivascularRegionPartition();
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
										PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
										break;
									}
									logUpdateWithStageSettings("EVTargetDefine::ComputeLesionPerivascularRegionPartition", def, def->getID());
									def->unsavedChanges = true;
								}
								else {
									try {
										pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLesionPerivascularRegionPartition(reg->regFileName.toStdString());
										qInfo() << "...OpenLesionPerivascularRegionPartition done";
									}
									catch (std::exception& e) {
										eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
										PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
										break;
									}
								}
							}
							else {
								PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
								break;
							}
						}
					}
					else if (reg->regFileName != "") {
						qWarning() << "target in list position" << i << "has a partitioned perivascular EVRegion for lesions, but pipeline state doesn't allow it.";
						//return;
					}
				}








				// lesion readings
				if (def->getLesionReadingsFileName() != "")
				{
					if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionReadingsPreConditions()) {
						QFileInfo checkFile(def->getLesionReadingsFileName());
						if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
							if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */)
							{
								try
								{
									pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionReadings();
								}
								catch (std::exception& e)
								{
									eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
									PromptMessage(EV_MSG_ERROR_COMPUTING_RESET_TARGET);
									break;
								}
								logUpdateWithStageSettings("EVTargetDefine::ComputeLesionReadings", def, def->getID());
								def->unsavedChanges = true;
							}
							else
							{
								try
								{
									pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLesionReadings(def->getLesionReadingsFileName().toStdString());
									qInfo() << "...OpenLesionReadings done";
								}
								catch (std::exception& e)
								{
									eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
									PromptMessage(EV_MSG_ERROR_OPENING_RESET_TARGET);
									break;
								}
							}
						else {
							PromptMessage(EV_MSG_ERROR_LIST_ENDING_PRELOAD_EARLY_FILE_NOT_EXIST);
							break;
						}
					}
					else
					{
						qWarning() << "target in list position" << i << "has lesion readings, but pipeline state doesn't allow it.";
						//return;
					}
				} // end-if there are readings
			} // end-if the target can be initialized
			else
				qWarning() << "target in list position" << i << "cannot be initialized, skipping.";



			presentTarget(def);

		} // end-if the minimum spec is in place (ID, bodySite, and folder all being non-blank) 
		else
			qWarning() << "target in list position" << i << "does not have the minimum specification, skipping.";
	} // end-for each listed target

}
