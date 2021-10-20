// Copyright (c) Elucid Bioimaging

#ifndef TARGETDEFINE_H
#define TARGETDEFINE_H

#include "seriesSurvey.h"
#include "workItemListFileToken.h"
#include "SegmentationEditor.h"
#include "ebiVesselTarget.h"
#include "manifest.h"

#include <QWidget>
#include <QString>
#include <QComboBox>
#include <QPushButton>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QFormLayout>
#include <QList>
#include <QStackedWidget>
#include <QHttpMultiPart>
#include <iostream>



  /**
   * @{
   **
   * @brief Target Define Package
   *
   * The target package comprises the following related classes:
   *
   * 1.      processingParameters, probabilityMap, region, and valueMap
   *                 |
   * 2.          targetDef
   *                 |
   * 3.         targetDefine
   *
   *  \copyright Elucid Bioimaging
   *  \ingroup targetDefine
   */

   /**
    * @brief class probabilityMap represents analyte probability maps
    */
  class probabilityMap
  {
  public:
    probabilityMap(QString name, QString fileName) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ probabilityName = name; probabilityMapFileName = fileName; }
    QString getProbabilityMapFileName() const { ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; return probabilityMapFileName; }
    void setProbabilityMapFileName(QString name) { ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; probabilityMapFileName = name; }
    QString probabilityName, probabilityMapFileName;
  };
  /** @} */

  /**
   * \ingroup targetDefine
   * @{
   **
   * @brief class region represents individual target regions
   */
  class region
  {
  public:
    region(QString name, QString fileName) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ regName = name; regFileName = fileName; }
    QString getRegFileName() const { ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; return regFileName; }
    void setRegFileName(QString name) { ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; regFileName = name; }
    QString regName, regFileName;
  };
  /** @} */

  /**
   * \ingroup targetDefine
   * @{
   **
   * @brief class valueMap represents individual value maps, for example, wall thickness
   */
  class valueMap
  {
  public:
    valueMap(QString name, QString fileName) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ valueName = name; valueMapFileName = fileName; }
    QString getValueMapFileName() const { ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; return valueMapFileName; }
    void setValueMapFileName(QString name) { ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; valueMapFileName = name; }
    QString valueName, valueMapFileName;
  };
  /** @} */

  /**
   * \ingroup targetDefine
   * @{
   **
   * @brief class targetDef represents individual targets, providing all definition needed for computation and display
   */
  class targetDef
  {
  public:
    targetDef();
    targetDef(const QString &id, const QString &site);
    ~targetDef();
    void closeOutCompletedSession();
    void saveTargetDerivedDataChanges(QString baseFolder);

    // these last across analysis tab instantiations
    QList<probabilityMap> probabilityMaps;
    QList<region> regions;
    QList<valueMap> valueMaps;
    QList<QString> keyImages;

    // these get reset when an analysis tab closes
    QString rootVesselName;
    bool isViable;
    bool unsavedChanges;
    ebiVesselPipeline::Pointer pipeline;
    ebID targetPipelineID,
      initializerIDdefine, initializerIDanalyze,
      pathIDdefine, pathIDanalyze,
      readingsIDdefine, readingsIDanalyze,
      lesionReadingsIDdefine, lesionReadingsIDanalyze,
      lumenSegIDdefine, lumenSegIDanalyze,
      wallSegIDdefine, wallSegIDanalyze,
      wallThickIDdefine, wallThickIDanalyze,
      periRegIDdefine, periRegIDanalyze,
      capThickIDdefine, capThickIDanalyze;
    std::set<ebID> lumenPartIDsDefine, lumenPartIDsAnalyze, wallPartIDsDefine, wallPartIDsAnalyze, periRegPartIDsDefine, periRegPartIDsAnalyze, compositionSegIDsAnalyze;
    std::set<ebID> lesionLumenPartIDsDefine, lesionLumenPartIDsAnalyze, lesionLumenAndWallPartIDsDefine, lesionLumenAndWallPartIDsAnalyze, lesionPeriRegPartIDsDefine, lesionPeriRegPartIDsAnalyze;
    processingParameters *parameters;

    void setParentPipeline(ebiVesselPipeline::Pointer p) { pipeline = p; }

    QString getID() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return ID; }
    void setID(const QString &id) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  ID = id; }

    QString getBodySite() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return bodySite; }
    void setBodySite(QString site) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  bodySite = site; }

    QString getTargetFolder() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return targetFolder; }
    bool setTargetFolder(QString workItemFolder) {
      ebLog eblog(Q_FUNC_INFO); eblog << workItemFolder.toStdString() << std::endl;
      QString absTargetFolder = workItemFolder;
      absTargetFolder.append("/");
      absTargetFolder.append(getID());
      QDir targetFolderDir(QDir::current());
      if (targetFolderDir.mkpath(absTargetFolder)) {
        targetFolder = targetFolderDir.relativeFilePath(absTargetFolder);
        return true;
      }
      else {
        targetFolder = workItemFolder;
        return false;
      }
    }

    void repointTargetFiles(QString oldIndividualID, QString newIndividualID) {
      ebLog eblog(Q_FUNC_INFO); eblog << ("repointing target files from " + oldIndividualID + " to " + newIndividualID).toStdString() << std::endl;
      targetFolder.replace(oldIndividualID, newIndividualID);
      readingsLocalFileName.replace(oldIndividualID, newIndividualID);
      targetPathFileName.replace(oldIndividualID, newIndividualID);
      targetInitializerFileName.replace(oldIndividualID, newIndividualID);
      registrationTransforms.replace(oldIndividualID, newIndividualID);

      for (int i = 0; i < probabilityMaps.size(); i++) {
        probabilityMap *map = &(probabilityMaps[i]);
        map->probabilityMapFileName.replace(oldIndividualID, newIndividualID);
      }

      for (int i = 0; i < regions.size(); i++) {
        region *reg = &(regions[i]);
        reg->regFileName.replace(oldIndividualID, newIndividualID);
      }

      for (int i = 0; i < valueMaps.size(); i++) {
        valueMap *map = &(valueMaps[i]);
        map->valueMapFileName.replace(oldIndividualID, newIndividualID);
      }

      for (int i = 0; i < keyImages.size(); i++) {
        QString *keyImage = &(keyImages[i]);
        keyImage->replace(oldIndividualID, newIndividualID);
      }
    }

    QString getTargetInitializerFileName() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return targetInitializerFileName; }
    void setTargetInitializerFileName(QString name) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  targetInitializerFileName = name; }

    QString getTargetPathFileName() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return targetPathFileName; }
    void setTargetPathFileName(QString name) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ targetPathFileName = name; }

    QString getTargetReadingsFileName() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return readingsLocalFileName; }
    void setTargetReadingsFileName(QString name) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ readingsLocalFileName = name; }

    QString getLesionReadingsFileName() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return lesionReadingsLocalFileName; }
    void setLesionReadingsFileName(QString name) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ lesionReadingsLocalFileName = name; }

    QString getTargetRegistrationTransforms() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return registrationTransforms; }
    void setTargetRegistrationTransforms(QString name) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ registrationTransforms = name; }

    void readTargetDef(const QJsonObject &json) {
      //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
      if (parameters != NULL) {
        delete parameters;
        parameters = NULL;
      }
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
        probabilityMap map(key, mapFile);
        probabilityMaps.append(map);
      }

      regions.clear();
      list = json[regions_token].toObject();
      foreach(const QString key, list.keys()) {
        QString regFile = list[key].toString();
        region reg(key, regFile);
        regions.append(reg);
      }

      valueMaps.clear();
      list = json[valueMaps_token].toObject();
      foreach(const QString key, list.keys()) {
        QString mapFile = list[key].toString();
        qDebug() << "mapFile is" << mapFile << ", for key=" << key;
        valueMap map(key, mapFile);
        valueMaps.append(map);
      }

      keyImages.clear();
      QJsonArray keyImageArray = json[keyImages_token].toArray();
      for (int i = 0; i < keyImageArray.size(); ++i) {
        keyImages.append(keyImageArray[i].toString());
      }
    }

    void writeTargetDef(QJsonObject &json) const {
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
      foreach(const probabilityMap map, probabilityMaps)
        probabilityMapsObject[map.probabilityName] = map.probabilityMapFileName;
      json[probabilityMaps_token] = probabilityMapsObject;

      QJsonObject regionsObject;
      foreach(const region reg, regions)
        regionsObject[reg.regName] = reg.regFileName;
      json[regions_token] = regionsObject;

      QJsonObject valueMapsObject;
      foreach(const valueMap map, valueMaps)
        valueMapsObject[map.valueName] = map.valueMapFileName;
      json[valueMaps_token] = valueMapsObject;

      QJsonArray keyImageArray;
      foreach(const QString im, keyImages) {
        keyImageArray.append(im);
      }
      json[keyImages_token] = keyImageArray;
    }

    void writeTargetDef(QString parentDir, QString prefix, manifestQueue *mfiles) const
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

      foreach(const probabilityMap map, probabilityMaps) {
        if ("" != map.probabilityMapFileName) {
          QString keyname = keyPrefix + "probabilityMaps/" + map.probabilityName;
          QString name = nameStr + keyname;
          mfiles->addFile(keyname, QString("application/octet-stream"), name, parentDir, prefix, map.probabilityMapFileName);
        }
      }
      foreach(const region reg, regions) {
        if ("" != reg.regFileName) {
          QString keyname = keyPrefix + "regions/" + reg.regName;
          QString name = nameStr + keyname;
          mfiles->addFile(keyname, QString("application/octet-stream"), name, parentDir, prefix, reg.regFileName);
        }
      }

      foreach(const valueMap map, valueMaps) {
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

    bool updateTarget(targetDef *otarget)
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
        probabilityMap nmap(probabilityMaps[i].probabilityName, probabilityMaps[i].probabilityMapFileName);
        if (i < otarget->probabilityMaps.size())
          otarget->probabilityMaps[i] = nmap;
        else {
          otarget->probabilityMaps.append(nmap);
        }
      }
      for (int i = 0; i < regions.size(); i++) {
        qInfo() << "updating target region " << i;
        region nreg(regions[i].regName, regions[i].regFileName);
        if (i < otarget->regions.size())
          otarget->regions[i] = nreg;
        else {
          otarget->regions.append(nreg);
        }
      }
      for (int i = 0; i < valueMaps.size(); i++) {
        qInfo() << "updating target vmap " << i;
        valueMap vmap(valueMaps[i].valueName, valueMaps[i].valueMapFileName);
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

    void pushTargetParametersToPipeline(ebiVesselTargetPipeline *p) const {
      ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
      SetMetaData(p, bodySite_token, bodySite.toStdString());
    }

  private:
    QString ID, bodySite, targetFolder, targetInitializerFileName, targetPathFileName, readingsLocalFileName, lesionReadingsLocalFileName, registrationTransforms;
  };
  /** @} */

  namespace Ui {
    class targetDefine;
  }

  typedef std::pair<ebiMultiImageReader*, ebID> MultiReaderIDPair;

  /**
   * \ingroup targetDefine
   * @{
   **
   * @brief class targetDefine (in namepace Ui) (subclassed from QWidget): the main class with the list of definitions as a whole and the
   * methods to access and interact with it. Responsible for all screen management associated with targets, including creation, definition, etc.
   */
  class targetDefine : public QWidget
  {
    Q_OBJECT

  public:
    explicit targetDefine(QWidget *owner = 0, QMenu *m = NULL, bool masterWithRespectToMenu = false);
    ~targetDefine();
    QList<targetDef> *targetDefs() { ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;  return targets; };
    void preloadDefinePre(QString product, int index, ebiVesselPipeline::Pointer p, QList<imageSeries> *imageSeriesSet, QList<targetDef> *targetDefs);
    void resetWI(ebiVesselPipeline::Pointer p, QList<imageSeries> *imageSeriesSet, QList<targetDef> *targetDefs);
    void preloadDefinePost();
    void createTargetPre(QString bodySite);
    void establishNewTarget(QString bodySite);
    void completeDistalVessel();
    void labelDistalVessel(QString distalVesselName);
    bool switchTargetPath();
    void deleteTarget();
    void toggleOblique(bool checked);
    void addDistalVessel();
    void resetWall();
    void resetTarget();
    void evaluateLumen();
    void evaluateWall();
    void editSegmentation(bool checked);
    void giveToAnalyzeIfNotAlreadyThere();
    void toggleSegmentation(QString regName, bool checked);
    void setDefiningRootFlag(bool flag) { definingRootFlag = flag; }
    bool definingRoot() { return definingRootFlag; }
    void establishTools(QString product, int index, bool masterWithRespectToMenu);
    vtkSmartPointer<ebvLinkedViewers2> viewers;
    void disconnectMenuActions();
    void connectMenuActions();
    void disableMenuActions();
    void enableMenuActions();
    bool computeRemainingStages(targetDef *def);
    void logUpdateWithStageSettings(QString step, targetDef *def, QString ID);

    public slots:
    void on_backToSurveyButton_clicked();
    void on_continueWithAnalysisButton_clicked();

  private:
    QString thisProduct;
    Ui::targetDefine *ui;
    cap *owner;
    QMenu *targetDefineMenu;
    int viableTargetsCount;
    imageSeries *currentBackingSeries; // various methods need to know what series to effect
    ebID backingImageId;
    targetDef *currentTarget; // various methods need to know what target to effect
    QErrorMessage *message;
    int sessionItemIndex;
    ebiVesselPipeline::Pointer pipeline;
    int savedParallelScale, savedPerspectiveDistance;
    QList<imageSeries> *images; // actual list is declared as a member of workItemListEntry, this is a pointer to it
    QList<targetDef> *targets; // actual list is declared as a member of workItemListEntry, this is a pointer to it
    ebID ebvIdVolume, ebvIdCoronal, ebvIdSagittal, ebvIdAxial; // viewer identifiers on the targetDefine screen itself
    ebvViewer::ViewerType axialType, coronalType, sagittalType;
    capTools *tools;
    std::set< MultiReaderIDPair > added_multireader_images;
    bool editingSegmentation;
    QAction *processingParametersAction, *gotoTargetDefineAction;
    std::map<QAction *, bool> *menuActionEnabledMap;
    int savedSlabThickness;
    bool definingRootFlag;
    bool editingWall;
    SegmentationEditor *segmentationEditor;
    bool readTargetFiles(bool makeCurrentIfViable, targetDef *def);
    void setToggleToolStates();
    void presentTarget(targetDef *def);
    void selectCurrentTarget(targetDef *Def);
    void enableToolButtons();
    void disableToolButtons();
    void clearTargetBeyondCurrentStage(targetDef *def);
    void acceptScreenControlCommon(QStackedWidget *seriesSelectionArea, imageSeries *series);
    void addVessel(QString startFromName);
    bool updateServer();

    private slots:
    void ensureOnTargetDefinePage();
    void acceptScreenControlFromAnalyze(QStackedWidget *seriesSelectionArea, imageSeries *series, targetDef *def);
    void acceptScreenControlFromSurvey(QStackedWidget *seriesSelectionArea, imageSeries *series);
    void resetBackingSeries(imageSeries *series);
    void addRootVesselAction();
    void labelDistalVesselAction();
    void resetAllTargetsDueToChangesInImages();
    void resetAllTargetsDueToDifferentImages();
    void on_processingParametersButton_clicked();
    void processingParametersSettingsChanged();

  signals:
    void logUpdate(QString step, QString ID, stageParameters *stageParams, int sessionItemIndex);
    void giveScreenControlToAnalyze(QStackedWidget *seriesSelectionArea, imageSeries *series);
    void setCurrentTarget(targetDef *def);
    void processCompositionSettingsChange();
  };
  /** @} */


#endif // TARGETDEFINE_H
