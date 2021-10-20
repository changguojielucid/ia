// Copyright (c) Elucid Bioimaging
#ifndef SYSTEMPREFERENCES_H
#define SYSTEMPREFERENCES_H

#include "ebLog.h"

#include <QDialog>
#include <QStandardPaths>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileInfo>
#include <QMessageBox>

// General tab
#define encounterWindow_token "encounterWindow"
#define promptBeforeSave_token "promptBeforeSave"
#define displayExtractMessages_token "displayExtractMessages"
#define confirmWhenSetProcessingParameters_token "confirmWhenSetProcessingParameters"
#define presentCompositionControl_token "presentCompositionControl"
#define labelTabsWith_token "labelTabsWith"
// Import tab
#define recentlyImportedImages_token "recentlyImportedImages"
#define importFoldersRecursively_token "importFoldersRecursively"
#define requireOriginal_token "requireOriginal"
#define excludeLocalizer_token "excludeLocalizer"
#define excludePreContrast_token "excludePreContrast"
// Viewer tab
#define targetDefineIntensityWindow_token "targetDefineIntensityWindow"

enum field { WORKITEMID, PATIENTID, PATIENTNAME, SUBJECTID, ALSOKNOWNAS };

/**
 * @{ 
 ** 
 * @brief class systemPreferences to manage suer configuration settings
 *
 *  \copyright Elucid Bioimaging
 *  \ingroup cap
 */
  
namespace Ui {
class systemPreferences;
}

class systemPreferences : public QDialog
{
  Q_OBJECT

public:
  explicit systemPreferences(QWidget *owner = 0);
  ~systemPreferences();
  // General tab
  int   getEncounterWindow() { return encounterWindow; }
  bool  getPromptBeforeSave() { return promptBeforeSave; }
  bool  getConfirmationWhenSetProcessingParameters() { return confirmWhenSetProcessingParameters; }
  bool  getPresentCompositionControl() { return presentCompositionControl; }
  bool  getDisplayExtractMessages() { return displayExtractMessages; }
  field getLabelTabsWith() { return labelTabsWith; }
  // Import tab
  double getRecentlyImportedImages() const { return recentlyImportedImages; }
  bool   getImportFoldersRecursively() const { return importFoldersRecursively; }
  bool   getRequireOriginal() const { return requireOriginal; }
  bool   getExcludeLocalizer() const { return excludeLocalizer; }
  bool   getExcludePreContrast() const { return excludePreContrast; }
  // Viewer tab
  double getTargetDefineIntensityWindow() const { return targetDefineIntensityWindow; }

public slots:
  void accept();
  void reject();
  void displayPreferences();

private:
  Ui::systemPreferences *ui;
  QWidget *owner;
  // General tab
  int   encounterWindow;
  bool  promptBeforeSave;
  bool  confirmWhenSetProcessingParameters;
  bool  presentCompositionControl;
  bool  displayExtractMessages;
  field labelTabsWith;
  // Import tab
  double recentlyImportedImages;
  bool   importFoldersRecursively;
  bool   requireOriginal;
  bool   excludeLocalizer;
  bool   excludePreContrast;
  // Viewer tab
  double targetDefineIntensityWindow;
  
  void assignValuesFromSaved();
};

#endif // SYSTEMPREFERENCES_H
