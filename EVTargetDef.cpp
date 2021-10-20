
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

void EVTargetDef::closeOutCompletedSession()
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // we keep all the file-oriented objects, but null out the pipeline IDs
  pipeline = NULL;
  isViable = false;

  unsavedChanges = false;
  targetPipelineID = null_ebID;
  
  initializerIDdefine = null_ebID; 
  initializerIDanalyze = null_ebID;
  
  pathIDdefine = null_ebID; 
  pathIDanalyze = null_ebID;
  
  readingsIDdefine = null_ebID; 
  readingsIDanalyze = null_ebID;
  
  lesionReadingsIDdefine = null_ebID; 
  lesionReadingsIDanalyze = null_ebID;
  
  lumenSegIDdefine = null_ebID; 
  lumenSegIDanalyze = null_ebID;
  
  wallSegIDdefine = null_ebID; 
  wallSegIDanalyze = null_ebID;
  
  periRegIDdefine = null_ebID; 
  periRegIDanalyze = null_ebID;
  
  wallThickIDdefine = null_ebID; 
  wallThickIDanalyze = null_ebID;
  
  capThickIDdefine = null_ebID; 
  capThickIDanalyze = null_ebID;
  
  lumenPartIDsDefine.clear(); 
  lumenPartIDsAnalyze.clear();
  
  wallPartIDsDefine.clear(); 
  wallPartIDsAnalyze.clear();
  
  periRegPartIDsDefine.clear(); 
  periRegPartIDsAnalyze.clear();
  
  lesionLumenPartIDsDefine.clear(); 
  lesionLumenAndWallPartIDsDefine.clear(); 
  
  lesionPeriRegPartIDsDefine.clear();
  
  lesionLumenPartIDsAnalyze.clear(); 
  lesionLumenAndWallPartIDsAnalyze.clear(); 
  lesionPeriRegPartIDsAnalyze.clear();
}

EVTargetDef::EVTargetDef() :
  ID(QString("")),
  bodySite(QString(""))
{
  closeOutCompletedSession();
  probabilityMaps.clear();
  regions.clear();
  valueMaps.clear();
  keyImages.clear();
  targetFolder = "";
  readingsLocalFileName = "";
  lesionReadingsLocalFileName = "";
  targetPathFileName = "";
  registrationTransforms = "";
}

EVTargetDef::EVTargetDef(const QString &id, const QString &site) :
  ID(id),
  bodySite(site)
{
  closeOutCompletedSession();
  probabilityMaps.clear();
  regions.clear();
  valueMaps.clear();
  keyImages.clear();
  targetFolder = "";
  readingsLocalFileName = "";
  lesionReadingsLocalFileName = "";
  targetPathFileName = "";
  registrationTransforms = "";
}

EVTargetDef::~EVTargetDef()
{
  regions.clear();
  probabilityMaps.clear();
  valueMaps.clear();
}

bool EVTargetDef::SetTargetFolder(const QString& workItemFolder) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << workItemFolder.toStdString() << std::endl;
  QString absTargetFolder = workItemFolder;
  absTargetFolder.append("/");
  absTargetFolder.append(getID());
  QDir targetFolderDir(QDir::current());
  if (targetFolderDir.mkpath(absTargetFolder)) 
  {
    targetFolder = targetFolderDir.relativeFilePath(absTargetFolder);
    return true;
  }
 
  targetFolder = workItemFolder;  //JCG don't understand this
  return false;
}

void EVTargetDef::RepointTargetFiles(const QString& oldIndividualID, const QString& newIndividualID)
{
  ebLog eblog(Q_FUNC_INFO); eblog << ("repointing target files from " + oldIndividualID + " to " + newIndividualID).toStdString() << std::endl;
  targetFolder.replace(oldIndividualID, newIndividualID);
  readingsLocalFileName.replace(oldIndividualID, newIndividualID);
  targetPathFileName.replace(oldIndividualID, newIndividualID);
  targetInitializerFileName.replace(oldIndividualID, newIndividualID);
  registrationTransforms.replace(oldIndividualID, newIndividualID);

  for (int i = 0; i < probabilityMaps.size(); i++) {
    EVProbabilityMap *map = &(probabilityMaps[i]);
    map->probabilityMapFileName.replace(oldIndividualID, newIndividualID);
  }

  for (int i = 0; i < regions.size(); i++) {
    EVRegion *reg = &(regions[i]);
    reg->regFileName.replace(oldIndividualID, newIndividualID);
  }

  for (int i = 0; i < valueMaps.size(); i++) {
    EVValueMap *map = &(valueMaps[i]);
    map->valueMapFileName.replace(oldIndividualID, newIndividualID);
  }

  for (int i = 0; i < keyImages.size(); i++) {
    QString *keyImage = &(keyImages[i]);
    keyImage->replace(oldIndividualID, newIndividualID);
  }
}

bool EVTargetDef::SaveInitialization(const QString& baseFolder, const QString& prefix)
{
  ebiVesselPipeline::Pointer pPipeline = GetPipeline();
  ebAssert(pPipeline);

  if (!pPipeline->GetVesselTargetPipeline(targetPipelineID)->GetInitialization())
    return false;

  QString targetInitializer = prefix;
  targetInitializer.append("_initializer.json");
  setTargetInitializerFileName(targetInitializer);
  QString targetInitializerFull = baseFolder;
  targetInitializerFull.append("/");
  targetInitializerFull.append(getTargetInitializerFileName());
  pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveInitialization(targetInitializerFull.toStdString());
  return true;
}

bool EVTargetDef::SaveLumenSegmentation(const QString& baseFolder, const QString& prefix)
{
  ebiVesselPipeline::Pointer pPipeline = GetPipeline();
  ebAssert(pPipeline);

  if (!pPipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenSegmentation())
    return false;

  QString lumenSegFileName = prefix;
  lumenSegFileName.append("_lumenSegmentation.nrrd");
  EVRegion lumenSeg("lumenSegmentation", lumenSegFileName);
  QString lumenSegFileNameFull = baseFolder;
  lumenSegFileNameFull.append("/");
  lumenSegFileNameFull.append(lumenSeg.getRegFileName());
  pPipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLumenSegmentation(lumenSegFileNameFull.toStdString());
  regions.append(lumenSeg);
  return true;
}

bool EVTargetDef::SavePath(const QString& baseFolder, const QString& prefix)
{
  ebiVesselPipeline::Pointer pPipeline = GetPipeline();
  ebAssert(pPipeline);

  if (!pPipeline->GetVesselTargetPipeline(targetPipelineID)->GetPath())
    return false;
 
  QString targetPath = prefix;
  targetPath.append("_path.json");
  setTargetPathFileName(targetPath);
  QString targetPathFull = baseFolder;
  targetPathFull.append("/");
  targetPathFull.append(getTargetPathFileName());
  pPipeline->GetVesselTargetPipeline(targetPipelineID)->SavePath(targetPathFull.toStdString());
  return true;
}

bool EVTargetDef::SaveLumenPartition(const QString& baseFolder, const QString& prefix)
{
  ebiVesselPipeline::Pointer pPipeline = GetPipeline();
  ebAssert(pPipeline);

  if (!pPipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenPartition())
    return false;

  QString lumenPartFileName = prefix;
  lumenPartFileName.append("_lumenPartition.multi.nrrd");
  EVRegion lumenPart("lumenPartition", lumenPartFileName);
  QString lumenPartFileNameFull = baseFolder;
  lumenPartFileNameFull.append("/");
  lumenPartFileNameFull.append(lumenPart.getRegFileName());
  pPipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLumenPartition(lumenPartFileNameFull.toStdString());
  regions.append(lumenPart);
  return true;
}

bool EVTargetDef::SaveRegistration(const QString& baseFolder, const QString& prefix)
{
  ebiVesselPipeline::Pointer pPipeline = GetPipeline();
  ebAssert(pPipeline);

  if (!pPipeline->GetVesselTargetPipeline(targetPipelineID)->GetResampledRegisteredImages())
    return false;

  QString registrationTransforms = prefix;
  registrationTransforms.append("_transforms.json");
  setTargetRegistrationTransforms(registrationTransforms);
  QString registrationTransformsFull = baseFolder;
  registrationTransformsFull.append("/");
  registrationTransformsFull.append(getTargetRegistrationTransforms());
  pPipeline->GetVesselTargetPipeline(targetPipelineID)->SaveRegistration(registrationTransformsFull.toStdString());
  return true;
}

bool EVTargetDef::SaveLumenAndWallSegmentation(const QString& baseFolder, const QString& prefix)
{
  ebiVesselPipeline::Pointer pPipeline = GetPipeline();
  ebAssert(pPipeline);

  if (!pPipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenAndWallSegmentation())
    return false;

  QString wallSegFileName = prefix;
  wallSegFileName.append("_wallSegmentation.nrrd");
  EVRegion wallSeg("wallSegmentation", wallSegFileName);
  QString wallSegFileNameFull = baseFolder;
  wallSegFileNameFull.append("/");
  wallSegFileNameFull.append(wallSeg.getRegFileName());
  pPipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLumenAndWallSegmentation(wallSegFileNameFull.toStdString());
  regions.append(wallSeg);
  return true;
}

bool EVTargetDef::SaveLumenAndWallPartition(const QString& baseFolder, const QString& prefix)
{
  ebiVesselPipeline::Pointer pPipeline = GetPipeline();
  ebAssert(pPipeline);

  if (!pPipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenAndWallPartition())
    return false;

  QString wallPartFileName = prefix;
  wallPartFileName.append("_wallPartition.multi.nrrd");
  EVRegion wallPart("wallPartition", wallPartFileName);
  QString wallPartFileNameFull = baseFolder;
  wallPartFileNameFull.append("/");
  wallPartFileNameFull.append(wallPart.getRegFileName());
  pPipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLumenAndWallPartition(wallPartFileNameFull.toStdString());
  regions.append(wallPart);
  return true;
}

bool EVTargetDef::SaveWallThickness(const QString& baseFolder, const QString& prefix)
{
  ebiVesselPipeline::Pointer pPipeline = GetPipeline();
  ebAssert(pPipeline);

  if (!pPipeline->GetVesselTargetPipeline(targetPipelineID)->GetWallThickness())
    return false;

  QString wallThickFileName = prefix;
  wallThickFileName.append("_wallThickness.nrrd");
  EVValueMap wallThick("wallThickness", wallThickFileName);
  QString wallThickFileNameFull = baseFolder;
  wallThickFileNameFull.append("/");
  wallThickFileNameFull.append(wallThick.getValueMapFileName());
  pPipeline->GetVesselTargetPipeline(targetPipelineID)->SaveWallThickness(wallThickFileNameFull.toStdString());
  valueMaps.append(wallThick);
  return true;
}

bool EVTargetDef::SavePerivascularRegion(const QString& baseFolder, const QString& prefix)
{
  ebiVesselPipeline::Pointer pPipeline = GetPipeline();
  ebAssert(pPipeline);

  if (!pPipeline->GetVesselTargetPipeline(targetPipelineID)->GetPerivascularRegion())
    return false;

  QString periRegFileName = prefix;
  periRegFileName.append("_perivascularRegion.nrrd");
  EVRegion periReg("perivascularRegion", periRegFileName);
  QString periRegFileNameFull = baseFolder;
  periRegFileNameFull.append("/");
  periRegFileNameFull.append(periReg.getRegFileName());
  pPipeline->GetVesselTargetPipeline(targetPipelineID)->SavePerivascularRegion(periRegFileNameFull.toStdString());
  regions.append(periReg);
  return true;
}

bool EVTargetDef::SavePerivascularRegionPartition(const QString& baseFolder, const QString& prefix)
{
  ebiVesselPipeline::Pointer pPipeline = GetPipeline();
  ebAssert(pPipeline);

  if (!pPipeline->GetVesselTargetPipeline(targetPipelineID)->GetPerivascularRegionPartition())
    return false;

  QString periRegPartFileName = prefix;
  periRegPartFileName.append("_periRegPartition.multi.nrrd");
  EVRegion periRegPart("periRegPartition", periRegPartFileName);
  QString periRegPartFileNameFull = baseFolder;
  periRegPartFileNameFull.append("/");
  periRegPartFileNameFull.append(periRegPart.getRegFileName());
  pPipeline->GetVesselTargetPipeline(targetPipelineID)->SavePerivascularRegionPartition(periRegPartFileNameFull.toStdString());
  regions.append(periRegPart);
  return true;
}

bool EVTargetDef::SaveComposition(const QString& baseFolder, const QString& prefix)
{
  ebiVesselPipeline::Pointer pPipeline = GetPipeline();
  ebAssert(pPipeline);

  if (!pPipeline->GetVesselTargetPipeline(targetPipelineID)->GetComposition())
    return false;

  QString compositionFileName = prefix;
  compositionFileName.append("_composition.multi.nrrd");
  EVProbabilityMap composition("composition", compositionFileName);
  QString compositionFileNameFull = baseFolder;
  compositionFileNameFull.append("/");
  compositionFileNameFull.append(composition.getProbabilityMapFileName());
  pPipeline->GetVesselTargetPipeline(targetPipelineID)->SaveComposition(compositionFileNameFull.toStdString());
  probabilityMaps.append(composition);
  return true;
}

bool EVTargetDef::SaveCapThickness(const QString& baseFolder, const QString& prefix)
{
  ebiVesselPipeline::Pointer pPipeline = GetPipeline();
  ebAssert(pPipeline);

  if (!pPipeline->GetVesselTargetPipeline(targetPipelineID)->GetCapThickness())
    return false;

  QString capThickFileName = prefix;
  capThickFileName.append("_capThickness.nrrd");
  EVValueMap capThick("capThickness", capThickFileName);
  QString capThickFileNameFull = baseFolder;
  capThickFileNameFull.append("/");
  capThickFileNameFull.append(capThick.getValueMapFileName());
  pPipeline->GetVesselTargetPipeline(targetPipelineID)->SaveCapThickness(capThickFileNameFull.toStdString());
  valueMaps.append(capThick);
  return true;
}

bool EVTargetDef::SaveReadings(const QString& baseFolder, const QString& prefix)
{
  ebiVesselPipeline::Pointer pPipeline = GetPipeline();
  ebAssert(pPipeline);

  if(!pPipeline->GetVesselTargetPipeline(targetPipelineID)->GetReadings())
    return false;

  QString readingsLocalFileName = prefix;
  readingsLocalFileName.append("_readings.json");
  setTargetReadingsFileName(readingsLocalFileName);
  QString readingsLocalFileNameFull = baseFolder;
  readingsLocalFileNameFull.append("/");
  readingsLocalFileNameFull.append(getTargetReadingsFileName());
  pPipeline->GetVesselTargetPipeline(targetPipelineID)->SaveReadings(readingsLocalFileNameFull.toStdString());
  return true;
}

bool EVTargetDef::SaveLesionLumenPartition(const QString& baseFolder, const QString& prefix)
{
  ebiVesselPipeline::Pointer pPipeline = GetPipeline();
  ebAssert(pPipeline);

  if (!pPipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionLumenPartition())
    return false;

  QString lesionLumenPartitionFileName = prefix;
  lesionLumenPartitionFileName.append("_lesionLumenPartition.multi.nrrd");
  EVRegion lesionLumenPart("lesionLumenPartition", lesionLumenPartitionFileName);
  QString lesionLumenPartFileNameFull = baseFolder;
  lesionLumenPartFileNameFull.append("/");
  lesionLumenPartFileNameFull.append(lesionLumenPart.getRegFileName());
  pPipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLesionLumenPartition(lesionLumenPartFileNameFull.toStdString());
  regions.append(lesionLumenPart);
  return true;
}

bool EVTargetDef::SaveLesionLumenAndWallPartition(const QString& baseFolder, const QString& prefix)
{
  ebiVesselPipeline::Pointer pPipeline = GetPipeline();
  ebAssert(pPipeline);

  if (!pPipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionLumenAndWallPartition())
    return false;

  QString lesionLumenAndWallPartitionFileName = prefix;
  lesionLumenAndWallPartitionFileName.append("_lesionLumenAndWallPartition.multi.nrrd");
  EVRegion lesionLumenAndWallPart("lesionLumenAndWallPartition", lesionLumenAndWallPartitionFileName);
  QString lesionLumenAndWallPartFileNameFull = baseFolder;
  lesionLumenAndWallPartFileNameFull.append("/");
  lesionLumenAndWallPartFileNameFull.append(lesionLumenAndWallPart.getRegFileName());
  pPipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLesionLumenAndWallPartition(lesionLumenAndWallPartFileNameFull.toStdString());
  regions.append(lesionLumenAndWallPart);
  return true;
}

bool EVTargetDef::SaveLesionPerivascularRegionPartition(const QString& baseFolder, const QString& prefix)
{
  ebiVesselPipeline::Pointer pPipeline = GetPipeline();
  ebAssert(pPipeline);

  if (!pPipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionPerivascularRegionPartition())
    return false;

  QString lesionPerivascularRegionPartitionFileName = prefix;
  lesionPerivascularRegionPartitionFileName.append("_lesionPerivascularRegionPartition.multi.nrrd");
  EVRegion lesionPerivascularRegionPart("lesionPerivascularRegionPartition", lesionPerivascularRegionPartitionFileName);
  QString lesionPerivascularRegionPartFileNameFull = baseFolder;
  lesionPerivascularRegionPartFileNameFull.append("/");
  lesionPerivascularRegionPartFileNameFull.append(lesionPerivascularRegionPart.getRegFileName());
  pPipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLesionPerivascularRegionPartition(lesionPerivascularRegionPartFileNameFull.toStdString());
  regions.append(lesionPerivascularRegionPart);
  return true;
}

bool EVTargetDef::SaveLesionReadings(const QString& baseFolder, const QString& prefix)
{
  ebiVesselPipeline::Pointer pPipeline = GetPipeline();
  ebAssert(pPipeline);

  if (!pPipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionReadings())
    return false;

  QString lesionReadingsLocalFileName = prefix;
  lesionReadingsLocalFileName.append("_lesionReadings.json");
  setLesionReadingsFileName(lesionReadingsLocalFileName);
  QString lesionReadingsLocalFileNameFull = baseFolder;
  lesionReadingsLocalFileNameFull.append("/");
  lesionReadingsLocalFileNameFull.append(getLesionReadingsFileName());
  pPipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLesionReadings(lesionReadingsLocalFileNameFull.toStdString());
  return true;
}

void EVTargetDef::saveTargetDerivedDataChanges(const QString& baseFolder)
{
  ebLog eblog(Q_FUNC_INFO); eblog << baseFolder.toStdString() << std::endl;
  if (unsavedChanges) 
  {
    // first, scrub all target derived data objects as the correct state will be saved by virtue of what follows
    setTargetInitializerFileName("");
    setTargetPathFileName("");
    setTargetRegistrationTransforms("");
    setTargetReadingsFileName("");
    setLesionReadingsFileName("");
    regions.clear();
    valueMaps.clear();
    probabilityMaps.clear();

    // set up a file name prefix
    QString prefix = getTargetFolder();
    prefix.append("/");
    prefix.append("getLocalUserName"); // JCG function 
    prefix.append("_");
    QDateTime dt = QDateTime::currentDateTime();
    QString dtStr = dt.toString("yyyyMMdd-hhmm");
    QStringList dtSubStr = dtStr.split('-');
    prefix.append(dtSubStr.at(0));
    prefix.append("_");

    if (!SaveInitialization(baseFolder, prefix))
      return;

    if (!SaveLumenSegmentation(baseFolder, prefix))
      return;

    if (!SavePath(baseFolder, prefix))
      return;

    if (!SaveLumenPartition(baseFolder, prefix))
      return;

    if (!SaveRegistration(baseFolder, prefix))
      return;

    if (!SaveLumenAndWallSegmentation(baseFolder, prefix))
      return;

    if (!SaveLumenAndWallPartition(baseFolder, prefix))
      return;

    if (!SaveWallThickness(baseFolder, prefix))
      return;

    if (!SavePerivascularRegion(baseFolder, prefix))
      return;

    if (!SavePerivascularRegionPartition(baseFolder, prefix))
      return;

    if (!SaveComposition(baseFolder, prefix))
      return;

    if (!SaveCapThickness(baseFolder, prefix))
      return;

    if (!SaveReadings(baseFolder, prefix))
      return;

    if (!SaveLesionLumenPartition(baseFolder, prefix))
      return;

    if (!SaveLesionLumenAndWallPartition(baseFolder, prefix))
      return;

    if (!SaveLesionPerivascularRegionPartition(baseFolder, prefix))
      return;

    SaveLesionReadings(baseFolder, prefix); // don't want to check the logics with a return checking

    unsavedChanges = false;
  }
}


void EVTargetDef::readTargetDef(const QJsonObject &json) 
{
  ID = json[targetID_token].toString();
  bodySite = json[bodySite_token].toString();
  targetFolder = json[targetLocalFolderName_token].toString();
  readingsLocalFileName = json[readingsLocalFileName_token].toString();
  lesionReadingsLocalFileName = json[lesionReadingsLocalFileName_token].toString();
  targetPathFileName = json[targetPathFileName_token].toString();
  targetInitializerFileName = json[targetInitializerFileName_token].toString();
  registrationTransforms = json[registrationTransforms_token].toString();

  probabilityMaps.clear();
  QJsonObject list = json[probabilityMaps_token].toObject();
  foreach(const QString key, list.keys()) {
    QString mapFile = list[key].toString();
    EVProbabilityMap map(key, mapFile);
    probabilityMaps.append(map);
  }

  regions.clear();
  list = json[regions_token].toObject();
  foreach(const QString key, list.keys()) {
    QString regFile = list[key].toString();
    EVRegion reg(key, regFile);
    regions.append(reg);
  }

  valueMaps.clear();
  list = json[valueMaps_token].toObject();
  foreach(const QString key, list.keys()) {
    QString mapFile = list[key].toString();
    qDebug() << "mapFile is" << mapFile << ", for key=" << key;
    EVValueMap map(key, mapFile);
    valueMaps.append(map);
  }

  keyImages.clear();
  QJsonArray keyImageArray = json[keyImages_token].toArray();
  for (int i = 0; i < keyImageArray.size(); ++i) {
    keyImages.append(keyImageArray[i].toString());
  }
}

void EVTargetDef::writeTargetDef(QJsonObject &json) 
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  json[targetID_token] = ID;
  json[bodySite_token] = bodySite;
  json[targetLocalFolderName_token] = targetFolder;
  json[readingsLocalFileName_token] = readingsLocalFileName;
  json[lesionReadingsLocalFileName_token] = lesionReadingsLocalFileName;
  json[targetPathFileName_token] = targetPathFileName;
  json[targetInitializerFileName_token] = targetInitializerFileName;
  json[registrationTransforms_token] = registrationTransforms;

  QJsonObject probabilityMapsObject;
  foreach(const EVProbabilityMap map, probabilityMaps)
    probabilityMapsObject[map.probabilityName] = map.probabilityMapFileName;
  json[probabilityMaps_token] = probabilityMapsObject;

  QJsonObject regionsObject;
  foreach(const EVRegion reg, regions)
    regionsObject[reg.regName] = reg.regFileName;
  json[regions_token] = regionsObject;

  QJsonObject valueMapsObject;
  foreach(const EVValueMap map, valueMaps)
    valueMapsObject[map.valueName] = map.valueMapFileName;
  json[valueMaps_token] = valueMapsObject;

  QJsonArray keyImageArray;
  foreach(const QString im, keyImages) {
    keyImageArray.append(im);
  }
  json[keyImages_token] = keyImageArray;
}

void EVTargetDef::writeTargetDef(const QString& parentDir, const QString& prefix, manifestQueue *mfiles)
{
  //  the json of this target has already been captured in the workitem json

  QString keyPrefix = ID + ":";
  QString nameStr = "form-data; name=\"";

  if ("" != readingsLocalFileName) {
    QString name = nameStr + keyPrefix + readingsLocalFileName_token;
    mfiles->addFile(keyPrefix + QString(readingsLocalFileName_token), QString("application/json"), name, parentDir, prefix, readingsLocalFileName);
  }

  if ("" != lesionReadingsLocalFileName) {
    QString name = nameStr + keyPrefix + lesionReadingsLocalFileName_token;
    mfiles->addFile(keyPrefix + QString(lesionReadingsLocalFileName_token), QString("application/json"), name, parentDir, prefix, lesionReadingsLocalFileName);
  }

  if ("" != targetPathFileName) {
    QString name = nameStr + keyPrefix + targetPathFileName_token;
    mfiles->addFile(keyPrefix + QString(targetPathFileName_token), QString("application/json"), name, parentDir, prefix, targetPathFileName);
  }

  if ("" != targetInitializerFileName) {
    QString name = nameStr + keyPrefix + targetInitializerFileName_token;
    mfiles->addFile(keyPrefix + QString(targetInitializerFileName_token), QString("application/json"), name, parentDir, prefix, targetInitializerFileName);
  }

  if ("" != registrationTransforms) {
    QString name = nameStr + keyPrefix + registrationTransforms_token;
    mfiles->addFile(keyPrefix + QString(registrationTransforms_token), QString("application/json"), name, parentDir, prefix, registrationTransforms);
  }

  foreach(const EVProbabilityMap map, probabilityMaps) {
    if ("" != map.probabilityMapFileName) {
      QString keyname = keyPrefix + "probabilityMaps/" + map.probabilityName;
      QString name = nameStr + keyname;
      mfiles->addFile(keyname, QString("application/octet-stream"), name, parentDir, prefix, map.probabilityMapFileName);
    }
  }
  foreach(const EVRegion reg, regions) {
    if ("" != reg.regFileName) {
      QString keyname = keyPrefix + "regions/" + reg.regName;
      QString name = nameStr + keyname;
      mfiles->addFile(keyname, QString("application/octet-stream"), name, parentDir, prefix, reg.regFileName);
    }
  }

  foreach(const EVValueMap map, valueMaps) {
    if ("" != map.valueMapFileName) {
      QString keyname = keyPrefix + "valueMaps/" + map.valueName;
      QString name = nameStr + keyname;
      mfiles->addFile(keyname, QString("application/octet-stream"), name, parentDir, prefix, map.valueMapFileName);
    }
  }

  int count = 0;
  foreach(const QString im, keyImages) {
    QString keyname = keyPrefix + QString(keyImages_token) + "[" + QString::number(count) + "]";
    QString name = nameStr + keyname;
    count++;
    mfiles->addFile(keyname, QString("application/octet-stream"), name, parentDir, prefix, im);
  }
}

bool EVTargetDef::updateTarget(EVTargetDef *otarget)
{
  qInfo() << "updating target " << ID;
  otarget->ID = ID;
  otarget->bodySite = bodySite;
  otarget->targetFolder = targetFolder;
  otarget->readingsLocalFileName = readingsLocalFileName;
  otarget->lesionReadingsLocalFileName = lesionReadingsLocalFileName;
  otarget->targetPathFileName = targetPathFileName;
  otarget->targetInitializerFileName = targetInitializerFileName;
  otarget->registrationTransforms = registrationTransforms;
  for (int i = 0; i < probabilityMaps.size(); i++) {
    qInfo() << "updating target pmap " << i;
    EVProbabilityMap nmap(probabilityMaps[i].probabilityName, probabilityMaps[i].probabilityMapFileName);
    if (i < otarget->probabilityMaps.size())
      otarget->probabilityMaps[i] = nmap;
    else {
      otarget->probabilityMaps.append(nmap);
    }
  }
  for (int i = 0; i < regions.size(); i++) {
    qInfo() << "updating target EVRegion " << i;
    EVRegion nreg(regions[i].regName, regions[i].regFileName);
    if (i < otarget->regions.size())
      otarget->regions[i] = nreg;
    else {
      otarget->regions.append(nreg);
    }
  }
  for (int i = 0; i < valueMaps.size(); i++) {
    qInfo() << "updating target vmap " << i;
    EVValueMap vmap(valueMaps[i].valueName, valueMaps[i].valueMapFileName);
    if (i < otarget->valueMaps.size())
      otarget->valueMaps[i] = vmap;
    else
      otarget->valueMaps.append(vmap);
  }
  for (int i = 0; i < keyImages.size(); i++) {
    qInfo() << "updating target keyimage " << i;
    if (i < otarget->keyImages.size())
      otarget->keyImages[i] = keyImages[i];
    else
      otarget->keyImages.append(keyImages[i]);
  }
  return true;
}

void EVTargetDef::pushTargetParametersToPipeline(ebiVesselTargetPipeline *p) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  SetMetaData(p, bodySite_token, bodySite.toStdString());
}

