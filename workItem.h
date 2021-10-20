// Copyright (c) Elucid Bioimaging
#ifndef WORKITEM_H
#define WORKITEM_H

#include "patientAnalyze.h"
#include "patientReport.h"
#include "capTools.h"
#include "workItemListFileToken.h"
#include "ebiVesselPipeline.h"
#include "ebvLinkedViewers2.h"
#include "sourceConfiguration.h"
#include "ImageDatabase.h"
#include "dicomQueryRetrieve.h"
#include "manifest.h"

#include <QWidget>
#include <QString>
#include <QComboBox>
#include <QPushButton>
#include <QList>
#include <QDebug>
#include <QDateTime>
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QFileDialog>
#include <QErrorMessage>
#include <QProgressDialog>
#include <QListWidgetItem>
#include <QTreeWidget>
#include <QMutex>
#include <QMutexLocker>

#include <iostream>
#include <memory>

// forward declarations
class cap;
class workItem;

enum saveFormat { Json, Binary };

/**
 * @{ 
 ** 
 * @brief Work Item Package
 *
 * The workItem package provides capability for listing work items, reading and saving lists, and processing individual ones.  The 
 * package comprises the following related classes:
 *
 *                 workItemProvenance  workItemListEntry
 *                          \                /
 *                               workItem
 *                          /                \
 *  EnableWorkItemSelectionWhenReaderReady      workItemListViewingModel
 *
 *  The package also includes sourceConfigurations, WorkItemTableView, ImageDatabase, dicomQueryRetrieve, and DicomImagesTreeWidget.
 *
 *  \copyright Elucid Bioimaging
 *  \ingroup workItem
 */

/**
 * @{ 
 ** 
 * @brief class workItemListProvenance to provide methods for the provenance of the workItem list as a whole.
 */
class workItemListProvenance
{
public:
  void setPerformer(const QString &name) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ performer = name; };
  QString getPerformer() const { return performer; };
  
  void setPerformDateTime(const QString &name) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ performDateTime = name; };
  QString getPerformDateTime() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return performDateTime; };
  
  void setLastModifier(const QString &name) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ lastModifier = name; };
  QString getLastModifier() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return lastModifier; };

  void setLastModifiedDateTime(const QString &name) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ lastModifiedDateTime = name; };
  QString getLastModifiedDateTime() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return lastModifiedDateTime; };

  void setListFileName(const QString &name) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ listFileName = name; };
  QString getListFileName() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return listFileName; };

  void initializeWorkItemListProvenance() { 
    ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
    performer = ""; 
    performDateTime = ""; 
    lastModifier = ""; 
    lastModifiedDateTime = ""; 
    listFileName = "";
  };

  void readWorkItemListProvenance(const QJsonObject &json) { 
    //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
          performer = json[performer_token].toString(); 
    performDateTime = json[performDateTime_token].toString(); 
    lastModifier = json[lastModifier_token].toString(); 
    lastModifiedDateTime = json[lastModifiedDateTime_token].toString(); 
    wilistVersion = json[wilist_version_token].toString(); 
  };

  void writeWorkItemListProvenance(QJsonObject &json) const { 
    //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
          json[performer_token] = performer; 
          json[performDateTime_token] = performDateTime; 
          json[lastModifier_token] = lastModifier; 
          json[lastModifiedDateTime_token] = lastModifiedDateTime;
    json[wilist_version_token] = "1.0.0"; 
  };

private:
  QString performer, performDateTime, lastModifier, lastModifiedDateTime, wilistVersion, listFileName;
};
/** @} */

class workitem_callback_data;

/**
 * \ingroup workItem
 * @{ 
 ** 
 * @brief class workItemListEntry the work-horse class for each work item.
 */
class workItemListEntry
{
public:
  QList<workItemUpdate> updates;
  QString listFileName;
  ebiVesselPipeline::Pointer pipeline;
  QList<imageSeries> images_cg;
  QList<targetDef> targets;
  bool unsavedChanges;
  bool dataSentToServer;
  QString urlStr;
  QJsonObject *wiObjectFromFile;
  void closeOutTargetsFromCompletedSession();
  bool cloned;

  workItemListEntry(cap *p) { 
    //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
    owner = p;
    wiObjectFromFile = NULL;
    images_cg.clear(); targets.clear(); updates.clear();
    unsavedChanges = true; 
    dataSentToServer = false;
    survey = NULL; define = NULL; analyze = NULL; report = NULL;
    timepoint = 0; acqrep = 0;
    uploadLock = new QMutex(QMutex::Recursive);
    uploadCount = 0;
    cloned = false;
  }

  ~workItemListEntry() { 
    //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
    images_cg.clear(); targets.clear(); updates.clear();
    unsavedChanges = false; 
    survey = NULL; define = NULL; analyze = NULL; report = NULL;
    timepoint = 0; acqrep = 0;
  }

  workItemListEntry * clone() {
    workItemListEntry *wi = new workItemListEntry(owner);
    *wi = *this;
    cloned = true;
    return wi;
  }

  void setSessionItemIndex(int index) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ sessionItemIndex = index; }

  QList<imageSeries> *getImageSeriesSet() { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return &images_cg; }

  QList<targetDef> *getTargetDefs() { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return &targets; }

  QList<workItemUpdate> *getWorkItemUpdates() { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return &updates; }

  void setTabLabel(QString label) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ tabLabel = label; }
  const QString getTabLabel() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return tabLabel; }

  void setWorkItemID(QString str) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ workItemID = str; }
  const QString getWorkItemID() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return workItemID; }

  void setWorkItemPriority(QString str) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ workItemPriority = str; }
  const QString getWorkItemPriority() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return workItemPriority; }

  void setSubjectType(QString str) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ subjectType = str; }
  const QString getSubjectType() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return subjectType; }

  void setIndividualID(QString str) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ individualID = str; }
  void changeIndividualID(QString str);
  const QString getIndividualID() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return individualID.trimmed().isEmpty() ? "EMPTY" : individualID; }

  void setPatientName(QString str) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ patientName = str; }
  const QString getPatientName() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return patientName; }

  void setAlsoKnownAs(QString str) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ alsoKnownAs = str; }
  const QString getAlsoKnownAs() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return alsoKnownAs; }

  void setDOB(QString str) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ DOB = str; }
  const QString getDOB() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return DOB; }

  void setSex(QString str) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ sex = str; }
  const QString getSex() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return sex; }

  void setAppliesDate(QString str) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ appliesDate = str; }
  const QString getAppliesDate() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return appliesDate; }

  void setSubjectID(QString str) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ subjectID = str; }
  const QString getSubjectID() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return subjectID; }

  void setIndication(QString str) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ indication = str; }
  const QString getIndication() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return indication; }

  void setAge(QString str) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ age = str; }
  const QString getAge() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return age; }

  void setModelOrganism(QString str) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ modelOrganism = str; }
  const QString getModelOrganism() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return modelOrganism; }

  void setAccession(QString str) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ accession = str; }
  const QString getAccession() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return accession; }

  void setTimepoint(int tp) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ timepoint = tp; }
  const int getTimepoint() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return timepoint; }

  void setAcqrep(int rep) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ acqrep = rep; }
  const int getAcqrep() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return acqrep; }

  void readWorkItemListEntry(const QJsonObject &json);
  void writeWorkItemListEntry(QJsonObject &json) const;

  bool update(workItemListEntry *oentry);

  void logWorkItemListEntryUpdate(QString step, QString ID, stageParameters *stageParams, workItem *workItemOwner);

  void pushWorkItemParametersToPipeline(QString clinicalJurisdiction) const { 
    ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
    //SetMetaData(pipeline, clinicalJurisdiction_token, clinicalJurisdiction.toStdString()); 
    SetMetaData(pipeline, subjectType_token, subjectType.toStdString()); 
    SetMetaData(pipeline, indication_token, indication.toStdString()); 
    SetMetaData(pipeline, sex_token, sex.toStdString()); 
    SetMetaData(pipeline, age_token, age.toStdString()); 
    SetMetaData(pipeline, modelOrganism_token, modelOrganism.toStdString()); 
  }

  seriesSurvey *getSurvey() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ return survey; }
  void setSurvey(seriesSurvey *s) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ survey = s; }

  targetDefine *getDefine() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ return define; }
  void setDefine(targetDefine *d) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ define = d; }

  patientAnalyze *getAnalyze() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ return analyze; }
  void setAnalyze(patientAnalyze *a) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ analyze = a; }

  patientReport *getReport() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ return report; }
  void setReport(patientReport *r) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ report = r; }

  void setWorkItemFolder(QString str) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ workItemFolder = str; }

  QString getWorkItemFolder() const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ return workItemFolder; }
  bool setWorkItemFolder();

  manifest *getManifest()
  { 
      QMutexLocker locker(uploadLock);
      if (NULL == files) files = new manifest();
      return files;
  }

  manifestQueue *getManifestQueue()
  { 
      getManifest();
      return new manifestQueue();
  }

  void uploadComplete()
  {
    QMutexLocker locker(uploadLock);
    uploadCount--;
  }

  bool isUploadComplete()
  {
    QMutexLocker locker(uploadLock);
    return !files->hasFilesToUpload() && (0 == uploadCount);
  }

  int getUploadCount()
  {
    QMutexLocker locker(uploadLock);
    return uploadCount;
  }

  int continueUpload(workitem_callback_data *cbdata)
  {
    QMutexLocker locker(uploadLock);
    // upload in progress, no files or too many concurrently running or processing partial file
    if (!(files->hasFilesToUpload() && (6 > uploadCount)) || (files->hasPartialFile() && (0 != uploadCount)))
        return 0;
    return 1;
  }

  void makeUpload(workItem *wi, workitem_callback_data *cbdata);
  void startUpload(workItem *wi, manifestQueue *mfq);
  void checkPendingUpload(workItem *wi);

  void reconcile(QString key, int fsize, double fdate)
  {
    QMutexLocker locker(uploadLock);
    files->reconcile(key, fsize, fdate);
  }

private:
  cap *owner;
  QString tabLabel;
  int sessionItemIndex;
  seriesSurvey *survey;
  targetDefine *define;
  patientAnalyze *analyze;
  patientReport *report;
  QString workItemFolder;
  QString workItemID, workItemName, workItemPriority, subjectType, individualID, patientName, alsoKnownAs, DOB, sex, appliesDate, subjectID, indication, age, modelOrganism, accession;
  int timepoint, acqrep;
  manifest *files = NULL;
  QMutex *uploadLock;
  QQueue<manifestQueue *> uploadQueue;
  int uploadCount;
};
/** @} */

class workitem_callback_data : public QObject
{
    Q_OBJECT

#define WI_STATE_UPLOAD 1
#define WI_STATE_FILES  2
#define WI_STATE_META   3
#define WI_STATE_POST   4
#define WI_STATE_FINAL  5

public:
    workitem_callback_data(workItemListEntry *entry, bool forReport)
    {
        workitem = entry;
        doReport = forReport;
        uploads = entry->getManifest()->getUploads();
    }

    workitem_callback_data(workitem_callback_data *cb)
    {
        workitem = cb->workitem;
        doReport = cb->doReport;
        uploads = cb->workitem->getManifest()->getUploads();
        uploadStr = cb->uploadStr;
        workItemState = cb->workItemState;
    }

    ~workitem_callback_data()
    {
        if (NULL != uploads)
            delete uploads;
    }

    workItemListEntry *entry() { return workitem; }
    bool isReport() { return doReport; }
    void setUploadState(int state) { workItemState = state; }
    int getUploadState() { return workItemState; }

    QQueue<fileinfo *> *getUploads() { return uploads; }

    void setReply(QNetworkReply *reply) { UpdateUPSWorkitemRADreply = reply; }
    QNetworkReply *getReply() { return UpdateUPSWorkitemRADreply; }

    void setUploadStr(QString str) { uploadStr = str; }
    QString getUploadStr() { return uploadStr; }

    QString getUploadStateStr() { return uploadStateStr[ workItemState ]; }

private:
    workItemListEntry *workitem;
    bool doReport;
    QQueue<fileinfo *>* uploads = NULL;
    int workItemState = WI_STATE_UPLOAD;
    QString uploadStr;
    QNetworkReply *UpdateUPSWorkitemRADreply;
    static QString uploadStateStr[WI_STATE_FINAL + 1];
};

namespace Ui {
class workItem;
}

/**
 * \ingroup workItem
 * @{ 
 ** 
 * @brief class workItem the main class with the list as a whole and the methods to display and interact with it.
 */
class workItem : public QWidget
{
  Q_OBJECT

public:
  QString tabLabel;
  int sessionItemIndex;
  explicit workItem(QWidget *p=0, int index=0, QMenu *m=NULL, bool masterWithRespectToMenu=false);
  ~workItem();
  cap *getOwner() const { return owner; }
  QMenu *workItemMenu;
  QErrorMessage *message;
  QList<workItemListEntry> *workItemList();
  workItemListProvenance *getListProvenance();
  void setListProvenance(workItemListProvenance prov);
  int selectedWorkItem;
  bool selectedWorkItemValid;
  workItemListEntry *selectedWorkItemListEntryPtr;
  QList<QString> allowableSeriesTypes;
  void newWorkItemList();

  bool loadWorkItemListFromFile(QString name, saveFormat saveFormat);
  void readWorkItemList(const QJsonObject &json);
  
  void logListProvenanceUpdate();
  bool getUnsavedChangesFlag();
  void setUnsavedChangesFlag(bool flag);
  int getOpenAnalysisCount();
  void setOpenAnalysisCount(int count);
  QString getListFileName();
  void disconnectMenuActions();
  void connectMenuActions();
  void reconnectMenuActions();
  void disableMenuActions();
  QListWidgetItem *getCurrentlyOpenWorkItemListItem() { return currentlyOpenWorkItemListItem; }
  void setEntryUnsavedChangesFlag(int entryIndex, bool flag);
  QString getISODateTime();
  bool shipDataToServer(QString dir, workItemListEntry *wi, bool transferToReport);
  void prepChangedDataForServer(QString wiFolder, workItemListEntry *wi);
  void populateImportedDICOMImages();

  void initiateUpload(workItemListEntry *wie, manifest *wiFiles, manifestQueue *mfq);
  void makeUpload(workItemListEntry *wi, manifest *wiFiles, workitem_callback_data *cbdata);
									   
public slots:
  unsigned int importImages(QString path, bool recursive, bool requireOriginal, bool excludeLocalizer, bool excludePreContrast, bool showProgress);
  QString getTabLabel() { ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;  return(tabLabel); }
  void workItemSelectedForAnalysis(int i); // could be private except need for automated unit test
  void on_deleteWorkItemButton_clicked();
  void on_addSourceButton_clicked();
  void on_proceedButton_clicked();
  void useSeriesInCurrentWorkItemList();
  void updateWorkItemDisplaysGivenSave();
  void on_recentlyImportedImages_clicked() { populateImportedDICOMImages(); }
  void on_allImportedImages_clicked() { populateImportedDICOMImages(); }
  
protected:
  cap *owner;  // name changed from parent, which shadows a function in QObject; type changed from QWidget * to cap *
  Ui::workItem *ui;
  ImageDatabase *getImageDatabase();
  
private:
  void setupModel();
  void addWorkItem();
  void blankPreviewer();
  void updateWorkItemListNameText();
  void populateWorkItemLists(QString itemToLeaveSelected);
  bool eventFilter(QObject *obj, QEvent *event);
  void workItemSelected(int i);
  QString workItemListFileName;
  QObject *source;
  QObject *proxy;

  QNetworkAccessManager GetAuthorizationTokenITI_71;
  QNetworkAccessManager UpdateUPSWorkitemRAD;

  QNetworkReply *GetAuthorizationTokenITI_71reply;

  QProgressDialog *generateProgressIndicator;
  QAction *addWorkItemAction, *proceedAction, *saveListChangesAction;
  std::map<QAction *, bool> *menuActionEnabledMap;
  vtkSmartPointer<ebvLinkedViewers2> viewers;
  ebID ida, idc, ids; // previewer identifiers (axial, coronal, sagittal) 

  // import sources
  struct ImportSource {
    ImportSource() { item = nullptr; queryRetrieve = nullptr; config = nullptr; }
    ~ImportSource() { if (queryRetrieve) delete queryRetrieve; if (config) delete config; }  // item will be cleared by ui->sources
    QListWidgetItem     *item;
    sourceConfiguration *config;
    dicomQueryRetrieve  *queryRetrieve;  // only used if PACS
  };
  std::vector<std::shared_ptr<ImportSource>> importSources;

  QListWidgetItem *currentlyOpenWorkItemListItem;
  ebiMultiImageReader::Pointer multiReader; // for current series
  ebID imageID, image4ID; // for current series
  QVBoxLayout *seriesSetBoxLayout;
  QList<QLineEdit *> seriesFolders;
  QList<QComboBox *> seriesTypes;
  QFormLayout *seriesSetEntryLayout;

private slots:
  void ensureOnWorkItemPage();
  void workItemRowSelected(int i);
  void on_addWorkItemButton_clicked();
  void on_workItemList_clicked(const QModelIndex &index);
  void packageDataAndTransitionToReport();
  void parseReportAuthorizationTokenITI_71reply(QNetworkReply*);
  void parseUPSWorkitemContentsRADreply(QNetworkReply*);
  void processWebError(QNetworkReply*);
  //void processWebSslErrors(QList<QSslError>)));
  void on_sources_itemClicked(QListWidgetItem *item);
  void setSelectedItem(QTreeWidgetItem *item);
  void on_dicomImagesTreeWidget_itemPressed(QTreeWidgetItem *item);
  void on_workItemLists_itemClicked(QListWidgetItem *item);
  void showSourceContextMenu(const QPoint&);
  void configureSource();
  void removeSource();
  void showWorkItemListContextMenu(const QPoint&);
  void deleteWorkItemList();

signals:
  void addWorkItemToModel();
  void giveScreenControlToReport(QString ID, QUrl url, QProgressDialog *generateProgressIndicator);
};
/** @} */

/**
 * \ingroup workItem
 * @{ 
 ** 
 * @brief class workItemListViewingModel a support class which implements the abstract model that serves as the basis for the actual list display.
 */
class workItemListViewingModel : public QAbstractTableModel
{
  Q_OBJECT

public:
  workItemListViewingModel(workItem *workItemOwner) { 
    ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
    owner = workItemOwner; 
    connect(owner, SIGNAL(addWorkItemToModel()), this, SLOT(addWorkItem()));
  };
  int rowCount(const QModelIndex &owner = QModelIndex()) const Q_DECL_OVERRIDE;
  int columnCount(const QModelIndex &owner = QModelIndex()) const Q_DECL_OVERRIDE;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
  QVariant headerData(int section, Qt::Orientation orientation, int role) const Q_DECL_OVERRIDE;
  bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole) Q_DECL_OVERRIDE;
  bool insertRows(int position, int rows, const QModelIndex &owner);
  Qt::ItemFlags flags(const QModelIndex & index) const Q_DECL_OVERRIDE { 
    //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
    return Qt::ItemIsSelectable |  Qt::ItemIsEditable | Qt::ItemIsEnabled | QAbstractTableModel::flags(index); 
  }

private:
  workItem *owner;

private slots:
  void addWorkItem();
};
/** @} */

// class workItem supports managing the workitem list for the UI
// class wilist supports managing the workitem list for download update

class wilist;

wilist* getWIlist(cap *p, QString pathname);

class wilistentry
{
public:
    wilistentry(QString k, wilist *v, wilistentry *existing) {
        key = k;
        value = v;
        next = existing;
    }

    wilist *getValue() { return value; }

    wilistentry *getNext() { return next; }

private:
    QString key;
    wilist *value;
    wilistentry *next;
};

typedef QMap<QString, wilistentry *> KeyListMap;

wilistentry * getListEntry(KeyListMap kmap, QString key);

class wientry : public workItemListEntry
{
public:
    wientry(cap *p) : workItemListEntry(p) { }
    bool doDelete() { return deleted; }
    void setDelete(bool v) { added = false; deleted = v; }

    bool doAdd() { return added; }
    void setAdded(bool v) { deleted = false; added = v; }

private:
    bool deleted = false;
    bool added = false;
};

class wilist
{
public:
    wilist(cap *p, QString winame)
    {
        this->parent = p;
        this->winame = winame;
        existing = QMap<QString, wientry*>();
        QStringList pathparts = winame.split("/");
        QString path = pathparts.mid(0, pathparts.size() - 1).join("/");
        widir = new QDir( path );
    }

    ~wilist()
    {
        clear();
        delete widir;
    }

    void clear()
    {
        foreach (wientry *wi, existing) {
            delete wi;
        }
        existing.clear();
    }
    

    QDir *getWIDir() { return widir; }

    void addEntry(KeyListMap &kmap, wientry *wie)
    {
        QString key = wie->getWorkItemID();
        kmap[key] = new wilistentry(key, this, getListEntry(kmap, key));
    }

    void readList(KeyListMap &kmap) 
    {
        QFile wifile( winame );
        if (widir->exists() && QFile::exists(wifile.fileName()) && wifile.open(QIODevice::ReadOnly)) {
            QByteArray data = wifile.readAll();
            QJsonDocument loadDoc(QJsonDocument::fromJson(data));
            QJsonObject obj = loadDoc.object();

            QJsonArray wiArray = obj[workItemList_token].toArray();
            for (int idx = 0; idx < wiArray.size(); idx++) {
                wientry *entry = new wientry(parent);
                entry->wiObjectFromFile = new QJsonObject(wiArray[idx].toObject());
                entry->readWorkItemListEntry(*entry->wiObjectFromFile);
                existing[entry->getWorkItemID()] = entry;
                addEntry(kmap, entry);
            }    
            provenance.readWorkItemListProvenance( obj[listProvenance_token].toObject() );
            wifile.close();
        }
    }

    void writeList(QString user, QString date)
    {
        QJsonObject newObject;
        QJsonObject listProvenanceObject;

        if ("" == provenance.getPerformer()) {
            provenance.setPerformer( user );
            provenance.setPerformDateTime( date );
        }
        provenance.setLastModifier( user );
        provenance.setLastModifiedDateTime( date );
        provenance.writeWorkItemListProvenance(listProvenanceObject);

        newObject[listProvenance_token] = listProvenanceObject;

        QJsonArray wiArray;
        for (auto &key : existing.keys()) {
            wientry *wi = existing[key];
            if (wi->doDelete()) {
                continue;
            }
            QJsonObject wiObject;
            wi->writeWorkItemListEntry(wiObject);
            wiArray.append(wiObject);
        }
        newObject[workItemList_token] = wiArray;
        QJsonDocument saveDoc(newObject);
        if (!widir->exists())
            QDir().mkdir(widir->path());
        QFile newFile( winame );
        if (!newFile.open(QIODevice::WriteOnly)) {
            qInfo() << "cannot open output for write " << newFile.fileName();
            return;
        }
        newFile.write(saveDoc.toJson());
        qInfo() << "wrote updated workitem file " << newFile.fileName();
        newFile.close();
    }

    bool wiExists(QString key)
    {
        QMap<QString, wientry*>::iterator iter = existing.find(key);
        return iter != existing.end();
    }

    void remove(QString key)
    {
        wientry *entry = existing[key];
        entry->setDelete(true);
        updated = true;
    }

    void add(QString wijson)
    {
        wientry *entry = new wientry(parent);
        QJsonDocument wi = QJsonDocument::fromJson( wijson.toLocal8Bit() );
        entry->readWorkItemListEntry( wi.object() );
        existing[entry->getWorkItemID()] = entry;
        updated = true;
    }

    void update(wientry *curr, wientry *newwi)
    {
        if (newwi->update(curr))
            updated = true;
    }

    wientry * parse(QString wijson)
    {
        wientry *entry = new wientry(parent);
        QJsonDocument wi = QJsonDocument::fromJson( wijson.toLocal8Bit() );
        entry->readWorkItemListEntry( wi.object() );
        return entry;
    }

    wientry * get(QString key)
    {
        QMap<QString, wientry*>::iterator iter = existing.find(key);
        if (iter != existing.end())
            return existing[key];
        return NULL;
    }
    
    bool isUpdated() { return updated; }

    bool trylock()
    {
        return flock.tryLock(0);
    }

    void unlock()
    {
        flock.unlock();
    }

private:
    bool updated = false;
    cap *parent;
    QString winame;
    QMutex flock;

    QDir *widir;
    QMap<QString, wientry*> existing;
    workItemListProvenance  provenance;
};


#endif // WORKITEM_H
