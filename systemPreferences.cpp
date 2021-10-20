// Copyright (c) Elucid Bioimaging

#include "systemPreferences.h"
#include "ui_systemPreferences.h"
#include "cap.h"
#include "ebAssert.h"

/**
 * \ingroup cap
 * @{
 *
 * See cap.h for description of the CAP package purpose and contents.  This file has member functions for classes in the package.
 */

/**
 * @page systemPreferences member functions
 */
void systemPreferences::assignValuesFromSaved()
{
  // General tab
  ui->encounterWindow->setValue(encounterWindow);
  ui->promptBeforeSave->setChecked(promptBeforeSave);
  ui->confirmWhenSetProcessingParameters->setChecked(confirmWhenSetProcessingParameters);
  ui->presentCompositionControl->setChecked(presentCompositionControl);
  ui->displayExtractMessages->setChecked(displayExtractMessages);
  ui->labelTabsWithWorkItemID->setChecked(false);
  ui->labelTabsWithIndividualID->setChecked(false);
  ui->labelTabsWithSubjectID->setChecked(false);
  ui->labelTabsWithPatientName->setChecked(false);
  ui->labelTabsWithAlsoKnownAs->setChecked(false);
  switch (labelTabsWith) {
          case WORKITEMID:
            ui->labelTabsWithWorkItemID->setChecked(true);
            break;
          case PATIENTID:
            ui->labelTabsWithIndividualID->setChecked(true);
            break;
          case PATIENTNAME:
            ui->labelTabsWithPatientName->setChecked(true);
            break;
          case SUBJECTID:
            ui->labelTabsWithSubjectID->setChecked(true);
            break;
          case ALSOKNOWNAS:
            ui->labelTabsWithAlsoKnownAs->setChecked(true);
            break;
  }
  // Import tab
  ui->recentlyImportedImages->setValue(recentlyImportedImages);
  ui->importFoldersRecursivelyCheckBox->setChecked(importFoldersRecursively);
  ui->requireOriginalCheckBox->setChecked(requireOriginal);
  ui->excludeLocalizerCheckBox->setChecked(excludeLocalizer);
  ui->excludePreContrastCheckBox->setChecked(excludePreContrast);
  // Viewer tab
  ui->targetDefineIntensityWindow->setValue(targetDefineIntensityWindow);
}

systemPreferences::systemPreferences(QWidget *p) :
    QDialog(p),
    ui(new Ui::systemPreferences)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner = p;

  // default values (if not overridden by values persisted in config file)
  // General tab
  encounterWindow = 2;
  promptBeforeSave = true;
  confirmWhenSetProcessingParameters = true;
  presentCompositionControl = true;
  displayExtractMessages = false;
  labelTabsWith = PATIENTID;
  // Import tab
  recentlyImportedImages = 3.0;
  importFoldersRecursively = true;
  requireOriginal = true;
  excludeLocalizer = true;
  excludePreContrast = true;
  // Viewer tab
  targetDefineIntensityWindow = 400;
  
  // if the config file is present, use it to override the default values (as means to save across sessions)
  QString configFileName = cap::getConfigurationDir();
  configFileName.append("/systemPreferences.json");
  QFileInfo configFileInfo(configFileName);
  if (configFileInfo.exists() && configFileInfo.isFile() && configFileInfo.isReadable()) {
    QFile configFile(configFileName);

    if (!configFile.open(QIODevice::ReadOnly))
      QMessageBox::warning(owner, tr("Warning: Cannot open system preferences config file for reading."), QString(tr("Warning: Cannot open system preferences config file for reading (should not effect normal CAP operation but please contact Elucid)")));
    else {
      QByteArray saveData = configFile.readAll();
      QJsonDocument configDoc = QJsonDocument::fromJson(saveData);
      QJsonObject json = configDoc.object();
      // General tab
      encounterWindow = (int) (json[encounterWindow_token].toDouble()); 
      promptBeforeSave = json[promptBeforeSave_token].toBool(); 
      displayExtractMessages = json[displayExtractMessages_token].toBool(); 
      confirmWhenSetProcessingParameters = json[confirmWhenSetProcessingParameters_token].toBool(); 
      presentCompositionControl = json[presentCompositionControl_token].toBool(); 
      QString labelTabsWithString = json[labelTabsWith_token].toString(); 
      if (labelTabsWithString.toUpper() == "WORKITEMID")
        labelTabsWith = WORKITEMID;
      else if (labelTabsWithString.toUpper() == "PATIENTID")
        labelTabsWith = PATIENTID;
      else if (labelTabsWithString.toUpper() == "PATIENTNAME")
        labelTabsWith = PATIENTNAME;
      else if (labelTabsWithString.toUpper() == "SUBJECTID")
        labelTabsWith = SUBJECTID;
      else if (labelTabsWithString.toUpper() == "ALSOKNOWNAS")
        labelTabsWith = ALSOKNOWNAS;
      else
        QMessageBox::warning(owner, tr("Warning: Value given for labelTabsWith not valid"), QString(tr("Warning: Value given for labelTabsWith not valid")));
      // Imported tab
      if (json.find(recentlyImportedImages_token) != json.end())
        recentlyImportedImages = json[recentlyImportedImages_token].toDouble();
      if (json.find(importFoldersRecursively_token) != json.end())
        importFoldersRecursively = json[importFoldersRecursively_token].toBool();
      if (json.find(requireOriginal_token) != json.end())
        requireOriginal = json[requireOriginal_token].toBool();
      if (json.find(excludeLocalizer_token) != json.end())
        excludeLocalizer = json[excludeLocalizer_token].toBool();
      if (json.find(excludePreContrast_token) != json.end())
        excludePreContrast = json[excludePreContrast_token].toBool();
      // Viewer tab
      if (json.find(targetDefineIntensityWindow_token) != json.end())
        targetDefineIntensityWindow = json[targetDefineIntensityWindow_token].toDouble(); 
    }
  }
  
  // setup the form
  ui->setupUi(this);
  setWindowFlags(windowFlags()|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);

  // now use the established values
  assignValuesFromSaved();
}

systemPreferences::~systemPreferences()
{
  hide();
  delete ui;
}

void systemPreferences::displayPreferences()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  hide(); //first hide before show, which has the effect of forcing to top
  show();
}

void systemPreferences::accept()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // take the necessary steps to act on the values
  // General tab
  encounterWindow = ui->encounterWindow->value();
  promptBeforeSave = ui->promptBeforeSave->isChecked();
  displayExtractMessages = ui->displayExtractMessages->isChecked();
  confirmWhenSetProcessingParameters = ui->confirmWhenSetProcessingParameters->isChecked();
  presentCompositionControl = ui->presentCompositionControl->isChecked();
  if (ui->labelTabsWithWorkItemID->isChecked())
          labelTabsWith = WORKITEMID;
  if (ui->labelTabsWithIndividualID->isChecked())
          labelTabsWith = PATIENTID;
  if (ui->labelTabsWithPatientName->isChecked())
          labelTabsWith = PATIENTNAME;
  if (ui->labelTabsWithSubjectID->isChecked())
          labelTabsWith = SUBJECTID;
  if(ui->labelTabsWithAlsoKnownAs->isChecked())
          labelTabsWith = ALSOKNOWNAS;
  // Import tab
  recentlyImportedImages = ui->recentlyImportedImages->value();
  importFoldersRecursively = ui->importFoldersRecursivelyCheckBox->isChecked();
  requireOriginal = ui->requireOriginalCheckBox->isChecked();
  excludeLocalizer = ui->excludeLocalizerCheckBox->isChecked();
  excludePreContrast = ui->excludePreContrastCheckBox->isChecked();
  // Viewer tab
  targetDefineIntensityWindow = ui->targetDefineIntensityWindow->value();
  // update items that need immediate updating
  cap *capOwner = dynamic_cast<cap*>(owner);
  ebAssert(capOwner);
  for (int sessionItemIndex = 0; sessionItemIndex < MAXSESSIONITEMS; ++sessionItemIndex) {
    // refresh imported dicom images
    workItem *wi = capOwner->getWorkItem(sessionItemIndex);
    if (wi)
      wi->populateImportedDICOMImages();
    // set target define viewers target initialization intensity window
    targetDefine *td = capOwner->getTargetDefine(sessionItemIndex);
    if (td)
      td->viewers->SetVesselTargetInitializationIntensityWindow(targetDefineIntensityWindow);
  }
  
  // also write to config file for next time
  QString configFileName = cap::getConfigurationDir();
  configFileName.append("/systemPreferences.json");
  QFileInfo configFileInfo(configFileName);
  QFile saveFile(configFileName);
  if (!saveFile.open(QIODevice::WriteOnly)) {
    QMessageBox::warning(owner, tr("Warning: Cannot open system preferences config file for writing."), QString(tr("Warning: Cannot open system preferences config file for writing (should not effect normal CAP operation but please contact Elucid)")));
  }
  else {
    QJsonObject json;
    // General tab
    json[encounterWindow_token] = encounterWindow; 
    json[promptBeforeSave_token] = promptBeforeSave; 
    json[displayExtractMessages_token] = displayExtractMessages; 
    json[confirmWhenSetProcessingParameters_token] = confirmWhenSetProcessingParameters; 
    json[presentCompositionControl_token] = presentCompositionControl; 
    if (labelTabsWith == WORKITEMID)
      json[labelTabsWith_token] = "WORKITEMID";
    else if (labelTabsWith == PATIENTID)
      json[labelTabsWith_token] = "PATIENTID";
    else if (labelTabsWith == PATIENTNAME)
      json[labelTabsWith_token] = "PATIENTNAME";
    else if (labelTabsWith == SUBJECTID)
      json[labelTabsWith_token] = "SUBJECTID";
    else if(labelTabsWith == ALSOKNOWNAS)
      json[labelTabsWith_token] = "ALSOKNOWNAS";
    // Import tab
    json[recentlyImportedImages_token] = recentlyImportedImages;
    json[importFoldersRecursively_token] = importFoldersRecursively;
    json[requireOriginal_token] = requireOriginal;
    json[excludeLocalizer_token] = excludeLocalizer;
    json[excludePreContrast_token] = excludePreContrast;
    // Viewer tab
    json[targetDefineIntensityWindow_token] = targetDefineIntensityWindow; 

    QJsonDocument saveDoc = QJsonDocument(json);
    saveFile.write(saveDoc.toJson());
    saveFile.flush();
    saveFile.close();
  }

  hide();
}

void systemPreferences::reject()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // the user rejected any changes they may have made, so restore prior values
  assignValuesFromSaved();
  hide();
}
/** @} */
