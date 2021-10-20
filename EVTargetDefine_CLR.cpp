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

void EVTargetDefine::ClearTargetBeyondStageInitialization(EVTargetDef* def, int currentStage)
{
	EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
	ebAssert(pEVLineedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	if (def->initializerIDdefine != null_ebID) {
		pEVLineedViewers->RemoveVesselTarget(def->initializerIDdefine);
		def->initializerIDdefine = null_ebID;
		def->unsavedChanges = true;
	}
	def->setTargetInitializerFileName("");
}

void EVTargetDefine::ClearTargetBeyondStageLumenSegmentation(EVTargetDef* def, int currentStage)
{
	EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
	ebAssert(pEVLineedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	if (def->lumenSegIDdefine != null_ebID) {
		pEVLineedViewers->RemoveSegmentation4(def->lumenSegIDdefine);
		def->lumenSegIDdefine = null_ebID;
		def->unsavedChanges = true;
	}
	def->lumenSegIDdefine = null_ebID;
	for (int j = 0; j < def->regions.size(); j++) {
		EVRegion* reg = &((def->regions)[j]);
		if (reg->regName.contains("lumenSegmentation")) {
			def->regions.removeAt(j);
			break;
		}
	}

}

void EVTargetDefine::ClearTargetBeyondStagePath(EVTargetDef* def, int currentStage)
{
	EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
	ebAssert(pEVLineedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	if (def->pathIDdefine != null_ebID) {
		pEVLineedViewers->RemoveVesselTarget(def->pathIDdefine);
		def->pathIDdefine = null_ebID;
		// if there is an initializer, need to restore it
		if ((currentStage > 0) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization() != NULL)) {
			def->initializerIDdefine = pEVLineedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization(), "initializer");
		}
		def->unsavedChanges = true;
	}
	def->setTargetPathFileName("");

}

void EVTargetDefine::ClearTargetBeyondStageLumenPartition(EVTargetDef* def, int currentStage)
{
	EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
	ebAssert(pEVLineedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	if (!def->lumenPartIDsDefine.empty()) {
		for (auto partitionID : def->lumenPartIDsDefine) {
			pEVLineedViewers->RemoveSegmentation4(partitionID);
		}
		def->unsavedChanges = true;
	}
	def->lumenPartIDsDefine.clear();
	for (int j = 0; j < def->regions.size(); j++) {
		EVRegion* reg = &((def->regions)[j]);
		if (reg->regName.contains("lumenPartition")) {
			def->regions.removeAt(j);
			break;
		}
	}

}

void EVTargetDefine::ClearTargetBeyondStageResampledRegisteredImages(EVTargetDef* def, int currentStage)
{
	EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
	ebAssert(pEVLineedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	def->setTargetRegistrationTransforms("");
}

void EVTargetDefine::ClearTargetBeyondStageLumenAndWallSegmentation(EVTargetDef* def, int currentStage)
{
	EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
	ebAssert(pEVLineedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	if (def->wallSegIDdefine != null_ebID) {
		pEVLineedViewers->RemoveSegmentation4(def->wallSegIDdefine);
		def->unsavedChanges = true;
	}
	def->wallSegIDdefine = null_ebID;
	for (int j = 0; j < def->regions.size(); j++) {
		EVRegion* reg = &((def->regions)[j]);
		if (reg->regName.contains("wallSegmentation")) {
			def->regions.removeAt(j);
			break;
		}
	}
}

void EVTargetDefine::ClearTargetBeyondStageLumenAndWallPartition(EVTargetDef* def, int currentStage)
{
	EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
	ebAssert(pEVLineedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	if (!def->wallPartIDsDefine.empty()) {
		for (auto partitionID : def->wallPartIDsDefine) {
			pEVLineedViewers->RemoveSegmentation4(partitionID);
		}
		def->unsavedChanges = true;
	}
	def->wallPartIDsDefine.clear();
	for (int j = 0; j < def->regions.size(); j++) {
		EVRegion* reg = &((def->regions)[j]);
		if (reg->regName.contains("wallPartition")) {
			def->regions.removeAt(j);
			break;
		}
	}
}

void EVTargetDefine::ClearTargetBeyondStageWallThickness(EVTargetDef* def, int currentStage)
{
	EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
	ebAssert(pEVLineedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	if (def->wallThickIDdefine != null_ebID) {
		pEVLineedViewers->RemoveImage4(def->wallThickIDdefine);
		def->unsavedChanges = true;
	}
	def->wallThickIDdefine = null_ebID;
	for (int j = 0; j < def->valueMaps.size(); j++) {
		EVValueMap* map = &((def->valueMaps)[j]);
		if (map->valueName.contains("wallThickness")) {
			def->valueMaps.removeAt(j);
			break;
		}
	}
}

void EVTargetDefine::ClearTargetBeyondStagePerivascularRegion(EVTargetDef* def, int currentStage)
{
	EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
	ebAssert(pEVLineedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	if (def->periRegIDdefine != null_ebID) {
		pEVLineedViewers->RemoveSegmentation4(def->periRegIDdefine);
		def->unsavedChanges = true;
	}
	def->periRegIDdefine = null_ebID;
	for (int j = 0; j < def->regions.size(); j++) {
		EVRegion* reg = &((def->regions)[j]);
		if (reg->regName.contains("perivascularRegion")) {
			def->regions.removeAt(j);
			break;
		}
	}
}

void EVTargetDefine::ClearTargetBeyondStagePerivascularRegionPartition(EVTargetDef* def, int currentStage)
{
	EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
	ebAssert(pEVLineedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	if (!def->periRegPartIDsDefine.empty()) {
		for (auto partitionID : def->periRegPartIDsDefine) {
			pEVLineedViewers->RemoveSegmentation4(partitionID);
		}
		def->unsavedChanges = true;
	}
	def->periRegPartIDsDefine.clear();
	for (int j = 0; j < def->regions.size(); j++) {
		EVRegion* reg = &((def->regions)[j]);
		if (reg->regName.contains("periRegPartition")) {
			def->regions.removeAt(j);
			break;
		}
	}

}

void EVTargetDefine::ClearTargetBeyondStageComposition(EVTargetDef* def, int currentStage)
{
	EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
	ebAssert(pEVLineedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	for (int j = 0; j < def->probabilityMaps.size(); j++) {
		EVProbabilityMap* map = &((def->probabilityMaps)[j]);
		if (map->probabilityName.contains("composition")) {
			def->probabilityMaps.removeAt(j);
			break;
		}
	}
}

void EVTargetDefine::ClearTargetBeyondStageCapThickness(EVTargetDef* def, int currentStage)
{
	EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
	ebAssert(pEVLineedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	if (def->capThickIDdefine != null_ebID) {
		pEVLineedViewers->RemoveImage4(def->capThickIDdefine);
		def->unsavedChanges = true;
	}
	def->capThickIDdefine = null_ebID;
	for (int j = 0; j < def->valueMaps.size(); j++) {
		EVValueMap* map = &((def->valueMaps)[j]);
		if (map->valueName.contains("capThickness")) {
			def->valueMaps.removeAt(j);
			break;
		}
	}
}

void EVTargetDefine::ClearTargetBeyondStageReadings(EVTargetDef* def, int currentStage)
{
	EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
	ebAssert(pEVLineedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	if (def->readingsIDdefine != null_ebID) {
		pEVLineedViewers->RemoveVesselTarget(def->readingsIDdefine);
		def->readingsIDdefine = null_ebID;
		// if there is either a path, or even an initializer, need to restore accordingly
		if ((currentStage > ebiVesselTargetPipeline::PATH_STAGE) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath() != NULL)) {
			def->pathIDdefine = pEVLineedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath(), "path");
		}
		else if ((currentStage > ebiVesselTargetPipeline::INITIALIZATION_STAGE) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization() != NULL)) {
			def->initializerIDdefine = pEVLineedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization(), "initializer");
		}
	}
	def->setTargetReadingsFileName("");
}

void EVTargetDefine::ClearTargetBeyondStageLesionLumenPartition(EVTargetDef* def, int currentStage)
{
	EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
	ebAssert(pEVLineedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	if (!def->lesionLumenPartIDsDefine.empty()) {
		for (auto partitionID : def->lesionLumenPartIDsDefine) {
			pEVLineedViewers->RemoveSegmentation4(partitionID);
		}
		def->unsavedChanges = true;
	}
	def->lesionLumenPartIDsDefine.clear();
	for (int j = 0; j < def->regions.size(); j++) {
		EVRegion* reg = &((def->regions)[j]);
		if (reg->regName.contains("lesionLumenPartition")) {
			def->regions.removeAt(j);
			break;
		}
	}
}

void EVTargetDefine::ClearTargetBeyondStageLesionLumenAndWallPartition(EVTargetDef* def, int currentStage)
{
	EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
	ebAssert(pEVLineedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	if (!def->lesionLumenAndWallPartIDsDefine.empty()) {
		for (auto partitionID : def->lesionLumenAndWallPartIDsDefine) {
			pEVLineedViewers->RemoveSegmentation4(partitionID);
		}
		def->unsavedChanges = true;
	}
	def->lesionLumenAndWallPartIDsDefine.clear();
	for (int j = 0; j < def->regions.size(); j++) {
		EVRegion* reg = &((def->regions)[j]);
		if (reg->regName.contains("lesionLumenAndWallPartition")) {
			def->regions.removeAt(j);
			break;
		}
	}
}

void EVTargetDefine::ClearTargetBeyondStageLesionPerivascularRegionPartition(EVTargetDef* def, int currentStage)
{
	EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
	ebAssert(pEVLineedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	if (!def->lesionPeriRegPartIDsDefine.empty()) {
		for (auto partitionID : def->lesionPeriRegPartIDsDefine) {
			std::cerr << "line 1214" << std::endl;
			pEVLineedViewers->RemoveSegmentation4(partitionID);
		}
		def->unsavedChanges = true;
	}
	def->lesionPeriRegPartIDsDefine.clear();
	for (int j = 0; j < def->regions.size(); j++) {
		EVRegion* reg = &((def->regions)[j]);
		if (reg->regName.contains("lesionPeriRegPartition")) {
			def->regions.removeAt(j);
			break;
		}
	}
}

void EVTargetDefine::ClearTargetBeyondStageLesionReadings(EVTargetDef* def, int currentStage)
{
	EVLinkedViewers* pEVLineedViewers = GetEVLinkedViewers();
	ebAssert(pEVLineedViewers);

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	if (def->lesionReadingsIDdefine != null_ebID) {
		pEVLineedViewers->RemoveVesselTarget(def->lesionReadingsIDdefine);
		def->lesionReadingsIDdefine = null_ebID;
		// if there is either readings, a path, or even an initializer, need to restore accordingly
		if ((currentStage > ebiVesselTargetPipeline::READINGS_STAGE) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetReadings() != NULL)) {
			def->readingsIDdefine = pEVLineedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetReadings(), "readings");
		}
		else if ((currentStage > ebiVesselTargetPipeline::PATH_STAGE) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath() != NULL)) {
			def->pathIDdefine = pEVLineedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath(), "path");
		}
		else if ((currentStage > ebiVesselTargetPipeline::INITIALIZATION_STAGE) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization() != NULL)) {
			def->initializerIDdefine = pEVLineedViewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization(), "initializer");
		}
	}
	def->setLesionReadingsFileName("");
}

#define CHECK_STAGE(__ptrstage__, __stage__)  if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->__ptrstage__ == NULL) currentStage = ebiVesselTargetPipeline::__stage__;

int EVTargetDefine::GetStageOfClearTargetBeyondCurrentStage(EVTargetDef* def)
{
	ebLog eblog(Q_FUNC_INFO); eblog << def->getID().toStdString() << std::endl;

	ebiVesselPipeline* pipeline = GetVesselPipeine();
	ebAssert(pipeline);

	int currentStage = -1;

	CHECK_STAGE(GetInitialization(), INITIALIZATION_STAGE)
		CHECK_STAGE(GetLumenSegmentation(), LUMENSEGMENTATION_STAGE)
		CHECK_STAGE(GetPath(), PATH_STAGE)
		CHECK_STAGE(GetLumenPartition(), LUMENPARTITION_STAGE)
		CHECK_STAGE(GetResampledRegisteredImages(), IMAGEREGISTRATION_STAGE)
		CHECK_STAGE(GetLumenAndWallSegmentation(), LUMENANDWALLSEGMENTATION_STAGE)
		CHECK_STAGE(GetLumenAndWallPartition(), LUMENANDWALLPARTITION_STAGE)
		CHECK_STAGE(GetWallThickness(), WALLTHICKNESS_STAGE)
		CHECK_STAGE(GetPerivascularRegion(), PERIVASCULARREGION_STAGE)
		CHECK_STAGE(GetPerivascularRegionPartition(), PERIVASCULARREGIONPARTITION_STAGE)
		CHECK_STAGE(GetComposition(), COMPOSITION_STAGE)
		CHECK_STAGE(GetCapThickness(), CAPTHICKNESS_STAGE)
		CHECK_STAGE(GetReadings(), READINGS_STAGE)
		CHECK_STAGE(GetLesionLumenPartition(), LESIONLUMENPARTITION_STAGE)
		CHECK_STAGE(GetLesionLumenAndWallPartition(), LESIONLUMENANDWALLPARTITION_STAGE)
		CHECK_STAGE(GetLesionPerivascularRegionPartition(), LESIONPERIVASCULARREGIONPARTITION_STAGE)
		CHECK_STAGE(GetLesionReadings(), LESIONREADINGS_STAGE)

		return currentStage;
}

void EVTargetDefine::clearTargetBeyondCurrentStage(EVTargetDef* def)
{
	int currentStage = GetStageOfClearTargetBeyondCurrentStage(def);
	if (-1 != currentStage)
	{
		// we use a switch statement because we want the fall-through behaviour
		switch (currentStage)
		{
		case ebiVesselTargetPipeline::INITIALIZATION_STAGE:
			ClearTargetBeyondStageInitialization(def, currentStage);
		case ebiVesselTargetPipeline::LUMENSEGMENTATION_STAGE:
			ClearTargetBeyondStageLumenSegmentation(def, currentStage);
		case ebiVesselTargetPipeline::PATH_STAGE:
			ClearTargetBeyondStagePath(def, currentStage);
		case ebiVesselTargetPipeline::LUMENPARTITION_STAGE:
			ClearTargetBeyondStageLumenPartition(def, currentStage);
		case ebiVesselTargetPipeline::IMAGEREGISTRATION_STAGE:
			ClearTargetBeyondStageResampledRegisteredImages(def, currentStage);
		case ebiVesselTargetPipeline::LUMENANDWALLSEGMENTATION_STAGE:
			ClearTargetBeyondStageLumenAndWallSegmentation(def, currentStage);
		case ebiVesselTargetPipeline::LUMENANDWALLPARTITION_STAGE:
			ClearTargetBeyondStageLumenAndWallPartition(def, currentStage);
		case ebiVesselTargetPipeline::WALLTHICKNESS_STAGE:
			ClearTargetBeyondStageWallThickness(def, currentStage);
		case ebiVesselTargetPipeline::PERIVASCULARREGION_STAGE:
			ClearTargetBeyondStagePerivascularRegion(def, currentStage);
		case ebiVesselTargetPipeline::PERIVASCULARREGIONPARTITION_STAGE:
			ClearTargetBeyondStagePerivascularRegionPartition(def, currentStage);
		case ebiVesselTargetPipeline::COMPOSITION_STAGE:
			ClearTargetBeyondStageComposition(def, currentStage);
		case ebiVesselTargetPipeline::CAPTHICKNESS_STAGE:
			ClearTargetBeyondStageCapThickness(def, currentStage);
		case ebiVesselTargetPipeline::READINGS_STAGE:
			ClearTargetBeyondStageReadings(def, currentStage);
		case ebiVesselTargetPipeline::LESIONLUMENPARTITION_STAGE:
			ClearTargetBeyondStageLesionLumenPartition(def, currentStage);
		case ebiVesselTargetPipeline::LESIONLUMENANDWALLPARTITION_STAGE:
			ClearTargetBeyondStageLesionLumenAndWallPartition(def, currentStage);
		case ebiVesselTargetPipeline::LESIONPERIVASCULARREGIONPARTITION_STAGE:
			ClearTargetBeyondStageLesionPerivascularRegionPartition(def, currentStage);
		case ebiVesselTargetPipeline::LESIONREADINGS_STAGE:
			ClearTargetBeyondStageLesionReadings(def, currentStage);
		}
	}
}