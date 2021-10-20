#pragma once

#include "EVMap.h"
#include "manifest.h"
#include "EVWorkItemListFileToken.h"

class EVTargetDef
{
public:
  EVTargetDef();
  EVTargetDef(const QString &id, const QString &site);
  virtual ~EVTargetDef();

public:
  inline ebiVesselPipeline::Pointer GetPipeline() { return pipeline; }
  void setParentPipeline(ebiVesselPipeline::Pointer p) { pipeline = p; }

  QString getTargetInitializerFileName() const { return targetInitializerFileName; }
  void setTargetInitializerFileName(const QString& name) { targetInitializerFileName = name; }

  QString getTargetPathFileName() const { return targetPathFileName; }
  void setTargetPathFileName(const QString& name) { targetPathFileName = name; }

  QString getTargetReadingsFileName() const { return readingsLocalFileName; }
  void setTargetReadingsFileName(const QString& name) { readingsLocalFileName = name; }

  QString getLesionReadingsFileName() const { return lesionReadingsLocalFileName; }
  void setLesionReadingsFileName(const QString& name) { lesionReadingsLocalFileName = name; }

  QString getTargetRegistrationTransforms() const { return registrationTransforms; }
  void setTargetRegistrationTransforms(const QString& name) { registrationTransforms = name; }

  QString getID() const { return ID; }
  void setID(const QString &id) { ID = id; }

  QString getBodySite() const { return bodySite; }
  void setBodySite(QString site) { bodySite = site; }

  QString getTargetFolder() const { return targetFolder; }
  bool SetTargetFolder(const QString& workItemFolder);

  void closeOutCompletedSession();

  void saveTargetDerivedDataChanges(const QString& baseFolder);

  bool SaveInitialization(const QString& baseFolder, const QString& prefix);

  bool SaveLumenSegmentation(const QString& baseFolder, const QString& prefix);

  bool SavePath(const QString& baseFolder, const QString& prefix);

  bool SaveLumenPartition(const QString& baseFolder, const QString& prefix);

  bool SaveRegistration(const QString& baseFolder, const QString& prefix);

  bool SaveLumenAndWallSegmentation(const QString& baseFolder, const QString& prefix);

  bool SaveLumenAndWallPartition(const QString& baseFolder, const QString& prefix);

  bool SaveWallThickness(const QString& baseFolder, const QString& prefix);

  bool SavePerivascularRegion(const QString& baseFolder, const QString& prefix);

  bool SavePerivascularRegionPartition(const QString& baseFolder, const QString& prefix);

  bool SaveComposition(const QString& baseFolder, const QString& prefix);

  bool SaveCapThickness(const QString& baseFolder, const QString& prefix);

  bool SaveReadings(const QString& baseFolder, const QString& prefix);

  bool SaveLesionLumenPartition(const QString& baseFolder, const QString& prefix);

  bool SaveLesionLumenAndWallPartition(const QString& baseFolder, const QString& prefix);

  bool SaveLesionPerivascularRegionPartition(const QString& baseFolder, const QString& prefix);

  bool SaveLesionReadings(const QString& baseFolder, const QString& prefix);

  void readTargetDef(const QJsonObject &json);

  void writeTargetDef(QJsonObject &json);
  
  void writeTargetDef(const QString& parentDir, const QString& prefix, manifestQueue *mfiles);

  bool updateTarget(EVTargetDef *otarget);

  void pushTargetParametersToPipeline(ebiVesselTargetPipeline *p);

  void RepointTargetFiles(const QString& oldIndividualID, const QString& newIndividualID);

public:
  // these last across analysis tab instantiations
  QList<EVProbabilityMap> probabilityMaps;
  QList<EVRegion> regions;
  QList<EVValueMap> valueMaps;
  QList<QString> keyImages;

  // these get reset when an analysis tab closes
  QString rootVesselName;
  bool isViable;
  bool unsavedChanges;

  ebiVesselPipeline::Pointer pipeline;
  ebID targetPipelineID;
  
  ebID initializerIDdefine;
  ebID initializerIDanalyze;
  
  ebID pathIDdefine;
  ebID pathIDanalyze;
  
  ebID readingsIDdefine;
  ebID readingsIDanalyze;

  ebID lesionReadingsIDdefine;
  ebID lesionReadingsIDanalyze;
  
  ebID lumenSegIDdefine;
  ebID lumenSegIDanalyze;
  
  ebID wallSegIDdefine;
  ebID wallSegIDanalyze;
  
  ebID wallThickIDdefine;
  ebID wallThickIDanalyze;
  
  ebID periRegIDdefine;
  ebID periRegIDanalyze;
  
  ebID capThickIDdefine;
  ebID capThickIDanalyze;

  std::set<ebID> lumenPartIDsDefine;
  std::set<ebID> lumenPartIDsAnalyze;

  std::set<ebID> wallPartIDsDefine;
  std::set<ebID> wallPartIDsAnalyze;
  
  std::set<ebID> periRegPartIDsDefine;
  std::set<ebID> periRegPartIDsAnalyze;
  
  std::set<ebID> compositionSegIDsAnalyze;

  std::set<ebID> lesionLumenPartIDsDefine;
  std::set<ebID> lesionLumenPartIDsAnalyze;

  std::set<ebID> lesionLumenAndWallPartIDsDefine;
  std::set<ebID> lesionLumenAndWallPartIDsAnalyze;
  
  std::set<ebID> lesionPeriRegPartIDsDefine;
  std::set<ebID> lesionPeriRegPartIDsAnalyze;

private:
  QString ID; 
  QString bodySite;
  QString targetFolder;
  QString targetInitializerFileName;
  QString targetPathFileName;
  QString readingsLocalFileName;
  QString lesionReadingsLocalFileName;
  QString registrationTransforms;
};




