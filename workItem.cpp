// Copyright (c) Elucid Bioimaging

#include "workItem.h"
#include "ui_workItem.h"
#include "cap.h"
#include "ebSystem.h"
#include "ebvOutputWindow.h"
#include "ebvEvent.h"
#include "ebiEventObject.h"
#include "dicomQueryRetrieve.h"
#include "ebAssert.h"

#include <QToolButton>
#include <QTableWidget>
#include <QDesktopWidget>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QErrorMessage>
#include <QPushButton>
#include <QStackedWidget>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSortFilterProxyModel>
#include <QMessageBox>
#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QStandardPaths>
#include <QDirIterator>
#include <QRegExp>
#include <QWidgetAction>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QAbstractButton>
#include <QTemporaryFile>

#include <string>
#include <iostream>

// Qt prior to 5.10 does not have an API to set the file time
#ifdef _WIN32
#include <windows.h>
#include <fileapi.h>
#include <handleapi.h>
#include <minwinbase.h>
#include <timezoneapi.h>
#else
#include <utime.h>
#endif

/**
 * \ingroup workItem
 * @{
 *
 * See workItem.h for description of the package purpose and contents.  This file has the member functions for classes in the package.
 */

/**
 * @page column data types and headers for the work item table view
 */
#define workItemPriority_col 0
#define workItemPriority_heading "Priority"

#define individualID_col 1
#define individualID_heading "Patient ID"

#define patientName_col 2
#define patientName_heading "Name"

#define subjectID_col 3
#define subjectID_heading "Subject ID"

#define alsoKnownAs_col 4
#define alsoKnownAs_heading "AKA"

#define indication_col 5
#define indication_heading "Indication"

#define appliesDate_col 6
#define appliesDate_heading "Applies Date"

#define timepoint_col 7
#define timepoint_heading "tp"

#define acqrep_col 8
#define acqrep_heading "acq"

#define dob_col 9
#define dob_heading "DOB"

#define age_col 10
#define age_heading "Age"

#define sex_col 11
#define sex_heading "Sex"

#define series_col 12
#define series_heading "Series"

#define targets_col 13
#define targets_heading "Targets"

#define lastStep_col 14
#define lastStep_heading "Last Update"

#define by_col 15
#define by_heading "by"

#define on_col 16
#define on_heading "on"

#define MAXCOL (17)

QString workitem_callback_data::uploadStateStr[] = {
    QString(""),
    QString("UPLOAD"),
    QString("FILES"),
    QString("META"),
    QString("POST"),
    QString("FINAL"),
    };

bool copyRecursively(const QString &srcFilePath, const QString &tgtFilePath)
{
  //ebLog eblog(Q_FUNC_INFO); eblog << (srcFilePath + " will be copied to " + tgtFilePath).toStdString() << std::endl;
  QString currentSave = QDir::currentPath();
  QFileInfo srcFileInfo(srcFilePath);
  if (srcFileInfo.isDir()) {
    QDir dir = QDir::root();
    if (!dir.mkpath(tgtFilePath)) {
      QDir::setCurrent(currentSave);
      return false;
    }
    QDir sourceDir(srcFilePath);
    QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
    foreach (const QString &fileName, fileNames) {
      const QString newSrcFilePath = srcFilePath + QLatin1Char('/') + fileName;
      const QString newTgtFilePath = tgtFilePath + QLatin1Char('/') + fileName;
      if (!copyRecursively(newSrcFilePath, newTgtFilePath)) {
        QDir::setCurrent(currentSave);
        return false;
      }
    }
  } else {
    if (!QFile::copy(srcFilePath, tgtFilePath)) {
      QDir::setCurrent(currentSave);
      return false;
    }
    // set the time from the original file
    setFileTime( srcFilePath, tgtFilePath );
  }
  QDir::setCurrent(currentSave);
  return true;
}

/**
 * @page workItemUpdate member functions
 */
workItemUpdate::workItemUpdate() :
  step("")
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  parametersForStage = NULL;
}

workItemUpdate::workItemUpdate(const QString &stepStr, const QString &IDStr, stageParameters *stageParams, const QString &performerName, const QString &performDateTimeStr) :
  step(stepStr),
  ID(IDStr),
  parametersForStage(stageParams),
  performer(performerName),
  performDateTime(performDateTimeStr)
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
}

/**
 * @page workItemListEntry member functions
 */
void workItemListEntry::readWorkItemListEntry(const QJsonObject &json) 
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  setWorkItemID(json[workItemID_token].toString());
  setWorkItemPriority(json[workItemPriority_token].toString());
  setSubjectType(json[subjectType_token].toString());
  setIndividualID(json[individualID_token].toString());
  setWorkItemFolder(json[workItemFolder_token].toString());
  setPatientName(json[patientName_token].toString());
  setDOB(json[dob_token].toString());
  setSex(json[sex_token].toString());
  setAppliesDate(json[appliesDate_token].toString());
  setAlsoKnownAs(json[alsoKnownAs_token].toString());
  setSubjectID(json[subjectID_token].toString());
  setIndication(json[indication_token].toString());
  setAge(json[age_token].toString());
  setModelOrganism(json[modelOrganism_token].toString());
  setTimepoint(json[timepoint_token].toInt());
  setAcqrep(json[acqrep_token].toInt());
  setAccession(json[accession_token].toString());
  
  images_cg.clear();
  QJsonArray seriesArray = json[imageSeriesSet_token].toArray();
  for (int seriesIndex = 0; seriesIndex < seriesArray.size(); ++seriesIndex) {
    QJsonObject seriesObject = seriesArray[seriesIndex].toObject();
    imageSeries *series = new imageSeries();
    series->readImageSeries(seriesObject);
    images_cg.append(*series);
    delete series;
  }

  targets.clear();
  QJsonArray targetArray = json[targetDefinitions_token].toArray();
  for (int targetIndex = 0; targetIndex < targetArray.size(); ++targetIndex) {
    QJsonObject targetObject = targetArray[targetIndex].toObject();
    targetDef *target = new targetDef();
    target->readTargetDef(targetObject);
    targets.append(*target);
    delete target;
  }

  updates.clear();
  QJsonArray updateArray = json[updates_token].toArray();
  for (int updateIndex = 0; updateIndex < updateArray.size(); ++updateIndex) {
    QJsonObject updateObject = updateArray[updateIndex].toObject();
    workItemUpdate *update = new workItemUpdate();
    update->readWorkItemUpdate(updateObject);
    updates.append(*update);
    delete update;
  }
}

void workItemListEntry::closeOutTargetsFromCompletedSession()
{
  ebLog eblog(Q_FUNC_INFO); eblog << getWorkItemID().toStdString() << std::endl;
  for (int k=0; k < targets.size(); k++) {
    targetDef *def = &((targets)[k]);
    def->closeOutCompletedSession();
  }
  dataSentToServer = false;
}

void workItemListEntry::writeWorkItemListEntry(QJsonObject &json) const 
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  json[workItemID_token] = getWorkItemID(); 
  json[workItemPriority_token] = getWorkItemPriority();
  json[subjectType_token] = getSubjectType();
  json[individualID_token] = getIndividualID();
  json[workItemFolder_token] = getWorkItemFolder();
  json[patientName_token] = getPatientName();
  json[dob_token] = getDOB();
  json[sex_token] = getSex(); 
  json[appliesDate_token] = getAppliesDate();
  json[alsoKnownAs_token] = getAlsoKnownAs();
  json[subjectID_token] = getSubjectID();
  json[indication_token] = getIndication();
  json[age_token] = getAge();
  json[modelOrganism_token] = getModelOrganism();
  json[timepoint_token] = getTimepoint();
  json[acqrep_token] = getAcqrep();
  json[accession_token] = getAccession();

  QJsonArray imageArray;
  foreach (const imageSeries image, images_cg) {
    QJsonObject imageObject;
    image.writeImageSeries(imageObject);
    imageArray.append(imageObject);
  }
  json[imageSeriesSet_token] = imageArray;

  QJsonArray targetArray;
  foreach (const targetDef target, targets) {
    QJsonObject targetObject;
    target.writeTargetDef(targetObject);
    targetArray.append(targetObject);
  }
  json[targetDefinitions_token] = targetArray;

  QJsonArray updateArray;
  foreach (const workItemUpdate update, updates) {
    QJsonObject updateObject;
    update.writeWorkItemUpdate(updateObject);
    updateArray.append(updateObject);
  }
  json[updates_token] = updateArray;
}

bool workItemListEntry::update(workItemListEntry *oentry)
{
    qInfo() << "update workitem " << workItemID;

    bool isUpdated = false;
    bool didUpdate = false;
    int indx = 0;

    qInfo() << "update images from " << oentry->images_cg.size() << " to " << images_cg.size();
    foreach (imageSeries image, images_cg) {
        if (indx < oentry->images_cg.size())
            didUpdate = image.updateSeries( &(oentry->images_cg[indx++]) );
        else {
            imageSeries *nseries = new imageSeries();
            image.updateSeries( nseries );
            oentry->images_cg.append( *nseries);
            didUpdate = true;
        }
        isUpdated = isUpdated || didUpdate;
    }

    qInfo() << "update targets from " << oentry->targets.size() << " to " << targets.size();
    indx = 0;
    didUpdate = false;
    foreach (targetDef target, targets) {
        if (indx < oentry->targets.size())
            didUpdate = target.updateTarget( &(oentry->targets[indx++]) );
        else {
            targetDef *ntarget = new targetDef();
            target.updateTarget(ntarget);
            oentry->targets.append( *ntarget);
            didUpdate = true;
        }
        isUpdated = isUpdated || didUpdate;
    }

    qInfo() << "update updates from " << oentry->updates.size() << " to " << updates.size();
    int ocount = oentry->updates.size();
    oentry->updates.clear();
    indx = 0;
    foreach (workItemUpdate wiupdate, updates){
        workItemUpdate *nupdate = new workItemUpdate();
        wiupdate.updateUpdate(nupdate);
        oentry->updates.append(*nupdate);
    }
    isUpdated = isUpdated || (ocount < oentry->updates.size());

    qInfo() << "update workitem " << workItemID << "  state is " << isUpdated;
    return isUpdated;
}

bool workItemListEntry::setWorkItemFolder() 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QString listDirectory = owner->getListDirectory();
  if (listDirectory.endsWith(WORKING_STORAGE_FOLDER)) {
    listDirectory.append(TEMPORARY_WORKITEM_FOLDER);
    owner->setListDirectory(listDirectory);
  }
  QDir::setCurrent(owner->getListDirectory());
  QString individualID = getIndividualID();
  workItemFolder = listDirectory+"/"+individualID+"/"+getWorkItemID();
  QDir wiDir(QDir::current());
  if (wiDir.mkpath(workItemFolder))
    return true;
  else
    return false;
}

void workItemListEntry::logWorkItemListEntryUpdate(QString step, QString ID, stageParameters *stageParams, workItem *workItemOwner) 
{
  QString label = step; label.append("|"); label.append(ID);
  ebLog eblog(Q_FUNC_INFO); eblog << label.toStdString() << std::endl;

  cap *capGrandOwner = workItemOwner->getOwner();
  QString name = capGrandOwner->getUserName();
  QString str = owner->getISODateTime();
  this->updates.append(workItemUpdate(step, ID, stageParams, name, str));
  this->unsavedChanges = true;
  workItemOwner->logListProvenanceUpdate();
}

void workItemListEntry::changeIndividualID(QString str)
{
  ebLog eblog(Q_FUNC_INFO); eblog << (individualID + " will be changed to " + str).toStdString() << std::endl;
  QString currentSave = QDir::currentPath();
  copyRecursively(owner->getListDirectory() + "/" + individualID + "/" + getWorkItemID(), 
                  owner->getListDirectory() + "/" + str + "/" + getWorkItemID());
  for (int i=0; i < targets.size(); i++) {
    targetDef *tgt = &(targets[i]);
    tgt->repointTargetFiles(individualID, str);
  }
  setIndividualID(str);
  QDir::setCurrent(currentSave);
  return;
}

/**
 * @page workItem member functions
 */
ImageDatabase *workItem::getImageDatabase()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  ebAssert(owner);
  return owner->getImageDatabase();
}

void workItem::setupModel()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  workItemListViewingModel *sourceModel = new workItemListViewingModel(this);
  QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(this);
  proxyModel->setSourceModel(sourceModel);
  ui->workItemList->setModel(proxyModel);
  ui->workItemList->verticalHeader()->setVisible(true);
  ui->workItemList->horizontalHeader()->setVisible(true);
  ui->workItemList->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  ui->workItemList->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
  connect(ui->workItemList->verticalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(workItemSelectedForAnalysis(int)));
  connect(ui->workItemList->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(workItemRowSelected(int)));
  source = sourceModel;
  proxy = proxyModel;
  ui->workItemList->setAcceptDrops(true);
}

void workItem::blankPreviewer()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  getImageDatabase()->SetSelectedSeriesPath("");
  ui->dicomImagesTreeWidget->clearSelection();
  ui->dicomMetadataTreeWidget->clear();
  if (image4ID != null_ebID) {
    viewers->RemoveImage4(image4ID);
    image4ID = null_ebID;
  }
  if (imageID != null_ebID) {
    multiReader->CloseImage(imageID);
    imageID = null_ebID;
  }
  multiReader = NULL;
  
  // ensure all screen changes take place
  this->repaint();
  qApp->processEvents();
  viewers->SyncViewersToScene();
  viewers->Render();
}

void workItem::updateWorkItemListNameText()
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (currentlyOpenWorkItemListItem != NULL) {
    if (getUnsavedChangesFlag())
      currentlyOpenWorkItemListItem->setText("*"+currentlyOpenWorkItemListItem->data(LIST_NAME_ROLE).toString()); // just a screen hint to user that the work item has unsaved changes
    else
      currentlyOpenWorkItemListItem->setText(currentlyOpenWorkItemListItem->data(LIST_NAME_ROLE).toString());
  }
}

QString workItem::getISODateTime()
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  return owner->getISODateTime();
}

QString workItem::getListFileName()
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  return getListProvenance()->getListFileName();
}

void workItem::populateWorkItemLists(QString itemToLeaveSelected)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  ui->workItemLists->clear();

  QString examDataPath = cap::getCAPExamDataDir();
  QDirIterator it(examDataPath, QStringList() << "wilist*.json", QDir::Files, QDirIterator::Subdirectories);
  while (it.hasNext()) {
    QString fullListName = it.next();
    QStringList splitted = fullListName.split("/");
    QString listName = splitted.last().replace("wilist_","").replace(".json","");
    if (!listName.contains(QRegExp("[a-zA-Z0-9]_20[0-9]{6}-[0-9]{4}"))) { // this long regular expression removes the backup lists which would otherwise be included
      QListWidgetItem *item = new QListWidgetItem(listName, ui->workItemLists);
    
      QVariant fullListNameVariant;
      fullListNameVariant.setValue(fullListName);
      item->setData(Qt::ToolTipRole, fullListNameVariant); // by storing it in the tool tip role, can always see the full path by hovering over it

      QVariant listNameVariant;
      listNameVariant.setValue(listName);
      item->setData(LIST_NAME_ROLE, listNameVariant); // we keep this because the actual text at any given time might be prefixed with *, denoting unsaved changes

      // finally, intiailize the provenance fields
      QVariant blankStringVariant;
      blankStringVariant.setValue(QString(""));
      item->setData(PERFORMER_ROLE, blankStringVariant);
      item->setData(PERFORM_DATETIME_ROLE, blankStringVariant);
      item->setData(LAST_MODIFIER_ROLE, blankStringVariant);
      item->setData(LAST_MODIFIED_DATETIME_ROLE, blankStringVariant);
      item->setData(NUM_ENTRIES_ROLE, blankStringVariant);
    }
  }
  ui->workItemLists->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->workItemLists, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showWorkItemListContextMenu(QPoint)));
  if (itemToLeaveSelected == "")
    currentlyOpenWorkItemListItem = NULL; 
  else {
    for (int i=0; i < ui->workItemLists->count(); i++) {
      if ((ui->workItemLists->item(i)->data(Qt::ToolTipRole).toString()) == itemToLeaveSelected) {
        ui->workItemLists->item(i)->setSelected(true);
        currentlyOpenWorkItemListItem = ui->workItemLists->item(i);
      }
    }
    ENABLECONTROL(ui->saveListChangesButton, saveListChangesAction, tr("Press to save any changes (or to save to another list name)"));
  }
}

workItem::workItem(QWidget *p, int index, QMenu *m, bool masterWithRespectToMenu) :
  QWidget(p),
  ui(new Ui::workItem) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner = dynamic_cast<cap *>(p);
  workItemMenu = m;
  selectedWorkItemValid = false;
  seriesSetBoxLayout = NULL;
  menuActionEnabledMap = new std::map<QAction *, bool>;
  allowableSeriesTypes = QList<QString>()
           << tr("CT")
           << tr("MRA")
           << tr("DCE")
           << tr("DWI")
           << tr("FIESTA")
           << tr("PC")
           << tr("T1w")
           << tr("T2w")
           << tr("TOF")
           << tr("DelayedEnhancement")
           << tr("DualEchoIn")
           << tr("DualEchoOut")
           << tr("US")
           << tr("IVUS")
           << tr("XA")
          ;

  if (masterWithRespectToMenu) {
    addWorkItemAction = new QAction(tr("Generate a new empty work item on the current list"), this);
    addWorkItemAction->setObjectName("addWorkItem");
    workItemMenu->addAction(addWorkItemAction);

    proceedAction = new QAction(tr("Process selected work item"), this);
    proceedAction->setObjectName("proceed");
    workItemMenu->addAction(proceedAction);

    saveListChangesAction = new QAction(tr("Save work item list data to disk"), this);
    saveListChangesAction->setObjectName("saveListChanges");
    workItemMenu->addAction(saveListChangesAction);
  } 
  else if (owner != 0) {
    foreach (QAction *a, workItemMenu->actions()) {
      if (a->objectName() == "addWorkItem") {
        addWorkItemAction = a;
        (*menuActionEnabledMap)[a] = true;
        a->setEnabled(true);
      }
      else if (a->objectName() == "proceed") {
        proceedAction = a;
        (*menuActionEnabledMap)[a] = false;
        a->setEnabled(false);
      }
      else if (a->objectName() == "saveListChanges") {
        saveListChangesAction = a;
        (*menuActionEnabledMap)[a] = false;
        a->setEnabled(false);
      }
    }
    connectMenuActions();
  }

  if ((owner != 0) && (masterWithRespectToMenu == false)) {
    ui->setupUi(this);
    workItemMenu = m;
    message = new QErrorMessage (this);
    message->setWindowFlags(message->windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    message->setModal(true);
    sessionItemIndex = index;
    populateWorkItemLists(getListProvenance()->getListFileName()); 
    connect(ui->createWorkItemListButton, SIGNAL(clicked()), owner, SLOT(saveAs()));
    connect(ui->saveListChangesButton, SIGNAL(clicked()), owner, SLOT(save()));
    updateWorkItemListNameText();

    // initialize imported images tree widget
    populateImportedDICOMImages();
    ui->dicomImagesTreeWidget->setColumnHidden(DICOM_IMAGES_COLUMN_TYPE,true);         // hide the type column
    ui->dicomImagesTreeWidget->setColumnHidden(DICOM_IMAGES_COLUMN_PATH,true);         // hide the path column
    ui->dicomImagesTreeWidget->setColumnHidden(DICOM_IMAGES_COLUMN_FIRST_IMAGE,true);  // hide the first image column
    ui->dicomImagesTreeWidget->setColumnWidth(DICOM_IMAGES_COLUMN_DESCRIPTION,400);
    ui->dicomImagesTreeWidget->setDragEnabled(false); // can't drag until there is a selected series, and that series has had its headers read in so that DICOM meta data is available
    ui->workItemList->showDropIndicator();
    connect(ui->workItemList, SIGNAL(seriesDropped()), this, SLOT(useSeriesInCurrentWorkItemList()));
    ui->seriesSetBox->hide();

    if (currentlyOpenWorkItemListItem != NULL) {
      QVariant performerVariant;
      performerVariant.setValue(getListProvenance()->getPerformer());
      currentlyOpenWorkItemListItem->setData(PERFORMER_ROLE, performerVariant); 

      QVariant performerDateTimeVariant;
      performerDateTimeVariant.setValue(getListProvenance()->getPerformDateTime());
      currentlyOpenWorkItemListItem->setData(PERFORM_DATETIME_ROLE, performerDateTimeVariant); 

      QVariant lastModifierVariant;
      lastModifierVariant.setValue(getListProvenance()->getLastModifier());
      currentlyOpenWorkItemListItem->setData(LAST_MODIFIER_ROLE, lastModifierVariant); 

      QVariant lastModifiedDateTimeVariant;
      lastModifiedDateTimeVariant.setValue(getListProvenance()->getLastModifiedDateTime());
      currentlyOpenWorkItemListItem->setData(LAST_MODIFIED_DATETIME_ROLE, lastModifiedDateTimeVariant);       

      QVariant numEntriesVariant;
      numEntriesVariant.setValue(QString::number(workItemList()->size()));
      currentlyOpenWorkItemListItem->setData(NUM_ENTRIES_ROLE, numEntriesVariant); 
    }

    // set up the preliminary model view as well as display aspects of workItemList
    setupModel();
    ui->workItemList->setWordWrap(true);
    ui->workItemList->setTextElideMode(Qt::ElideMiddle);
    ui->workItemList->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ENABLECONTROL(ui->saveListChangesButton, saveListChangesAction, tr("Press to save any changes (or to save to another list name)"));

    // connect for the network replies
    connect(&GetAuthorizationTokenITI_71, SIGNAL(finished(QNetworkReply*)), this, SLOT(parseReportAuthorizationTokenITI_71reply(QNetworkReply*)));
    connect(&UpdateUPSWorkitemRAD, SIGNAL(finished(QNetworkReply*)), this, SLOT(parseUPSWorkitemContentsRADreply(QNetworkReply*)));

    // set up the initial viewers
    viewers = vtkSmartPointer<ebvLinkedViewers2>::New();
    ida = viewers->AddViewer(ebvViewer::AXIAL,ui->perspective0render->GetRenderWindow());
    idc = viewers->AddViewer(ebvViewer::CORONAL,ui->perspective1render->GetRenderWindow());
    ids = viewers->AddViewer(ebvViewer::SAGITTAL,ui->perspective2render->GetRenderWindow());
    viewers->GetViewer(ida)->SetShowLogo(true);
    viewers->GetViewer(idc)->SetShowLogo(true);
    viewers->GetViewer(ids)->SetShowLogo(true);
    viewers->SetScreenScaleFitFactor2D(1.0);
    viewers->OptimizeScreenScale2D();
    vtkOutputWindow::SetInstance(ebvOutputWindow::New());
    viewers->SyncViewersToScene();
    viewers->Render();
    image4ID = null_ebID;
    imageID = null_ebID;
    
    // format the proceed button
    QString buttonText = tr("Process Work Item ");
    buttonText.append(QChar(0x25B6));
    ui->proceedButton->setText(buttonText);
    DISABLECONTROL(ui->proceedButton, proceedAction, tr("Select a work item to process first"));
    ui->deleteWorkItemButton->setEnabled(false);
    ui->seriesSetBox->hide();

    // populate the initial list of sources
    ui->sources->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->sources, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showSourceContextMenu(QPoint)));

    // get file system import sources from disk (create default if none exist)
    importSources.push_back(std::make_shared<ImportSource>());
    importSources.back()->item = new QListWidgetItem(DEFAULT_FILESYSTEM_SOURCE, ui->sources, TYPE_FILESYSTEM_SOURCE);
    importSources.back()->config = new sourceConfiguration(DEFAULT_FILESYSTEM_SOURCE, ui->sources, TYPE_FILESYSTEM_SOURCE);    
    QDir fsdir(cap::getConfigurationFileSystemSourcesDir());
    fsdir.setFilter(QDir::Files);
    QFileInfoList fsInfoList = fsdir.entryInfoList();
    for (int i = 0; i < fsInfoList.size(); ++i) {
      QString name = fsInfoList.at(i).fileName();
      name.chop(5);  // remove ".json"
      if (name != DEFAULT_FILESYSTEM_SOURCE) {
              importSources.push_back(std::make_shared<ImportSource>());
              importSources.back()->item = new QListWidgetItem(name, ui->sources, TYPE_FILESYSTEM_SOURCE);
              importSources.back()->config = new sourceConfiguration(name, ui->sources, TYPE_FILESYSTEM_SOURCE);
      }
    }

    // get PACS import sources from disk (create default if none exist)
    importSources.push_back(std::make_shared<ImportSource>());
    importSources.back()->item = new QListWidgetItem(DEFAULT_PACS_SOURCE, ui->sources, TYPE_PACS_SOURCE);
    importSources.back()->config = new sourceConfiguration(DEFAULT_PACS_SOURCE, ui->sources, TYPE_PACS_SOURCE);
    importSources.back()->queryRetrieve = new dicomQueryRetrieve(this);
    QDir pdir(cap::getConfigurationPACSSourcesDir());
    pdir.setFilter(QDir::Files);
    QFileInfoList pInfoList = pdir.entryInfoList();
    for (int i = 0; i < pInfoList.size(); ++i) {
      QString name = pInfoList.at(i).fileName();
      name.chop(5);  // remove ".json"
      if (name != DEFAULT_PACS_SOURCE) {
              importSources.push_back(std::make_shared<ImportSource>());
              importSources.back()->item = new QListWidgetItem(name, ui->sources, TYPE_PACS_SOURCE);
              importSources.back()->config = new sourceConfiguration(name, ui->sources, TYPE_PACS_SOURCE);      
              importSources.back()->queryRetrieve = new dicomQueryRetrieve(this);
      }
    }

    // and set the product logo
    if (owner->thisProduct == "vascuCAP") 
      ui->productLogo->setPixmap(QPixmap(QString::fromUtf8(":/cap/screenPixMaps/ElucidVivoLogotransp-for-dark.png")));
    else if (owner->thisProduct == "heartCAP")
      ui->productLogo->setPixmap(QPixmap(QString::fromUtf8(":/cap/screenPixMaps/heartCAPtransp-for-dark.png")));
    else if (owner->thisProduct == "kidneyCAP")
      ui->productLogo->setPixmap(QPixmap(QString::fromUtf8(":/cap/screenPixMaps/kidneyCAPtransp-for-dark.png")));
    else if (owner->thisProduct == "liverCAP")
      ui->productLogo->setPixmap(QPixmap(QString::fromUtf8(":/cap/screenPixMaps/liverCAPtransp-for-dark.png")));
    else if (owner->thisProduct == "lungCAP")
      ui->productLogo->setPixmap(QPixmap(QString::fromUtf8(":/cap/screenPixMaps/lungCAPtransp-for-dark.png")));
    ui->productLogo->setMinimumSize(QSize(200, ui->productLogo->height()));
    ui->productLogo->setMaximumSize(QSize(200, ui->productLogo->height()));
    ui->productLogo->setScaledContents(true);
  }
}

workItem::~workItem() 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (owner != 0) {
    // shut off the view update
    disconnect(ui->workItemList->verticalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(workItemSelectedForAnalysis(int)));
    disconnect(ui->workItemList->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(workItemRowSelected(int)));
    delete source;
    delete proxy;

    // take the viewers down (after checking that they aren't in the middle of a load)
    viewers->RemoveViewer(ida);
    viewers->RemoveViewer(idc);
    viewers->RemoveViewer(ids);
    viewers = NULL;

    ui->sources->clear();  // delete items owned by this QListWidget (pointed to from importSources)
    //sourceConfigurations.clear();
    //delete dicomQueryRetrieveObject;
    delete currentlyOpenWorkItemListItem;
    delete message;
    delete ui;
  } // end-if owner isn't 0
  /*else
    workItemList()->clear();*/
}

void workItem::disconnectMenuActions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  disconnect(workItemMenu, SIGNAL(triggered(QAction *)), this, SLOT(ensureOnWorkItemPage()));
  disconnect(addWorkItemAction, SIGNAL(triggered()), this, SLOT(on_addWorkItemButton_clicked()));
  disconnect(proceedAction, SIGNAL(triggered()), this, SLOT(on_proceedButton_clicked()));
  disconnect(saveListChangesAction, SIGNAL(triggered()), owner, SLOT(save()));
  workItemMenu->setEnabled(false);
}

void workItem::connectMenuActions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  connect(workItemMenu, SIGNAL(triggered(QAction *)), this, SLOT(ensureOnWorkItemPage()));
  connect(addWorkItemAction, SIGNAL(triggered()), this, SLOT(on_addWorkItemButton_clicked()));
  connect(proceedAction, SIGNAL(triggered()), this, SLOT(on_proceedButton_clicked()));
  connect(saveListChangesAction, SIGNAL(triggered()), owner, SLOT(save()));
  workItemMenu->setEnabled(true);
}

void workItem::reconnectMenuActions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  connectMenuActions();
  for (auto actionEnabled : *menuActionEnabledMap)
    actionEnabled.first->setEnabled(true);
}

void workItem::disableMenuActions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  for (auto actionEnabled : *menuActionEnabledMap)
    actionEnabled.first->setEnabled(false);
  workItemMenu->setEnabled(false);
}

void workItem::ensureOnWorkItemPage()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(WORKITEM);
}

void workItem::on_workItemList_clicked(const QModelIndex &index) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  selectedWorkItem = workItemList()->size(); 
  selectedWorkItemValid = false;
};

workItemListProvenance *workItem::getListProvenance() 
{
  /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ 
  return owner->getListProvenance(); 
}

void workItem::setListProvenance(workItemListProvenance prov) 
{ 
  /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ 
  owner->setListProvenance(prov);
}
  
int workItem::getOpenAnalysisCount() 
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  return owner->getOpenAnalysisCount(); 
}

void workItem::setOpenAnalysisCount(int count) 
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->setOpenAnalysisCount(count); 
}

void workItem::workItemRowSelected(int i) 
{ 
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; 
  workItemSelected(i);
}

void workItem::workItemSelected(int i) 
{ 
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; 
  
  seriesFolders.clear(); // don't want the event handler to fire
  seriesTypes.clear();
    
  // find the one selected (taking into account the prox sort order)
  int j=0;
  for (j=0; j < workItemList()->size(); j++) {
    if ((*workItemList())[j].getWorkItemID() 
        == ((QSortFilterProxyModel *)proxy)->headerData(i, Qt::Vertical, Qt::DisplayRole).toString().remove("*"))
      break;
  }
  if (j < workItemList()->size()) {
    selectedWorkItem = j; 
    selectedWorkItemValid = true;  
    ui->workItemList->selectRow(i);
    ENABLECONTROL(ui->proceedButton, proceedAction, tr("Press to process selected work item")); // enable now that one is selected
    ui->deleteWorkItemButton->setEnabled(true);
    ui->seriesSetBox->show();
  }
  else  {
    DISABLECONTROL(ui->proceedButton, proceedAction, tr("Select a work item to process first"));
    ui->deleteWorkItemButton->setEnabled(false);
    ui->seriesSetBox->hide();
  }
  ui->workItemList->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  // now we synchronize the work item table selection, image selector/previewer selection, and series set selection.  We do this in multiple steps: 
  // 1. if j indicates that there is no selected work item, hide the series set box and return without assessing consistency.
  // . populate the series list by getting rid of the prior one and replacing it with a newly built one
  // 3. see if both selections are compatible, and if so, return without changing selections. 
  // 4. having arrived at this step, blank the image widgets if the selected work item doesn't include the series which is selected in them. 
  // 5. finally, if arrive here and if the work item has at least one series, select it for the previewer. 

  // step 1: 
  if (j == workItemList()->size()) {
    ui->seriesSetBox->hide();
    return;
  }

  // step 2:
  // ...out with the old series set box:
  delete ui->seriesSetBox;
  QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Minimum);
  sizePolicy2.setHorizontalStretch(0);
  sizePolicy2.setVerticalStretch(0);
  ui->seriesSetBox = new QGroupBox(ui->widget_4);
  ui->seriesSetBox->setObjectName(QStringLiteral("seriesSetBox"));
  sizePolicy2.setHeightForWidth(ui->seriesSetBox->sizePolicy().hasHeightForWidth());
  ui->seriesSetBox->setSizePolicy(sizePolicy2);
  ui->seriesSetBox->setMaximumSize(QSize(500, 16777215));
  ui->gridLayout->addWidget(ui->seriesSetBox, 0, 2, 1, 1);
  ui->seriesSetBox->setTitle(tr("Series Set for Selected Workitem"));

  // ...and in with the new series set box.
  seriesSetBoxLayout = new QVBoxLayout(ui->seriesSetBox);
  seriesSetBoxLayout->setObjectName(QStringLiteral("seriesSetBoxLayout"));
  seriesFolders.clear();
  seriesTypes.clear();
  workItemListEntry *wi = &(*workItemList())[j];
  if (wi->images_cg.size() == 0) {
    seriesSetEntryLayout = new QFormLayout(ui->seriesSetBox);
    seriesSetEntryLayout->setObjectName(QStringLiteral("seriesSetEntryLayout"));
    seriesSetEntryLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
    seriesSetBoxLayout->addLayout(seriesSetEntryLayout, 0);
    QLabel *emptySeriesSetLabel = new QLabel(ui->seriesSetBox);
    emptySeriesSetLabel->setText(tr("(no series' have been added to work item yet)"));
    seriesSetEntryLayout->setWidget(0, QFormLayout::LabelRole, emptySeriesSetLabel);
  }
  else {
    for (int seriesIndex=0; seriesIndex < wi->images_cg.size(); seriesIndex++) {
      imageSeries *series = &((wi->images_cg)[seriesIndex]);
      if (seriesIndex == 0) {
        seriesSetEntryLayout = new QFormLayout(ui->seriesSetBox);
        seriesSetEntryLayout->setObjectName(QStringLiteral("seriesSetEntryLayout"));
        seriesSetEntryLayout->setFieldGrowthPolicy(QFormLayout::FieldsStayAtSizeHint);
        seriesSetBoxLayout->addLayout(seriesSetEntryLayout, 0);
      }

      seriesFolders << new QLineEdit(ui->seriesSetBox);
      QDir seriesFolderDir(series->seriesFolder);
      if (seriesFolderDir.exists()) {
        QFileInfo imageFileInfo = seriesFolderDir.entryInfoList().last();
        seriesFolders[seriesIndex]->setText(imageFileInfo.absolutePath());
        seriesFolders[seriesIndex]->setFrame(false);
        seriesFolders[seriesIndex]->setReadOnly(true);
        QSizePolicy folderSizePolicy = seriesFolders[seriesIndex]->sizePolicy(); //(QSizePolicy::Expanding, QSizePolicy::Preferred);
        folderSizePolicy.setHorizontalStretch(1);
        seriesFolders[seriesIndex]->setSizePolicy(folderSizePolicy);
        seriesSetEntryLayout->setWidget(seriesIndex, QFormLayout::LabelRole, seriesFolders[seriesIndex]);
        seriesFolders[seriesIndex]->resize(230, seriesFolders[seriesIndex]->height());
        /*test*/seriesFolders[seriesIndex]->setStyleSheet("QLineEdit { background: Qt::magenta}");
      }
      else
        message->showMessage(tr("Error: Series folder can not be opened (contact Elucid if condition persists)"));

      seriesTypes << new QComboBox(ui->seriesSetBox);
      foreach (QString seriesType, allowableSeriesTypes) 
        seriesTypes[seriesIndex]->addItem(seriesType);
      seriesTypes[seriesIndex]->setCurrentIndex(seriesTypes[seriesIndex]->findText(series->seriesType));
      seriesSetEntryLayout->setWidget(seriesIndex, QFormLayout::FieldRole, seriesTypes[seriesIndex]);
      seriesTypes[seriesIndex]->installEventFilter(this); // adds linkage to allow changes
    }
  }

  // step 3: see if the selections are consistent:
  bool selectionsAreConsistent = false;
  for (int k=0; k < wi->images_cg.size(); k++) {
    imageSeries *series = &((wi->images_cg)[k]);
    if (series->seriesFolder == getImageDatabase()->GetSelectedSeriesPath(ui->dicomImagesTreeWidget)) {
      selectionsAreConsistent = true;
    }
  }
  if (selectionsAreConsistent) {
    return;
  }
  
  // step 4: need to blank the previewer if the series in the previewer isn't included in the selected wi
  bool previewInSelectedWI = false;
  for (int k=0; k < wi->images_cg.size(); k++) {
    imageSeries *series = &((wi->images_cg)[k]);
    if (series->seriesFolder == getImageDatabase()->GetSelectedSeriesPath(ui->dicomImagesTreeWidget))
      previewInSelectedWI = true;
  }
  if (!previewInSelectedWI) {
    blankPreviewer();
  }

  // step 5: see if can select any series
  for (int k=0; k < wi->images_cg.size(); k++) {
    imageSeries *series = &((wi->images_cg)[k]);
    QDir seriesFolderDir(series->seriesFolder);
    if (seriesFolderDir.exists()) {
      QFileInfo imageFileInfo = seriesFolderDir.entryInfoList().last();
      QString seriesFolderAbsolute = imageFileInfo.absolutePath();
      QTreeWidgetItemIterator it(ui->dicomImagesTreeWidget);
      while (*it) {
        if ((*it)->text(DICOM_IMAGES_COLUMN_TYPE) == "SERIES") {
          if (seriesFolderAbsolute == (*it)->text(DICOM_IMAGES_COLUMN_PATH)) {
            on_dicomImagesTreeWidget_itemPressed((*it));
            getImageDatabase()->SetSelectedSeriesPath((*it)->text(DICOM_IMAGES_COLUMN_PATH));
            ui->workItemList->selectRow(i); // the above will actually de-select our wi!  Need to put it back.
            return;
          }
        }
        ++it;
      }
    }
    else
      message->showMessage(tr("Error: Series folder can not be opened (contact Elucid if condition persists)"));
  }
}

void workItem::workItemSelectedForAnalysis(int i) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << std::to_string(i) << std::endl;
  workItemListEntry *entry;
  blankPreviewer();

  // find the one selected (taking into account the prox sort order)
  int j=0;
  for (j=0; j < workItemList()->size(); j++) {
    if ((*workItemList())[j].getWorkItemID() 
        == ((QSortFilterProxyModel *)proxy)->headerData(i, Qt::Vertical, Qt::DisplayRole).toString().remove("*"))
      break;
  }
  if (j < workItemList()->size()) {
    entry = &(*workItemList())[j];
    if (owner->userLoggedInToServer) {
      // now that the client GUI is proceeding with the work item processing, in the background we also transfer data to CAPgraph so that it can start 
      // performing the processing that will result in the report, taking advantage of the opportunity to work in parallel so that the user perceives 
      // the minimum possible wait times in the event they choose to generate a report later.
      if (!entry->setWorkItemFolder()) { // this may be the first time an operation that requires the folder has been done
        message->showMessage(tr("Error: Cannot write work item folder."));
        return;
      }
      QString wiFolder = entry->getWorkItemFolder();
      if (entry->unsavedChanges) {
        prepChangedDataForServer(wiFolder, entry);
      } else
        shipDataToServer(wiFolder, entry, false);
    }
    owner->selectWorkItemForAnalysis(entry, sessionItemIndex);
  }
}

void workItem::prepChangedDataForServer(QString wiFolder, workItemListEntry *entry)
{
  ebLog eblog(Q_FUNC_INFO); eblog << wiFolder.toStdString() << std::endl;
  QString currentSave = QDir::currentPath();

  workItemListEntry *wi = entry->clone();

  // in this case, we know by definition that there are unsaved changes, so we must make a temp folder
  // starting from the normal work item folder but which we then update based on the changes made.
  // The reason we must do this is because even though the user hasn't yet made the decision to make the
  // save the chnages permanently, in the eventuality that they request a report, it will be on the basis
  // of the current data rather than whatever may have been last saved that they will do so so we need to
  // ensure that any report works with the most up to date data rather than only rely on what may
  // have already been formally saved.

  QDateTime dt = QDateTime::currentDateTime();
  QString str = dt.toString("yyyyMMdd-hhmmsszzz");
  QString workingStoragePath = cap::getCAPExamDataDir()+WORKING_STORAGE_FOLDER;
  QString wiTmpName = workingStoragePath + TEMPORARY_WORKITEM_FOLDER + "/" + wi->getWorkItemID() + "_" + str;
  QString wiDest = wiTmpName + "/" + wi->getIndividualID() + "/" + wi->getWorkItemID();
  copyRecursively(wiFolder, wiDest);
  // (see efficiency note below)

  // loop over targets to make sure anything that may not yet be saved for them is in the temp folder
  // we have to do all of them, not just this one, since we don't keep these temp folders. 
  // An optimization could be to somehow keep these temp folders during work item processing and not have to 
  // re-do this step but that it is unclear if the time saved is worth the complexity so we just suffer
  // the redundancy
  for (int i=0; i < wi->targets.size(); i++) {
    targetDef *tgt = &(wi->targets[i]);
    QString tgtTmpName = wiTmpName + "/" + tgt->getTargetFolder();
    QDir *tgtTmpDir = new QDir(tgtTmpName);
    tgtTmpDir->mkpath(tgtTmpName);
    tgt->saveTargetDerivedDataChanges(wiTmpName);
  }
  QDir::setCurrent(currentSave);
  shipDataToServer(wiDest, wi, false);
}

void workItem::populateImportedDICOMImages() {
  double importDays = std::numeric_limits<double>::max();
  if (ui->recentlyImportedImages->isChecked())
    importDays = owner->systemPreferencesObject->getRecentlyImportedImages();
  getImageDatabase()->PopulateDICOMImagesTreeWidget(ui->dicomImagesTreeWidget,importDays);
}

QList<workItemListEntry> *workItem::workItem::workItemList() 
{ 
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  return owner->workItemList(); 
}

bool workItem::getUnsavedChangesFlag() 
{ 
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  return owner->getUnsavedChangesFlag(); 
}

void workItem::setUnsavedChangesFlag(bool flag) 
{ 
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->setUnsavedChangesFlag(flag); 
}

void workItem::logListProvenanceUpdate() 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  QString name = owner->getUserName();
  getListProvenance()->setLastModifier(name);
  QString str = getISODateTime();
  getListProvenance()->setLastModifiedDateTime(str);
  if (currentlyOpenWorkItemListItem != NULL) {
    QVariant lastModifierVariant;
    lastModifierVariant.setValue(getListProvenance()->getLastModifier());
    currentlyOpenWorkItemListItem->setData(LAST_MODIFIER_ROLE, lastModifierVariant); 

    QVariant lastModifiedDateTimeVariant;
    lastModifiedDateTimeVariant.setValue(getListProvenance()->getLastModifiedDateTime());
    currentlyOpenWorkItemListItem->setData(LAST_MODIFIED_DATETIME_ROLE, lastModifiedDateTimeVariant); 
  }
  if (getListProvenance()->getPerformer() == "") {
    getListProvenance()->setPerformer(name);
    getListProvenance()->setPerformDateTime(str);
    if (currentlyOpenWorkItemListItem != NULL) {
      QVariant performerVariant;
      performerVariant.setValue(getListProvenance()->getPerformer());
      currentlyOpenWorkItemListItem->setData(PERFORMER_ROLE, performerVariant); 

      QVariant performerDateTimeVariant;
      performerDateTimeVariant.setValue(getListProvenance()->getPerformDateTime());
      currentlyOpenWorkItemListItem->setData(PERFORM_DATETIME_ROLE, performerDateTimeVariant); 
    }
  }
  setUnsavedChangesFlag(true);
  updateWorkItemListNameText(); // to get the * mark on it
}

bool workItem::loadWorkItemListFromFile(QString fileName, saveFormat format) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << fileName.toStdString() << std::endl;
  QFile loadFile(fileName);
  QFileInfo loadFileInfo(fileName);

  wilist *witem = getWIlist(owner, fileName);

  if (!witem->trylock()) {
      message->showMessage(tr("Warning: Cannot open work item list file for loading, update is in progress"));
      return false;
  }

  if (!loadFile.open(QIODevice::ReadOnly)) {
    message->showMessage(tr("Warning: Cannot open work item list file for loading."));
    witem->unlock();
    return false;
  }

  QFileInfo listFileInfo(fileName);
  owner->setListDirectory(listFileInfo.dir().path());

  workItemList()->clear();
  disconnect(ui->workItemList->verticalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(workItemSelectedForAnalysis(int)));
  disconnect(ui->workItemList->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(workItemRowSelected(int)));
  delete source;
  delete proxy;

  QByteArray saveData = loadFile.readAll();
  witem->unlock();

  QJsonDocument loadDoc(format == Json
    ? QJsonDocument::fromJson(saveData)
    : QJsonDocument::fromBinaryData(saveData));

  readWorkItemList(loadDoc.object());

  // reset up the model view
  setupModel();
  setUnsavedChangesFlag(false);
  ui->workItemList->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

  // finally, switch to the directory so that workitems open correctly, record the name, set button states, etc.
  QDir::setCurrent(loadFileInfo.absolutePath());
  getListProvenance()->setListFileName(loadFileInfo.absoluteFilePath());
  updateWorkItemListNameText();
  DISABLECONTROL(ui->proceedButton, proceedAction, tr("Select a work item to process first")); // any prior selection would of course be cleared when loading a new list
  ENABLECONTROL(ui->saveListChangesButton, saveListChangesAction, tr("Press to save any changes (or to save to another list name)"));
  ENABLECONTROL(ui->addWorkItemButton, addWorkItemAction, tr("Press to save any changes (or to save to another list name)"));
  ui->deleteWorkItemButton->setEnabled(false);
  ui->seriesSetBox->hide();
  blankPreviewer();
  return true;
}

void workItem::setEntryUnsavedChangesFlag(int entryIndex, bool flag) 
{ 
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  workItemListEntry *wi = &(*workItemList())[entryIndex];
  wi->unsavedChanges = flag; 
}

unsigned int workItem::importImages(QString path, bool recursive, bool requireOriginal, bool excludeLocalizer, bool excludePreContrast, bool showProgress)  
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  ui->dicomImagesTreeWidget->setEnabled(false);
  int numImported = getImageDatabase()->ImportImages(path,recursive,requireOriginal,excludeLocalizer,excludePreContrast,showProgress);
  ui->dicomImagesTreeWidget->setEnabled(true);
  populateImportedDICOMImages();
  return numImported;
}

void workItem::readWorkItemList(const QJsonObject &json) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  getListProvenance()->readWorkItemListProvenance(json[listProvenance_token].toObject());
  if (currentlyOpenWorkItemListItem != NULL) {
    QVariant performerVariant;
    performerVariant.setValue(getListProvenance()->getPerformer());
    currentlyOpenWorkItemListItem->setData(PERFORMER_ROLE, performerVariant); 

    QVariant performerDateTimeVariant;
    performerDateTimeVariant.setValue(getListProvenance()->getPerformDateTime());
    currentlyOpenWorkItemListItem->setData(PERFORM_DATETIME_ROLE, performerDateTimeVariant); 

    QVariant lastModifierVariant;
    lastModifierVariant.setValue(getListProvenance()->getLastModifier());
    currentlyOpenWorkItemListItem->setData(LAST_MODIFIER_ROLE, lastModifierVariant); 

    QVariant lastModifiedDateTimeVariant;
    lastModifiedDateTimeVariant.setValue(getListProvenance()->getLastModifiedDateTime());
    currentlyOpenWorkItemListItem->setData(LAST_MODIFIED_DATETIME_ROLE, lastModifiedDateTimeVariant); 
  }

  workItemList()->clear();
  QJsonArray wiArray = json[workItemList_token].toArray();
  for (int wiIndex = 0; wiIndex < wiArray.size(); ++wiIndex) {
    workItemListEntry *wi = new workItemListEntry(owner);
    wi->wiObjectFromFile = new QJsonObject(wiArray[wiIndex].toObject());
    wi->readWorkItemListEntry(*wi->wiObjectFromFile);
    wi->unsavedChanges = false;
    workItemList()->append(*wi);
  }

  if (currentlyOpenWorkItemListItem != NULL) {
    QVariant numEntriesVariant;
    numEntriesVariant.setValue(QString::number(workItemList()->size()));
    currentlyOpenWorkItemListItem->setData(NUM_ENTRIES_ROLE, numEntriesVariant); 
  }
}

bool wellFormed(QString appliesDate)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  bool ok;
  appliesDate.left(4).toInt(&ok);
  if (!ok)
    return false;
  appliesDate.mid(5,2).toInt(&ok);
  if (!ok)
    return false;
  appliesDate.mid(8,2).toInt(&ok);
  if (!ok)
    return false;
  appliesDate.mid(11,2).toInt(&ok);
  if (!ok)
    return false;
  appliesDate.mid(14,2).toInt(&ok);
  if (!ok)
    return false;
  //(tolerate seconds being wrong)
  return true;
}

QDate dateObject(QString appliesDate)
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  int year = appliesDate.left(4).toInt();
  int month = appliesDate.mid(5,2).toInt();
  int day = appliesDate.mid(8,2).toInt();
  QDate thisEncounter = QDate(year, month, day);
  qInfo() << "date=" << thisEncounter << " is being considered in setting tp and/or acqrep";
  return thisEncounter;
}

QTime timeObject(QString appliesDate)
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  int hour = appliesDate.mid(11,2).toInt();
  int minute = appliesDate.mid(14,2).toInt();
  int second = appliesDate.mid(17,2).toInt();
  QTime thisEncounter = QTime(hour, minute, second);
  qInfo() << "time=" << thisEncounter << " is being considered in setting tp and/or acqrep";
  return thisEncounter;
}

QDateTime dateTimeObject(QString appliesDate)
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QDate date = QDate(dateObject(appliesDate));
  QTime time = QTime(timeObject(appliesDate));
  QDateTime thisEncounter = QDateTime(date, time);
  qInfo() << "dateTime=" << thisEncounter << " is being considered in setting tp and/or acqrep";
  return thisEncounter;
}

void workItem::useSeriesInCurrentWorkItemList()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  
  // first, let's get the fields we need to see if it is associated with a patient which has already been added
  QString seriesFolder = getImageDatabase()->GetSelectedSeriesPath(ui->dicomImagesTreeWidget);
  QString individualID = getImageDatabase()->GetDICOMMetadata(0x0010,0x0020).c_str();
  QString studyUID = getImageDatabase()->GetDICOMMetadata(0x0020,0x000d).c_str();
  QString seriesUID = getImageDatabase()->GetDICOMMetadata(0x0020,0x000e).c_str();
  QString modality = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0008,0x0060));
  QString accession = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0008,0x0050));

  bool wiEmpty = false;
  
  // check if there is already a work item for this patient/study
  workItemListEntry *wi;
  int thisWI, j, k;
  for (thisWI=0; thisWI < workItemList()->size(); thisWI++) {
    wiEmpty = false;
    wi = &(*(workItemList()))[thisWI];
    QString wiAccession = wi->getAccession();
    if (0 == wi->images_cg.size() && individualID == wi->getIndividualID() && !wiAccession.isEmpty() && (wiAccession == accession)) {
        break;
    }

    if (individualID == wi->getIndividualID() && (wiAccession.isEmpty() || accession.isEmpty() || (wiAccession == accession))) {
      // loop over the series set to determine which one this applies to
      for (j=0; j < wi->images_cg.size(); j++) {
        imageSeries *existingSeries = &((wi->images_cg)[j]);
        if (studyUID == existingSeries->dicomUID) {
          wiEmpty = (0 == j) && (1 == wi->images_cg.size()) && existingSeries->seriesFolder.isEmpty();
          break;
        }
      }
      if (j < wi->images_cg.size()) {
        //workItemSelected(i);

        // now check for degenerate case of this series already being in there
        for (k=0; k < wi->images_cg.size(); k++) {
          imageSeries *existingSeries = &((wi->images_cg)[k]);
          if (seriesUID == existingSeries->seriesUID) {
            message->showMessage(tr("Series had already been added to list."));
            workItemSelected(thisWI);
            return;
          }
        }

        // we get here if there is already a work item for this series to go into and it has not yet been used in it
        break; // found a previously existing work item to add this series into
      }
    }
  }

  // we get here if it hasn't already been added.  Determine the series type; if modality is blank, default to CT with a message, else, if a direct match 
  // between modality and one of the allowable types (as will often occur wwith CT) then take that; else, prompt for it directly from the user.
  QString seriesType = "";
  if (modality == "") {
    seriesType = "CT";
    message->showMessage(tr("Series type is defaulted to CT, please verify and change as necessary."));
  }
  else {
    foreach (QString allowedSeriesType, allowableSeriesTypes) {
      if (modality == allowedSeriesType) {
        seriesType = modality;
      }
    }
  }
  if (seriesType == "") {
    QMenu menu(this);
    foreach (QString allowedSeriesType, allowableSeriesTypes) {
      QVariant seriesTypeParam;
      QString actionString;
      seriesTypeParam.setValue(allowedSeriesType);
      actionString = allowedSeriesType;
      QAction *typeAction = new QAction(actionString, this);
      typeAction->setData(seriesTypeParam);
      menu.addAction(typeAction);
    }
    QAction *act = menu.exec(QCursor::pos());
    if (act != 0) {
      seriesType = act->data().toString();
    }
    else
      return; // user decided not to continue
  }
  
  // now that have a valid type, proceed to perform the action.  
  // if didn't add it to an existing work item, create a new one (recall that the idex variable i retains its value form tyhe search above)
  if (thisWI == workItemList()->size()) {
    addWorkItem();
    wi = &(*(workItemList()))[0]; // the add was by inserting at 0, so point wi there
    wi->setAppliesDate(""); // initial value, will be set later

    // fields that are initialized from DICOM
    wi->setIndividualID(individualID);   
    wi->setPatientName(QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0010,0x0010)));
    wi->setDOB(QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0010,0x0030))); // should re-format
    wi->setSex(QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0010,0x0040)));
    wi->setAge(QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0010,0x1010)));

    // other fields to initialize that may be set by the user later if they so desire 
    wi->setWorkItemPriority("");
    wi->setSubjectType("");
    wi->setAlsoKnownAs("");
    wi->setSubjectID("");
    wi->setIndication("");
    wi->setModelOrganism("");
    thisWI = 0; // for later call to workItemSelected(i)
  }

  // add this series to the image series set of the found or newly created work item
  imageSeries *newSeries = new imageSeries(); // set up the new series
  newSeries->seriesFolder = seriesFolder;
  newSeries->seriesThumbFile = "";
  newSeries->seriesType = seriesType;
  newSeries->seriesAttributes = ""; // leave blank - only here for legacy, or may be used in future for experimentation
  
  // set values for the fields derived from DICOM tags
  newSeries->seriesUID = seriesUID;
  newSeries->dicomUID = studyUID;
  newSeries->anatomy = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0018,0x0015));
  newSeries->make = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0008,0x0070));
  newSeries->model = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0008,0x1090));
  newSeries->sliceThickness = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0018,0x0050));
  newSeries->acquisitionContrast = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0008,0x9209));
  newSeries->contrastAgent = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0018,0x0010));
  newSeries->modality = modality;
  newSeries->magneticFieldStrength = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0018,0x0087));
  newSeries->convolutionKernel = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0018,0x9315));
  newSeries->kvp = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0018,0x0060));
  newSeries->mas = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0018,0x1152));
  
  // get date/time, searchign from most general to most specific until find one with appropriate lengths for parsing
  newSeries->acquisitionDate = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0008,0x0020));
  if (newSeries->acquisitionDate.length() != 8)
    newSeries->acquisitionDate = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0008,0x0021));
  if (newSeries->acquisitionDate.length() != 8)
    newSeries->acquisitionDate = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0008,0x0022));
  if (newSeries->acquisitionDate.length() != 8)
    newSeries->acquisitionDate = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0008,0x0023));

  newSeries->acquisitionTime = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0008,0x0030));
  if (newSeries->acquisitionTime.length() < 6)
    newSeries->acquisitionTime = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0008,0x0031));
  if (newSeries->acquisitionTime.length() < 6)
    newSeries->acquisitionTime = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0008,0x0032));
  if (newSeries->acquisitionTime.length() < 6)
    newSeries->acquisitionTime = QString::fromStdString(getImageDatabase()->GetDICOMMetadata(0x0008,0x0033));

  // derive the applies date 
  QString appliesDateString = "";
  if ((newSeries->acquisitionDate.length() != 8) || (newSeries->acquisitionTime.length() < 6))
      message->showMessage(tr("Warning: Image date/time is not valid to set workitem applies date/time, please set manually or add a series with proper date/time."));
  else {
    appliesDateString = newSeries->acquisitionDate.left(4); // year
    appliesDateString.append("-");
    appliesDateString.append(newSeries->acquisitionDate.mid(4,2)); // month
    appliesDateString.append("-");
    appliesDateString.append(newSeries->acquisitionDate.right(2)); // day
    appliesDateString.append("T");
    appliesDateString.append(newSeries->acquisitionTime.left(2)); // hour 
    appliesDateString.append(":");
    appliesDateString.append(newSeries->acquisitionTime.mid(2,2)); // minute 
    appliesDateString.append(":");
    appliesDateString.append(newSeries->acquisitionTime.mid(4,2)); // sec  
  }

  // if not previously set, use the series acquisition date/time to set the wi applies date/time
  if (wi->getAppliesDate() == "") 
    wi->setAppliesDate(appliesDateString);

  // see if we have enough information to set the tp and acqrep for this work iiem (potentially resetting them for previously added work items as needed)
  // ...timepoint is determined by intervals greater than (owner->systemPreferencesObject->getEncounterWindow().  Note, would need to loop over to reset them in previously added work items based on what is newly added.
  // ...acqrep is determined by intervals within encounterWindow
  // first see if all applies dates are well-formed for this indiviudal.  If so, can set tp and acqreps automatically (can't otherwise)
  bool allWellFormed = true;
  for (int j=0; j < workItemList()->size(); j++) {
    workItemListEntry *wiForConsideration = &(*(workItemList()))[j];
    if (wiForConsideration->getIndividualID() == wi->getIndividualID()) {
      if (!wellFormed(wi->getAppliesDate())) {
        allWellFormed = false;
        break;
      }
    }
  }
  if (allWellFormed) {
    // given that we have well formed applies dates, proceed in steps to determine tp and acqreps:
    // 1. determine oldest encounter;
    // 2. then loop over to set timepoints for all, keeping track of largest timepoint assigned
    // 3. then for each of the assigned timepoints, loop over to determine the acqrep's within that timepoint
    QDate oldestEncounter = QDate(3000, 12, 31); // last day of year 3000
    for (int j=0; j < workItemList()->size(); j++) {
      workItemListEntry *wiForConsideration = &(*(workItemList()))[j];
      if (wiForConsideration->getIndividualID() == wi->getIndividualID()) {
        QDate thisEncounter = dateObject(wiForConsideration->getAppliesDate());
        if (thisEncounter < oldestEncounter) {
          oldestEncounter = thisEncounter;
        }
      }
    }
    int largestTP = 0;
    for (int j=0; j < workItemList()->size(); j++) {
      workItemListEntry *wiForConsideration = &(*(workItemList()))[j];
      if (wiForConsideration->getIndividualID() == wi->getIndividualID()) {
        QDate thisEncounter = dateObject(wiForConsideration->getAppliesDate());
        int tp = (oldestEncounter.daysTo(thisEncounter))/owner->systemPreferencesObject->getEncounterWindow();
        wiForConsideration->setTimepoint(tp);
        if (tp > largestTP) {
          largestTP = tp;
        }
      }
    }
    for (int tp=0; tp < largestTP; tp++) {
      QDate oldestEncounterInThisTP = QDate(3000, 12, 31); // last day of year 3000
      QList<QDateTime> acqreps; 
      for (int j=0; j < workItemList()->size(); j++) {
        workItemListEntry *wiForConsideration = &(*(workItemList()))[j];
        if ((wiForConsideration->getIndividualID() == wi->getIndividualID()) && (wiForConsideration->getTimepoint() == tp)) {
          acqreps.append(dateTimeObject(wiForConsideration->getAppliesDate()));
        }
      }
      qSort(acqreps.begin(), acqreps.end());
      for (int j=0; j < workItemList()->size(); j++) {
        workItemListEntry *wiForConsideration = &(*(workItemList()))[j];
        if ((wiForConsideration->getIndividualID() == wi->getIndividualID()) && (wiForConsideration->getTimepoint() == tp)) {
          wiForConsideration->setAcqrep(acqreps.indexOf(dateTimeObject(wiForConsideration->getAppliesDate())));
        }
      }
    }
  }

  // log updates
  wi->logWorkItemListEntryUpdate("seriesSurvey::addSeries", newSeries->seriesUID, NULL, this);
  wi->logWorkItemListEntryUpdate("seriesSurvey:specifySeries", newSeries->seriesUID, NULL, this);
  QList<imageSeries> *imageSeriesSet = wi->getImageSeriesSet();
  if (wiEmpty)
    imageSeriesSet->removeLast();
  imageSeriesSet->insert(0, *newSeries);
  workItemSelected(thisWI);
}

void workItem::addWorkItem()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  workItemListEntry *newEntry = new workItemListEntry(owner);
  QString newID("wi-");
  QUuid uid = QUuid::createUuid();
  newID.append((uid.toString().mid(1,36).toUpper()).left(8));
  newEntry->setWorkItemID(newID);
  workItemList()->insert(0, *newEntry);
  selectedWorkItem = 0;
  selectedWorkItemValid = true;

  if (currentlyOpenWorkItemListItem != NULL) {
    QVariant numEntriesVariant;
    numEntriesVariant.setValue(QString::number(workItemList()->size()));
    currentlyOpenWorkItemListItem->setData(NUM_ENTRIES_ROLE, numEntriesVariant); 
  }
  emit addWorkItemToModel();
  ENABLECONTROL(ui->saveListChangesButton, saveListChangesAction, tr("Press to save any changes (or to save to another list name)"));
}

void workItem::on_addWorkItemButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  blankPreviewer();
  addWorkItem();
  workItemSelected(0);
  workItemListEntry *wi = &(*(workItemList()))[0]; // the add was by inserting at 0, so point wi there
  QString patientName(tr("doe, john"));
  wi->setPatientName(patientName);
  wi->setTimepoint(0);
  wi->setAcqrep(0);
}

void workItem::on_proceedButton_clicked() 
{ 
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; 
  // should only be one row selected, but the general model thinks there could be more, so need to get all selected indexes and then just take the
  // row of the first one
  QModelIndexList indexes = ui->workItemList->selectionModel()->selection().indexes(); 
  if (selectedWorkItemValid) 
    workItemSelectedForAnalysis(indexes.at(0).row());
}
  
void workItem::on_deleteWorkItemButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  
  // get current selection
  if ( (selectedWorkItemValid == false) || (selectedWorkItem >= workItemList()->size()) ) {
    message->showMessage(tr("Warning: No work item selected."));
  }
  else if (workItemList()->at(selectedWorkItem).getSurvey() != NULL) {
    message->showMessage(tr("Warning: The work item is presently open for analysis.  Must close it first and then re-request deletion."));
    return;
  }
  else {
    // shut off the view update
    disconnect(ui->workItemList->verticalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(workItemSelectedForAnalysis(int)));
    disconnect(ui->workItemList->verticalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(workItemRowSelected(int)));
    delete source;
    delete proxy;

    // remove the item
    workItemList()->removeAt(selectedWorkItem);

    // now reset the model view and log an update to the list
    setupModel();
    logListProvenanceUpdate();

    // and update the context menu info
    if (currentlyOpenWorkItemListItem != NULL) {
      QVariant performerVariant;
      performerVariant.setValue(getListProvenance()->getPerformer());
      currentlyOpenWorkItemListItem->setData(PERFORMER_ROLE, performerVariant); 

      QVariant performerDateTimeVariant;
      performerDateTimeVariant.setValue(getListProvenance()->getPerformDateTime());
      currentlyOpenWorkItemListItem->setData(PERFORM_DATETIME_ROLE, performerDateTimeVariant); 

      QVariant lastModifierVariant;
      lastModifierVariant.setValue(getListProvenance()->getLastModifier());
      currentlyOpenWorkItemListItem->setData(LAST_MODIFIER_ROLE, lastModifierVariant); 

      QVariant lastModifiedDateTimeVariant;
      lastModifiedDateTimeVariant.setValue(getListProvenance()->getLastModifiedDateTime());
      currentlyOpenWorkItemListItem->setData(LAST_MODIFIED_DATETIME_ROLE, lastModifiedDateTimeVariant);       

      QVariant numEntriesVariant;
      numEntriesVariant.setValue(QString::number(workItemList()->size()));
      currentlyOpenWorkItemListItem->setData(NUM_ENTRIES_ROLE, numEntriesVariant); 
    }

    selectedWorkItemValid = false;
    DISABLECONTROL(ui->proceedButton, proceedAction, tr("Select a work item to process first")); // should be disabled now that there is no longer a selected workitem
    ui->deleteWorkItemButton->setEnabled(false);
    ui->seriesSetBox->hide();
  }
}

void workItem::on_addSourceButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  ui->sources->clearSelection();
  bool ok;
  QString sourceName;
  QString prompt = tr("Desired source name:");
  while (1) {
    sourceName=QInputDialog::getText(this, tr("Source"), prompt, QLineEdit::Normal, "", &ok);
    if (ok && !sourceName.isEmpty()) {
      // make sure that the name isn't already used
      int i;
      for (i=0; i < ui->sources->count(); i++) {
        if (sourceName == ui->sources->item(i)->text()) {
          prompt = tr("Cannot use the same name as another source already listed. New name:");
          break;
        }
      }
      if (i < ui->sources->count())
        continue; // there was a dupe
      else
        break;
    }
    else
      return; // user cancelled
  }

  // now that we have a name, get its type
  QMessageBox msgBox(this);
  QString str = tr("Adding ")+sourceName+".";
  msgBox.setText(str);
  msgBox.setInformativeText(tr("What type of source is this?"));
  QAbstractButton *fileSystemButton = msgBox.addButton(tr("File System"), QMessageBox::YesRole);
  QAbstractButton *pacsButton = msgBox.addButton(tr("Picture Archiving and Communication System"), QMessageBox::NoRole);
  msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
  int ret = msgBox.exec();
  importSources.push_back(std::make_shared<ImportSource>());
  if (msgBox.clickedButton() == fileSystemButton) {
    importSources.back()->item = new QListWidgetItem(sourceName, ui->sources, TYPE_FILESYSTEM_SOURCE);
    importSources.back()->config = new sourceConfiguration(sourceName, ui->sources, TYPE_FILESYSTEM_SOURCE);
  }
  else {
    importSources.back()->item = new QListWidgetItem(sourceName, ui->sources, TYPE_PACS_SOURCE);
    importSources.back()->config = new sourceConfiguration(sourceName, ui->sources, TYPE_PACS_SOURCE);
    importSources.back()->queryRetrieve = new dicomQueryRetrieve(this);
  }  

  // now configure it
  importSources.back()->config->displayConfiguration();
  ui->sources->clearSelection();
}

void workItem::updateWorkItemDisplaysGivenSave()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  
  disconnect(ui->workItemLists, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showWorkItemListContextMenu(QPoint)));
  populateWorkItemLists(getListProvenance()->getListFileName());
  ENABLECONTROL(ui->saveListChangesButton, saveListChangesAction, tr("Press to save any changes (or to save to another list name)"));

  // reset the provenance data into the work list item
  QVariant performerVariant;
  performerVariant.setValue(getListProvenance()->getPerformer());
  currentlyOpenWorkItemListItem->setData(PERFORMER_ROLE, performerVariant); 

  QVariant performerDateTimeVariant;
  performerDateTimeVariant.setValue(getListProvenance()->getPerformDateTime());
  currentlyOpenWorkItemListItem->setData(PERFORM_DATETIME_ROLE, performerDateTimeVariant); 

  QVariant lastModifierVariant;
  lastModifierVariant.setValue(getListProvenance()->getLastModifier());
  currentlyOpenWorkItemListItem->setData(LAST_MODIFIER_ROLE, lastModifierVariant); 

  QVariant lastModifiedDateTimeVariant;
  lastModifiedDateTimeVariant.setValue(getListProvenance()->getLastModifiedDateTime());
  currentlyOpenWorkItemListItem->setData(LAST_MODIFIED_DATETIME_ROLE, lastModifiedDateTimeVariant); 
  
  QVariant numEntriesVariant;
  numEntriesVariant.setValue(QString::number(workItemList()->size()));
  currentlyOpenWorkItemListItem->setData(NUM_ENTRIES_ROLE, numEntriesVariant); 

  updateWorkItemListNameText();
}

void workItem::packageDataAndTransitionToReport()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // determine whether there are unsaved changes and seek user confirmation if so
  if (getUnsavedChangesFlag()) {
    if (owner->systemPreferencesObject->getPromptBeforeSave() || (currentlyOpenWorkItemListItem == NULL)) {
      // either the user always wants a prompt or they have to be prompted anyway since the current list hasn't yet been named
      QMessageBox msgBox(this);
      msgBox.setText(tr("There are unsaved changes."));
      msgBox.setInformativeText(tr("Do you want to save these changes and proceed to report generation? Yes means save and proceed, No means do not save or proceed."));
      msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      msgBox.setDefaultButton(QMessageBox::No);
      msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
      int ret = msgBox.exec();
      if (ret == QMessageBox::No) {
        return;
      }
      owner->save();
      setUnsavedChangesFlag(false); 
    }
    else if (currentlyOpenWorkItemListItem != NULL) { // the case when user doesn't want to be prompted but there is an open list
      owner->save();
    }
  }

  // first start up a progress indicator
  generateProgressIndicator = new QProgressDialog(tr("Generating and transferring data for report..."), "Cancel", 0, 0, owner);
  generateProgressIndicator->setWindowModality(Qt::NonModal);
  generateProgressIndicator->setMinimumDuration(0);
  generateProgressIndicator->setWindowFlags(generateProgressIndicator->windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::WindowStaysOnTopHint);
  generateProgressIndicator->setCancelButton(nullptr);   // no cancel button on dialog
  generateProgressIndicator->show();
  this->repaint(); // ensure progress is shown
  qApp->processEvents();

  // now check authorization
  QString CAPgraph_host = owner->getCAPgraphHost();
  QString urlStr = CAPgraph_host + "/check_authorization/";
  QString localUser = getLocalUserName();
  urlStr.append(QString::fromStdString(ebSystem::GetSystemDiskUUID())).append("/").append(localUser).append("/");
  urlStr.append("?token=" + owner->token);
  QUrl transferUrl(urlStr);
  QNetworkRequest request(transferUrl);
  GetAuthorizationTokenITI_71reply = GetAuthorizationTokenITI_71.get(request);
}

void workItem::parseReportAuthorizationTokenITI_71reply(QNetworkReply *reply)  
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  //networkReply->deleteLater();
  if (reply->error() == QNetworkReply::NoError) {
    QByteArray byteArray = reply->readAll();
    if (byteArray.size() > 0) {
      QJsonParseError err;
      QJsonDocument doc = QJsonDocument::fromJson(byteArray, &err);
      if (doc.isObject()) {
        QJsonObject obj = doc.object();
        QJsonObject::iterator itr = obj.find("token");
        if (itr != obj.end()) {
          owner->token = obj["token"].toString();
        }
        itr = obj.find("identity");
        if (itr != obj.end()) {
          QString identity = obj["identity"].toString(); 
          QString name = owner->getUserName();
          if (identity == name) {
            // here we either need to ship data to server, which will then subsequently cause the actual
            // report geenration request to be sent, or, if the data has already been sent, just skip to
            // send the report generation transaction directly
            workItemListEntry *wi = &(*(workItemList()))[selectedWorkItem];
            if (!wi->dataSentToServer) {
              // save out the workItem for which the report is being generated to a separate file
              if (!wi->setWorkItemFolder()) { // this may be the first time an operation that requires the folder has been done
                message->showMessage(tr("parseReportAuthorizationTokenITI_71reply: Cannot write work item folder."));
                return;
              }
              // here we do not need to make a temporary directory as at this point we constrain the user
              // to save now that they are actually asking for a report.  in the other places where we use
              // this same function, we must prepare a temporary folder when changes have been made but
              // not yet saved, to work with the most current data but this level of processing is
              // obviated here because we know there are no changes otherwise we would have forced them
              // to save them because this is the "end of the line" and data consistency is more important
              // than user convenience at this final transfer.
              // on server side, all transfers are checked relative to prior ones for any updated data
              // fields which could trigger re-generation of any report 
              // elements thta depend on them, but here in the client all we have to do is to ship up the
              // data and leave it to the server to incrementally build or re-build report elements
              // as needed.
              if (shipDataToServer(wi->getWorkItemFolder(), wi, true))
                return;
            } else {
              // the case where the data was already sent so no need to send it again
              QUrl reportUrl(wi->urlStr);
              emit giveScreenControlToReport(workItemList()->at(selectedWorkItem).getWorkItemID(), reportUrl, generateProgressIndicator);
              return;
            }
          }
          else 
            QMessageBox::warning(this, tr("Error: Authorization returned a non-matching identity"), QString("parseReportAuthorizationTokenITI_71reply: Authorization returned a non-matching identity: %1").arg(identity));
        } 
        else 
          message->showMessage(tr("parseReportAuthorizationTokenITI_71reply: Network transfer failed, response did not contain an identity, skipping"));
      } // if response cannot be parsed as a json doc
      else
        QMessageBox::warning(this, tr("Error parsing server return"), QString("parseReportAuthorizationTokenITI_71reply error parsing server return: %1").arg(err.errorString()));
    } // skip empty replies
    else
      qInfo() << "Reply with zero bytes received, skipping";
  } // if network error
  else {
    QMessageBox msgBox(this);
    msgBox.setText(tr("You do not have an active CAPgraph session, which is necessary in order to generate a report."));
    msgBox.setInformativeText(tr("Do you want to start one now (which would allow you to re-request the report)?"));
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    int ret = msgBox.exec();
    if (ret != QMessageBox::No) {
      owner->userLoginStatusButton_clicked();
    }
  }
  
  // only get here if one of the error conditions ocurred, take down the progrss indicator
  generateProgressIndicator->setMaximum(100); // any value will take it down
  delete generateProgressIndicator;
}

void workItem::processWebError(QNetworkReply *errorCode)
{
    qInfo() << "Network error: " << errorCode;
    qInfo() << "Error details: " << errorCode->errorString();
    qInfo() << QString::fromStdString(errorCode->readAll().toStdString());
}

void workItem::parseUPSWorkitemContentsRADreply(QNetworkReply *reply)  
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  workitem_callback_data *cbdata = ((workitem_callback_data *)reply->request().originatingObject());
  cbdata->getReply()->deleteLater();
  workItemListEntry *wi = cbdata->entry();
  char *debug = getenv("WEB_DEBUG");

  wi->uploadComplete();

  if (reply->error() != QNetworkReply::NoError) {
    eblog << "Download error. parseUPSWorkitemContentsRADreply errorString=" << reply->errorString().toStdString() << std::endl;
    eblog << reply->readAll().toStdString() << std::endl;
    if (wi->cloned) {
        wi->cloned = false;
        delete wi;
    }
    delete cbdata;
    return;
  }

    qInfo() << "webReply uploadState: " << cbdata->getUploadStateStr();

    if (WI_STATE_UPLOAD == cbdata->getUploadState()) {
        // now set to the report

        QByteArray byteArray = reply->readAll();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(byteArray, &err);

        if (!doc.isObject()) {
          QMessageBox::warning(this, tr("Error parsing server return"), QString("parseUPSWorkitemContentsRADreply errorString: %1").arg(err.errorString()));
          return;
        }
        QJsonObject obj = doc.object();
        QJsonObject::iterator itr = obj.find("id");
        if (itr == obj.end()) {
            message->showMessage(tr("parseUPSWorkitemContentsRADreply network transfer failed (response has no id), skipping"));
            return;
        }

        wi->urlStr = owner->getCAPgraphHost() + "/workitem/" + obj["id"].toString() + "?user=" + owner->getUserName() + "&token=" + owner->token ;

        QString upload = owner->getCAPgraphHost() + "/workitem_file/" + obj["id"].toString() + "?user=" + owner->getUserName() + "&token=" + owner->token ;
        cbdata->setUploadStr( upload );

        itr = obj.find("filenames");
        if (itr != obj.end()) {
            QVariantMap filemap = obj.toVariantMap()["filenames"].toMap();
            for ( const auto& key : filemap.keys() ) {
                QVariantMap details = filemap[key].toMap();
                int fsize = details["size"].toInt();
                double fdate = details["date"].toDouble();
                qInfo() << "webReply: file key: " << key << " value: " << fsize << " " << QString::number(fdate, 'f');
                wi->reconcile(key, fsize, fdate);
            }
        }
        
        itr = obj.find("series");
        if (itr != obj.end()) {
            QVariantMap seriesmap = obj.toVariantMap()["series"].toMap();
            for ( const auto& sekey : seriesmap.keys() ) {
                QList<QVariant> sops = seriesmap[sekey].toList();
                for (auto sop : sops) {
                    QString sopuid = sop.toString();
                    qInfo() << "webReply: sop key: " << sopuid ;
                    wi->reconcile(sopuid, 0, -1);
                }
            }
        }
        wi->getManifest()->updateFileMetadata();
    }

    while (!cbdata->getUploads()->isEmpty()) {
        fileinfo *fi = cbdata->getUploads()->dequeue();
        if (0 == fi->offset)
            fi->uploaded = true;
    }

    if (WI_STATE_POST == cbdata->getUploadState()) {
        wi->dataSentToServer = true;
        cbdata->setUploadState(WI_STATE_FINAL);
        qInfo() << "webReply setUploadState: " << cbdata->getUploadStateStr();
        wi->checkPendingUpload(this);
    } else if (WI_STATE_META == cbdata->getUploadState()) {
        cbdata->setUploadState(WI_STATE_POST);
        qInfo() << "webReply setUploadState: " << cbdata->getUploadStateStr();
        // upload complete, post an empty packet to trigger server processing
        wi->makeUpload(this, cbdata);
    } else if (wi->isUploadComplete()) {
        cbdata->setUploadState(WI_STATE_META);
        qInfo() << "webReply setUploadState: " << cbdata->getUploadStateStr();
        wi->getManifest()->prepareMetadata();
        // upload and update metadata
        wi->makeUpload(this, cbdata);
    } else {
        cbdata->setUploadState(WI_STATE_FILES);
        qInfo() << "webReply setUploadState: " << cbdata->getUploadStateStr();
        // start uploading files
        while (wi->continueUpload(cbdata)) {
            qInfo() << "webReply create upload packet";
            wi->makeUpload(this, cbdata);
            if (NULL != debug)
                break;
        }
    }

    if (WI_STATE_FINAL == cbdata->getUploadState() && cbdata->isReport()) {
        QUrl reportUrl(wi->urlStr);
        emit giveScreenControlToReport(workItemList()->at(selectedWorkItem).getWorkItemID(), reportUrl, generateProgressIndicator);
        if (wi->cloned)
            delete wi;
    }
    delete cbdata;
}

void workItem::on_sources_itemClicked(QListWidgetItem *item)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  std::shared_ptr<ImportSource> importSource;
  for (auto &s : importSources)
    if (s->item == item)
      importSource = s;
  ebAssert(importSource);
  if (importSource->config->type == TYPE_FILESYSTEM_SOURCE) {
    QString startingFolder = cap::getDocumentsDir();
    QString importFolderAbsolute = QFileDialog::getExistingDirectory(this, tr("Import Folder"), importSource->config->startingFolder, QFileDialog::ShowDirsOnly);
    QDir    importFolderDir = QDir::current();
    QString importFolder = importFolderDir.relativeFilePath(importFolderAbsolute);
    // process the import (or bypass without further action if user hit cancel)
    if (importFolder.isNull()) {
      return;    
    }
    bool recursive = owner->systemPreferencesObject->getImportFoldersRecursively();
    bool original = owner->systemPreferencesObject->getRequireOriginal();
    bool localizer = owner->systemPreferencesObject->getExcludeLocalizer();
    bool precontrast = owner->systemPreferencesObject->getExcludePreContrast();
    bool showProgress = true;
    importImages(importFolderAbsolute,recursive,original,localizer,precontrast,showProgress);
  }    
  else {
    ebAssert(importSource->queryRetrieve);
    importSource->queryRetrieve->init(importSource->config->serverAETitle.toStdString(), importSource->config->serverHost.toStdString(), importSource->config->serverPort,
                                      importSource->config->localAETitle.toStdString(), importSource->config->localPort, importSource->config->secure);
    importSource->queryRetrieve->showAndRaise();
  }
}

void workItem::showSourceContextMenu(const QPoint &point)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QListWidgetItem *temp = ui->sources->itemAt(point);
  int index = -1;
  for (int i = 0; i < importSources.size(); ++i)
    if (importSources[i]->item == temp) {
      index = i;
      break;
    }
  if (index != -1) {
    QMenu menu(this);
    QAction *configureAct = new QAction(tr("Configure this source"), this);
    configureAct->setData(index);
    menu.addAction(configureAct);
    connect(configureAct, SIGNAL(triggered()), this, SLOT(configureSource()));
    if ((temp->text() != DEFAULT_FILESYSTEM_SOURCE) && (temp->text() != DEFAULT_PACS_SOURCE)) {
      QAction *removeAct = new QAction(tr("Remove this source from the list"), this);
      removeAct->setData(index);
      menu.addAction(removeAct);
      connect(removeAct, SIGNAL(triggered()), this, SLOT(removeSource()));
    }
    menu.exec(QCursor::pos());
  }
}

void workItem::configureSource()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QAction *act = qobject_cast<QAction *>(sender());
  if (act) {
    int index = act->data().toInt();
    if ((index >= 0) && (index < importSources.size())) {
      importSources.at(index)->config->displayConfiguration();
      ui->sources->clearSelection();
    }
  }
}

void workItem::removeSource()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QAction *act = qobject_cast<QAction *>(sender());
  if (act) {
    int index = act->data().toInt();
    if ((index >= 0) && (index < importSources.size())) {
      QFile::remove(((importSources.at(index)->config->type == TYPE_FILESYSTEM_SOURCE) ? cap::getConfigurationFileSystemSourcesDir() : cap::getConfigurationPACSSourcesDir()) 
                    + "/" + importSources.at(index)->config->name + ".json");
      int row = ui->sources->row(importSources.at(index)->item);
      ui->sources->takeItem(row);
      ui->sources->clearSelection();
      importSources.erase(importSources.begin()+index);
    }
  }
}

void workItem::setSelectedItem(QTreeWidgetItem *item)
{
    for (int j=0; j < workItemList()->size(); j++) {
      workItemListEntry *wi = &(*(workItemList()))[j];
      for (int k=0; k < wi->images_cg.size(); k++) {
        imageSeries *series = &((wi->images_cg)[k]);
        QDir seriesFolderDir(series->seriesFolder);
        QString seriesFolderAbsolute;
        if (seriesFolderDir.exists()) {
          QFileInfo imageFileInfo = seriesFolderDir.entryInfoList().last();
          seriesFolderAbsolute = imageFileInfo.absolutePath();
        }
        else {
          seriesFolderAbsolute = ""; // can't match what doesn't exist
        }
        if (seriesFolderAbsolute == item->text(DICOM_IMAGES_COLUMN_PATH)) {
          selectedWorkItem = j; 
          selectedWorkItemValid = true;  
          ui->workItemList->selectRow(j);
          ENABLECONTROL(ui->proceedButton, proceedAction, tr("Press to process selected work item")); // enable now that one is selected
          ui->deleteWorkItemButton->setEnabled(true);
          ui->seriesSetBox->show();
          return;
        }
      }
    }
}

void workItem::on_dicomImagesTreeWidget_itemPressed(QTreeWidgetItem *item)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (item->text(DICOM_IMAGES_COLUMN_TYPE) != "SERIES") {
    blankPreviewer();
    item->setSelected(false); // can only select series, not other levels
    return;
  }

  // now we check if this is the same as was already been previously selected
  QString priorSeriesPath = getImageDatabase()->GetPriorSeriesPath();
  QString thisSeriesPath = getImageDatabase()->ResetSelectedSeriesPath(ui->dicomImagesTreeWidget);
  if ((priorSeriesPath == thisSeriesPath) && (imageID != null_ebID)) {
    setSelectedItem(item);
    return; // don't do anything more if they clicked on the same one again
  } else {
    blankPreviewer();
  }
  item->setSelected(true); // make sure this one is selected, as a variety of conditions could have resulted in in it not being
  getImageDatabase()->SetSelectedSeriesPath(ui->dicomImagesTreeWidget->selectedItems().first()->text(DICOM_IMAGES_COLUMN_PATH));
  
  // synchronize the image selector/previewer with the work item table selection.  We do this in steps: 
  // 1. see if both selections are compatible, and if so, return without changing selections. 
  // 2. if there is a work item that has this series, select it. 

  // step 1: see if the selections are consistent:
  if (selectedWorkItemValid) {
    workItemListEntry *wi = &(*(workItemList()))[selectedWorkItem];
    bool selectionsAreConsistent = false;
    for (int k=0; k < wi->images_cg.size(); k++) {
      imageSeries *series = &((wi->images_cg)[k]);
      if (series->seriesFolder == getImageDatabase()->GetSelectedSeriesPath(ui->dicomImagesTreeWidget))
        selectionsAreConsistent = true;
    }
    if (!selectionsAreConsistent) {
      ui->workItemList->clearSelection(); // this selection invalidates any prior workitem selection
    }
  }

  setSelectedItem(item);

  // proceed with the processing for the newly selected series
  ui->dicomImagesTreeWidget->setDragEnabled(false); // can't drag until the series has had its headers read in so that DICOM meta data is available
  ui->currentWorkItemListGroupBox->setEnabled(false);
  getImageDatabase()->PopulateDICOMMetadataTreeWidget(ui->dicomMetadataTreeWidget,item);
  ui->dicomImagesTreeWidget->setDragEnabled(true); // drag/drop now OK since metadata has been loaded
  ui->currentWorkItemListGroupBox->setEnabled(true);
  
  if (item->text(DICOM_IMAGES_COLUMN_TYPE) == "SERIES") {
    std::string seriesPath = getImageDatabase()->ResetSelectedSeriesPath(ui->dicomImagesTreeWidget).toStdString();
    viewers->Clear();    
    std::cerr << "new multiReader" << std::endl;
     multiReader = ebiMultiImageReader::New();
    std::cerr << "new multiReader done" << std::endl;
    multiReader->SetUseThread(true);
    imageID = multiReader->OpenImage<float,3>(seriesPath);
    image4ID = viewers->AddImage4(multiReader,imageID,seriesPath);
    viewers->GetScene()->SetCursorPositionToSceneCenter();
    viewers->SyncViewersToScene();
    viewers->InitializeCameras();
    viewers->GetScene()->GetSelectedImage4()->SetWindowLevel(800, 200);
    viewers->Render();
  }
}

void workItem::on_workItemLists_itemClicked(QListWidgetItem *item)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  bool specialCaseWhenUnnamedListSaveAsFailed = false; // this is a nuisance corner case!

  for (int i=0; i < ui->workItemLists->count(); i++) {
    if (item == ui->workItemLists->item(i)) {
      // check conditions that would result in an early return
      if (item == currentlyOpenWorkItemListItem)
        return; // no need to do anything, as they just re-clicked what was already clicked
      else if (getOpenAnalysisCount() > 0) {
        message->showMessage(tr("Warning: Cannot load a new list file if analyses are still open.  Please close them first."));
        // make sure that the selection remains where it was (otherwise it misleadingly looks like the one right-clicked is now selected, which isn't actually the case)
        currentlyOpenWorkItemListItem->setSelected(true);
        return;
      }

      // now that the preliminary checks are done, proceed
      if (((currentlyOpenWorkItemListItem != NULL) || (workItemList()->size() > 0)) && (item != currentlyOpenWorkItemListItem) && getUnsavedChangesFlag()) {
        int ret = QMessageBox::Yes;
        if (owner->systemPreferencesObject->getPromptBeforeSave() || (currentlyOpenWorkItemListItem == NULL)) {
          QMessageBox msgBox(this);
          msgBox.setText(tr("There are unsaved changes in the previously open list."));
          msgBox.setInformativeText(tr("Would you like to save them before switching lists? Yes means save before switch, No means discard changes and do the switch."));
          msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
          msgBox.setDefaultButton(QMessageBox::No);
          msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
          ret = msgBox.exec();
        }
        if (ret == QMessageBox::Yes) {
          int j;
          for (j=0; j < ui->workItemLists->count(); j++) {
            if (currentlyOpenWorkItemListItem == ui->workItemLists->item(j)) {
              if (!owner->saveWorkItemDataToDisk(ui->workItemLists->item(j)->data(Qt::ToolTipRole).toString(), Json, true)) {
                QMessageBox msgBox(this);
                msgBox.setText(tr("There was a problem saving changes to the currently open list."));
                msgBox.setInformativeText(tr("Do you want to proceed to open the new list anyway? Yes means lose changes in the prior list and open the new, No means abort the switch, for example to save the changes to another name."));
                msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::Yes);
                msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
                int ret = msgBox.exec();
                if (ret == QMessageBox::No)  
                  return; // leave things as they are if user cancels (that is, don’t change the selected work item list in the navigator)
              }
            }
          }
          if (j==ui->workItemLists->count()) {
            // this case changes the index since it adds to the list, so need to reset the index to what was intended
            QString selection = ui->workItemLists->item(i)->data(Qt::ToolTipRole).toString();
            if (owner->saveAs()) {
              //item = ui->workItemLists->findItems(selection, Qt::MatchExactly).first();
              qDebug() << "i had been " << i;
              for (i=0; i < ui->workItemLists->count(); i++) {
                if (ui->workItemLists->item(i)->data(Qt::ToolTipRole).toString() == selection)
                  break;
              }
              qDebug() << "i now is " << i;
              item = ui->workItemLists->item(i);
              ui->workItemLists->setCurrentRow(i);
            }
            else {
              // deselect everything
              for (i=0; i < ui->workItemLists->count(); i++) {
                ui->workItemLists->item(i)->setSelected(false);
              }
              specialCaseWhenUnnamedListSaveAsFailed = true; // gotta hate GUI work
            }
            currentlyOpenWorkItemListItem = NULL;
          }
        }
      }
      // done with any prior list, if there was one, now proceed to the new one
      if (!specialCaseWhenUnnamedListSaveAsFailed) {
        if (item != currentlyOpenWorkItemListItem) {
          currentlyOpenWorkItemListItem = item; // save so that can tell if next selection differs
          qDebug() << ui->workItemLists->item(i)->data(Qt::ToolTipRole).toString();
          if (!loadWorkItemListFromFile(ui->workItemLists->item(i)->data(Qt::ToolTipRole).toString(), Json)) {
            QMessageBox::warning(owner, tr("could not open the selected work item list"), QString(tr("could not open the work item list. (if you are not aware of deleting it, please contact Elucid)")));
            return; // leave things as they are if file can't be read
          }
        }
      }
    }
  }
}

void workItem::showWorkItemListContextMenu(const QPoint &point)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  // first, let's see what item was right clicked
  QListWidgetItem *itemRightClicked = ui->workItemLists->itemAt(point);

  // (both here at the start as well as below at the end) make sure that the selection remains where it was (otherwise it misleadingly looks like the one right-clicked is now selected, which isn't actually the case)
  if (currentlyOpenWorkItemListItem != NULL)
    currentlyOpenWorkItemListItem->setSelected(true);
  else
    ui->workItemLists->clearSelection();

  // now do it
  QListWidgetItem *temp = ui->workItemLists->itemAt(point);
  if (temp != NULL) {
    QVariant workItemList;
    workItemList.setValue(temp->data(LIST_NAME_ROLE).toString());
    QMenu menu(this);
    int countNonBlankProvenance = 0;
    if (itemRightClicked == currentlyOpenWorkItemListItem) {
      if ((currentlyOpenWorkItemListItem->data(PERFORMER_ROLE).toString() != "") || (currentlyOpenWorkItemListItem->data(PERFORM_DATETIME_ROLE).toString() != "")) {
        QLabel *origination = new QLabel(tr("Originated: ")+currentlyOpenWorkItemListItem->data(PERFORM_DATETIME_ROLE).toString()
                                             +tr(" by: ")+currentlyOpenWorkItemListItem->data(PERFORMER_ROLE).toString());
        QWidgetAction *originationAction = new QWidgetAction(this);
        originationAction->setDefaultWidget(origination);
        menu.addAction(originationAction);
        countNonBlankProvenance++;
      }
      if ((currentlyOpenWorkItemListItem->data(LAST_MODIFIER_ROLE).toString() != "") || (currentlyOpenWorkItemListItem->data(LAST_MODIFIED_DATETIME_ROLE).toString() != "")) {
        QLabel *modification = new QLabel(tr("Last Modified: ")+currentlyOpenWorkItemListItem->data(LAST_MODIFIED_DATETIME_ROLE).toString()
                                                   +tr(" by: ")+currentlyOpenWorkItemListItem->data(LAST_MODIFIER_ROLE).toString());
        QWidgetAction *modificationAction = new QWidgetAction(this);
        modificationAction->setDefaultWidget(modification);
        menu.addAction(modificationAction);
        countNonBlankProvenance++;
      }
      if (currentlyOpenWorkItemListItem->data(PERFORMER_ROLE).toString() != "") {
        QLabel *numEntries = new QLabel(tr("Currently containing: ")+currentlyOpenWorkItemListItem->data(NUM_ENTRIES_ROLE).toString()+tr(" work items."));
        QWidgetAction *numEntriesAction = new QWidgetAction(this);
        numEntriesAction->setDefaultWidget(numEntries);
        menu.addAction(numEntriesAction);
        countNonBlankProvenance++;
      }
    }
    if (countNonBlankProvenance > 0)
      menu.addSeparator();
    QAction *deleteAct = new QAction(tr("Delete this work item list"), this);
    deleteAct->setData(workItemList);
    menu.addAction(deleteAct);
    connect(deleteAct, SIGNAL(triggered()), this, SLOT(deleteWorkItemList()));
    menu.exec(QCursor::pos());
  }
  /*else
    message->showMessage("not an item");*/
  // (both here at the end as well as above at the start) make sure that the selection remains where it was (otherwise it misleadingly looks like the one right-clicked is now selected, which isn't actually the case)
  if (currentlyOpenWorkItemListItem != NULL)
    currentlyOpenWorkItemListItem->setSelected(true);
  else
    ui->workItemLists->clearSelection();
}

void workItem::deleteWorkItemList()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QAction *act = qobject_cast<QAction *>(sender());
  if (act != 0) {
    QVariant data = act->data();
    for (int i=0; i < ui->workItemLists->count(); i++) {
      if (data == ui->workItemLists->item(i)->data(LIST_NAME_ROLE).toString()) {
        // prompt the user to confirm since this destroys data
        QMessageBox msgBox(this);
        msgBox.setText(tr("Deleting a work item list irreversibly deletes data from disk, including the analysis objects."));
        msgBox.setInformativeText(tr("Do you want to go ahead with the deletion? Yes means delete the list data, No means do not delete."));
        msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
        int ret = msgBox.exec();
        if (ret == QMessageBox::No)
          return;

        // if this is the currently loaded one, make sure that there are no open analyses and if not, re-initialize the list back to empty
        if (ui->workItemLists->item(i) == currentlyOpenWorkItemListItem) {
          if (getOpenAnalysisCount() > 0) {
            message->showMessage(tr("Cannot Cannot delete a work item list with open analysis tabs.  Please close them, then delete if desired."));
            return;
          }
          // reach here if nothing is open to prevent the deletion.  since this is the currently selcted one, want to clear out the various data structures
          owner->initializeWorkItemList();
          disconnect(ui->workItemLists, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showWorkItemListContextMenu(QPoint)));
          populateWorkItemLists(""); 
          currentlyOpenWorkItemListItem = NULL;
          setupModel();
        }

        // delete the folder with the data
        QFileInfo *listFileInfo = new QFileInfo(ui->workItemLists->item(i)->data(Qt::ToolTipRole).toString());
        QDir listFileDir = listFileInfo->absoluteDir();
        if (listFileInfo->baseName().replace(".json", "") == listFileDir.dirName()) 
          listFileDir.removeRecursively();
        else {
          message->showMessage(tr("As a precaution, only work item lists stored according to convention can be deleted in the application, and this one is saved differently so has not been deleted."));
          return;
        }

        // and finally remove the display item
        QListWidgetItem *item = ui->workItemLists->takeItem(i);
        delete item;
      } // end-if this is the work item list
    } // end-for each of our work item lists
  } // end-if action has a sender() 
}

bool workItem::eventFilter(QObject *obj, QEvent *event) 
{ 
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // one type of event we need to capture are those coming from the series set box series type selector
  for (int seriesIndex=0; seriesIndex < seriesTypes.size(); seriesIndex++) {
    if (obj == seriesTypes[seriesIndex]) {
      if (selectedWorkItemValid) {
        if (event->type() == QEvent::Leave) {
          workItemListEntry *wi = &(*workItemList())[selectedWorkItem];
          for (int k=0; k < wi->images_cg.size(); k++) {
            imageSeries *series = &((wi->images_cg)[k]);
            QDir seriesFolderDir(series->seriesFolder);
            if (seriesFolderDir.exists()) {
              QFileInfo imageFileInfo = seriesFolderDir.entryInfoList().last();
              QString seriesFolderAbsolute = imageFileInfo.absolutePath();
              if (seriesFolders[seriesIndex]->text() == seriesFolderAbsolute) {
                if (series->seriesType != seriesTypes[seriesIndex]->currentText()) {
                  if (wi->targets.size() == 0) {
                    series->seriesType = seriesTypes[seriesIndex]->currentText();
                    wi->logWorkItemListEntryUpdate("seriesSurvey:specifySeries", series->seriesUID, NULL, this);
                  }
                  else {
                    message->showMessage(tr("Cannot change series type from this screen since targets have already been defined (but it can be changed at any of the analysis screens)"));
                    seriesTypes[seriesIndex]->setCurrentIndex(seriesTypes[seriesIndex]->findText(series->seriesType)); // to revert the combo box itself
                  }
                }
              }
            }
            else {
              message->showMessage(tr("Error: Series folder can not be opened (contact Elucid if condition persists)"));
            }
          }
        }
      }
    }
  }
  // all events reaching this stage are passed further on
  return false;
}

bool workItem::shipDataToServer(QString dir, workItemListEntry *wi, bool transferToReport)
{
  ebLog eblog(Q_FUNC_INFO); eblog << dir.toStdString() << std::endl;

    // the purpose of this method is to package and send work item data that will be further processed on
    // the server.  It is called from 4 different places in order to facilitate timely
    // server-side processing.  The times this is called are:
    // 1) on initial work item selection;
    // 2) when wall partitioning is completed;
    // 3) when readings and/or lesion readings are calculated;
    // 4) when the report is requested.
    // Server-side, the manifest of files is evaluated as to state, enabling that which is supported by
    // the data at any given point in time.  Subsequent transfers for the same WI  are checked for
    // changed fields or files, with re-building performed accordingly.  

    // given the multiple times this is called and to avoid redundant transfers of large file objects,
    // a file manifest is maintained to identify what has either initally originated from, or has
    // previously been transferred to the server so as to skip it in this transfer

  QDateTime dt = QDateTime::currentDateTime();
  
    manifestQueue *wiFiles = wi->getManifestQueue();

    QString prefix = wi->getIndividualID() + "/" + wi->getWorkItemID();

    QString blank = QString("");
    DcmFileFormat metadata;
    DcmTag stoptag = DcmTag(0x0020, 0x0010);
    DcmTag soptag =  DcmTag(0x0008, 0x0018);
    DcmTag seriestag =  DcmTag(0x0020, 0x000e);
    for (int i = 0; wi->images_cg.size() > i; i++) {
        QString folder = wi->images_cg.at(i).seriesFolder;
        QDir contents(folder);
        QStringList dcmNames = contents.entryList(QDir::Files);
        QString nameStr = "form-data; name=\"";

        QString seuid = wi->images_cg.at(i).seriesUID;

        bool first = seuid.isEmpty() || (0 < seuid.size() && !seuid[0].isDigit());

        foreach (const QString &dcmName, dcmNames) {
            QString dcmFile = contents.absoluteFilePath( dcmName );
            QString sopuid;
            OFString value;
#if OFFIS_DCMTK_VERSION_NUMBER < 362
            metadata.loadFile(dcmFile.toLocal8Bit().data(), EXS_Unknown, EGL_noChange, 100, ERM_autoDetect);
#else
            metadata.loadFileUntilTag(dcmFile.toLocal8Bit().data(), EXS_Unknown, EGL_noChange, 100, ERM_autoDetect, stoptag);
#endif
            metadata.getDataset()->findAndGetOFString(soptag, value, 0);
            sopuid = QString(value.c_str());

            if (first) {
                metadata.getDataset()->findAndGetOFString(seriestag, value, 0);
                wi->images_cg[i].seriesUID = value.c_str();
                first = false;
            }

            wiFiles->addFile(sopuid, QString("application/dicom"), nameStr + dcmName, blank, blank, dcmFile);
        }
    }

    QJsonObject wiObject;
    wi->writeWorkItemListEntry(wiObject);
    QJsonDocument saveDoc(wiObject);
            
    // transfer objects

    wiFiles->addJson(saveDoc.toJson(), QString("work-item-file"), QString("application/json"), QString("form-data; name=\"workitem"));

    // loop over targets
    for (int i=0; i < wi->targets.size(); i++) {
        wi->targets.at(i).writeTargetDef(dir, prefix, wiFiles);
    } // end-for each target

    QString urlStr = owner->getCAPgraphHost() + "/workitems/?user=" + owner->getUserName();
    urlStr += "&identifier=" + wi->getWorkItemID() + "&token=" + owner->token;

    wiFiles->setUrl( urlStr );

    wiFiles->setData( wi );
    wiFiles->setTransferReport(transferToReport);
    wi->startUpload(this, wiFiles);

    return true;
}

/**
 * @page workItemListViewingModel member functions
 */
int workItemListViewingModel::rowCount(const QModelIndex & /*owner*/) const
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  return owner->workItemList()->size();
}

int workItemListViewingModel::columnCount(const QModelIndex & /*owner*/) const
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  return MAXCOL;
}

QVariant workItemListViewingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (role == Qt::DisplayRole) {
    if (orientation == Qt::Horizontal) {
      switch (section) {
        case workItemPriority_col: return QString(tr(workItemPriority_heading));
        case individualID_col: return QString(tr(individualID_heading));
        case patientName_col: return QString(tr(patientName_heading));
        case alsoKnownAs_col: return QString(tr(alsoKnownAs_heading));
        case dob_col: return QString(tr(dob_heading));
        case sex_col: return QString(tr(sex_heading));
        case appliesDate_col: return QString(tr(appliesDate_heading));
        case timepoint_col: return QString(tr(timepoint_heading));
        case acqrep_col: return QString(tr(acqrep_heading));
        case subjectID_col: return QString(tr(subjectID_heading));
        case indication_col: return QString(tr(indication_heading));
        case age_col: return QString(tr(age_heading));
        case series_col: return QString(tr(series_heading));
        case targets_col: return QString(tr(targets_heading));
        case lastStep_col: return QString(tr(lastStep_heading));
        case by_col: return QString(tr(by_heading));
        case on_col: return QString(tr(on_heading));
      }
    }
    else {
      if (owner->workItemList()->at(section).unsavedChanges) {
        QString idText = "*"; // just a screen hint to user that the work item has unsaved changes
        idText.append(owner->workItemList()->at(section).getWorkItemID());
        return idText;
      }
      else {
        return owner->workItemList()->at(section).getWorkItemID();
      }
      //return workItemList()->at(section).getWorkItemID(); // QString("Select: %1").arg(section); // QString(" Select ");
    }
  }
  return QVariant();
}

QVariant workItemListViewingModel::data(const QModelIndex &index, int role) const
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (role == Qt::DisplayRole)
  {
    switch (index.column()) {
      case individualID_col: return owner->workItemList()->at(index.row()).getIndividualID();
      case patientName_col: return owner->workItemList()->at(index.row()).getPatientName();
      case alsoKnownAs_col: return owner->workItemList()->at(index.row()).getAlsoKnownAs();
      case dob_col: return owner->workItemList()->at(index.row()).getDOB();
      case sex_col: return owner->workItemList()->at(index.row()).getSex();
      case appliesDate_col: return owner->workItemList()->at(index.row()).getAppliesDate();
      case timepoint_col: return QString::number(owner->workItemList()->at(index.row()).getTimepoint());
      case acqrep_col: return QString::number(owner->workItemList()->at(index.row()).getAcqrep());
      case subjectID_col: return owner->workItemList()->at(index.row()).getSubjectID();
      case indication_col: return owner->workItemList()->at(index.row()).getIndication();
      case age_col: return owner->workItemList()->at(index.row()).getAge();
      case series_col: return owner->workItemList()->at(index.row()).images_cg.size();
      case targets_col: return owner->workItemList()->at(index.row()).targets.size();
      case lastStep_col: {
        // search for last update:
        QString lastStep;
        QDateTime dateTime=QDateTime::fromString("1900-01-01", Qt::ISODate), u_dateTime;
        foreach (const workItemUpdate u, owner->workItemList()->at(index.row()).updates) {
          u_dateTime = QDateTime::fromString(u.performDateTime, Qt::ISODate);
          if (u_dateTime > dateTime)
            lastStep = u.step;
        }
        return lastStep;
      }
      case by_col: {
        // search for last update:
        QString lastModifiedBy;
        QDateTime dateTime=QDateTime::fromString("1900-01-01", Qt::ISODate), u_dateTime;
        foreach (const workItemUpdate u, owner->workItemList()->at(index.row()).updates) {
          u_dateTime = QDateTime::fromString(u.performDateTime, Qt::ISODate);
          if (u_dateTime > dateTime)
            lastModifiedBy = u.performer;
        }
        return lastModifiedBy;
      }
      case on_col: {
        // search for last update:
        QDateTime dateTime=QDateTime::fromString("1900-01-01", Qt::ISODate), u_dateTime;
        foreach (const workItemUpdate u, owner->workItemList()->at(index.row()).updates) {
          u_dateTime = QDateTime::fromString(u.performDateTime, Qt::ISODate);
          if (u_dateTime > dateTime)
            dateTime = u_dateTime;
        }
        return dateTime.toString(Qt::ISODate);
      }
    }
  }
  return QVariant();
}

bool workItemListViewingModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  auto& entry = (*owner->workItemList())[index.row()];
  if (entry.getSurvey()) {
    // being non-null says that the work item has an open analysis tab
    owner->message->showMessage(tr("Please close the open analysis tab for this work item before editing its fields."));
    return false;
  }
  if (role == Qt::EditRole) {
    switch (index.column()) {
      case workItemPriority_col: { entry.setWorkItemPriority(value.toString()); break; }
      case individualID_col: { entry.changeIndividualID(value.toString()); break; }
      case patientName_col: { entry.setPatientName(value.toString()); break; }
      case alsoKnownAs_col: { entry.setAlsoKnownAs(value.toString()); break; }
      case sex_col: { entry.setSex(value.toString()); break; }
      case subjectID_col: { entry.setSubjectID(value.toString()); break; }
      case indication_col: { entry.setIndication(value.toString()); break; }
      case timepoint_col: { entry.setTimepoint(value.toInt()); break; }
      case acqrep_col: { entry.setAcqrep(value.toInt()); break; }
      case series_col: 
      case targets_col: 
      case lastStep_col: 
      case by_col: 
      case on_col: {
        owner->message->showMessage(tr("Cannot edit this field directly, it is updated programmatically."));
        return false;
      } 
      default: {
        owner->message->showMessage(tr("Cannot edit this field in the application, need to change it at the source."));
        return false;
      }
    }
  }

  // log an update, emit the data changed signal for the row (so as to include the update fields too), and return.
  QString step = "workItem::specification";
  entry.logWorkItemListEntryUpdate(step, entry.getWorkItemID(), NULL, owner);
  entry.dataSentToServer = false;
  emit dataChanged(index.sibling(index.row(),0), index.sibling(index.row(),MAXCOL));
  return true;
}

void workItemListViewingModel::addWorkItem()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QModelIndex index;
  insertRows(0, 1, index);
}

bool workItemListViewingModel::insertRows(int position, int rows, const QModelIndex &indexp)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  beginInsertRows(QModelIndex(), position, position+rows-1);
  QString step = "workItem::generation";
  auto& entry = (*owner->workItemList())[0];
  entry.logWorkItemListEntryUpdate(step, entry.getWorkItemID(), NULL, owner);
  endInsertRows();
  return true;
}

QMutex llock;
QMap<QString, wilist*> lockedList = QMap<QString, wilist*>();
 
wilist* getWIlist(cap *p, QString pathname)
{               
    wilist* rv;

    llock.lock();
    if (lockedList.contains(pathname)) {
        rv = lockedList.value(pathname);
    } else {
        rv = new wilist(p, pathname);
        lockedList.insert(pathname, rv);
    }
    llock.unlock();
    return rv;
}           

wilistentry * getListEntry(KeyListMap kmap, QString key)
{
    wilistentry* old = NULL;
    KeyListMap::iterator iter = kmap.find(key);
    if (iter != kmap.end())
        old = kmap[key];
    return old;
}

// setFileTime should work on windows as it's using the POSIX API 
// if not, CreateFile / SetFileTime / CloseHandle

void setFileTime(QFile &target, double ftime)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
#ifdef _WIN32
    // convert unix EPOCH seconds to a WIN FileTime
    unsigned __int64 tstamp = (unsigned __int64) ftime;
    LONGLONG ll = Int32x32To64(tstamp, 10000000) + 116444736000000000;
    FILETIME msftime;
    msftime.dwLowDateTime = (DWORD) ll;
    msftime.dwHighDateTime = ll >> 32;
    HANDLE targetfile = CreateFileA(target.fileName().toLatin1().data(),
                        FILE_WRITE_ATTRIBUTES, FILE_SHARE_READ|FILE_SHARE_WRITE,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    SetFileTime(targetfile, &msftime, NULL, &msftime);
    CloseHandle(targetfile);
#else  /* _WIN32 */
    struct utimbuf ctime;

    ctime.actime = ctime.modtime = (int) ftime;

    // fails for utf filenames
    utime( target.fileName().toLatin1().data(), &ctime);
#endif /* _WIN32 */
#else  /* QT_VERSION */
    QDateTime qtime;
    qtime.setMSecsSinceEpoch( ftime * 1000 );
    target.setFileTime(qtime, QFileDevice::FileModificationTime);
#endif
}

void setFileTime(QString fromPath, QString target)
{
    QFile fileref(fromPath);
    fileref.open(QIODevice::ReadOnly);

    QFile targetref(target);
    targetref.open(QIODevice::ReadOnly);

    setFileTime(targetref, QT_DATETIME_SECS(QT_FILETIME(fileref)) );
}

void
workItem::initiateUpload(workItemListEntry *wie, manifest *wiFiles, manifestQueue *mfq)
{
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    wiFiles->createUpload(multiPart);

    while (!mfq->isEmpty()) {
        fileinfo *fi = mfq->getEntry();
        if (fi->key == "work-item-file")
            wiFiles->addJson(multiPart, fi->json, fi->key, fi->header, fi->dispo);
        else 
            wiFiles->addFile(fi->key, fi->header, fi->dispo, "", "", fi->fpath);
        delete fi;
    }

    QUrl transferUrl(mfq->getUrl());
    QNetworkRequest request(transferUrl);

    workitem_callback_data *cbdata = new workitem_callback_data(wie, mfq->getTransferReport());
    request.setOriginatingObject( cbdata );

    cbdata->setReply( UpdateUPSWorkitemRAD.post(request, multiPart) );
    multiPart->setParent(cbdata->getReply());
}

void
workItem::makeUpload(workItemListEntry *wi, manifest *wiFiles, workitem_callback_data *cbdata) 
{
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    wiFiles->createUpload(multiPart);

    workitem_callback_data *upload = new workitem_callback_data(cbdata);
    QUrl transferUrl(cbdata->getUploadStr());
    QNetworkRequest request(transferUrl);

    request.setOriginatingObject( upload );
    upload->setReply( UpdateUPSWorkitemRAD.post(request, multiPart) );

    multiPart->setParent(upload->getReply());
}

void workItemListEntry::startUpload(workItem *wi, manifestQueue *mfq) 
{
    QMutexLocker locker(uploadLock);
    // upload in progress, don't start a new one
    if (files->hasFilesToUpload() || (0 < uploadCount) ) {
        uploadQueue.enqueue(mfq);
        return;
    }
    wi->initiateUpload(this, files, mfq);
    uploadCount++;
}

void workItemListEntry::checkPendingUpload(workItem *wi)
{
    QMutexLocker locker(uploadLock);
    if (!uploadQueue.isEmpty()) {
        manifestQueue *mfq = uploadQueue.head();
        wi->initiateUpload(this, files, mfq);
        uploadCount++;
        uploadQueue.dequeue();
        delete mfq;
    }
}

void workItemListEntry::makeUpload(workItem *wi, workitem_callback_data *cbdata)
{
    QMutexLocker locker(uploadLock);
    wi->makeUpload(this, files, cbdata);
    uploadCount++;
}

/** @} */
