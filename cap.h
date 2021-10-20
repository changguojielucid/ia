// Copyright (c) Elucid Bioimaging
#ifndef CAP_H
#define CAP_H

#include "StatusWatcher.h"
#include "workItem.h"
#include "ImageDatabase.h"
#include "seriesSurvey.h"
#include "targetDefine.h"
#include "patientAnalyze.h"
#include "patientReport.h"
#include "processingParameters.h"
#include "systemPreferences.h"

#include <QMainWindow>
#include <QWidget>
#include <QToolButton>
#include <QStackedWidget>
#include <QSocketNotifier>
#include <QNetworkReply>
#include <QWebView>
#include <QCloseEvent>
#include <QListWidgetItem>
#include <QJsonObject>

#include <string>

/**
 * @{ 
 ** 
 * @brief Computer-Aided Phenotyping (CAP) Package
 *
 * The CAP package comprises the classes used at the highest levels of the application, namely, cap, StatusWatcher, systemPreferences, 
 * capTools, Base64, CAP license, and QITKProgressDialog.
 *
 *  \copyright Elucid Bioimaging
 *  \ingroup cap
 */

// VERSION HISTORY
// 1.0.0 - long-used default setting (through November 2016)
// A.0.1 - build system revamped (through December 2016)
// A.0.2 - major workItem package enhancement
// A.1   - first major release
// A.1.1 - CPR and segmentation editor (September 2017)
// A.1.2 - small vessel support (December 2018)
// A.2   - HIS interoperability release
// A.3   - IPH and other image processing enhancement release
#define CAP_VERSION "A.3"

#define ELUCID_COMPANY_ADDRESS "Elucid Bioimaging Inc., 2 Park Plaza, Suite 700, Boston, MA 02116 USA"
#define MAXSESSIONITEMS 100

#define EXAM_DATA_FOLDER "/CAP Exam Data"
#define IMAGES_FOLDER "/Images"
#define QUERY_FOLDER "/tmp/Query"
#define RETRIEVE_FOLDER "/tmp/Retrieve"
#define ACTIVITY_LOGS_FOLDER "/CAP Activity Logs"
#define SESSION_BACKUPS_FOLDER "/CAP Session Backups"
#define WORKING_STORAGE_FOLDER "/Working Storage"
#define TEMPORARY_WORKITEM_FOLDER "/temporary"
#define CONFIGURATION_FOLDER "/Configuration"
#define FILE_SYSTEM_SOURCES_FOLDER "/File System Sources"
#define PACS_SOURCES_FOLDER "/PACS Sources"
#define CAPGRAPH_CONFIG_FILE "/CAPgraph_config.json"
#define DEFAULT_CAPGRAPH_HOST "http://staging.ad.elucid.biz"

#define LIST_NAME_ROLE Qt::UserRole
#define PERFORMER_ROLE (LIST_NAME_ROLE+1)
#define PERFORM_DATETIME_ROLE (LIST_NAME_ROLE+2)
#define LAST_MODIFIER_ROLE (LIST_NAME_ROLE+3)
#define LAST_MODIFIED_DATETIME_ROLE (LIST_NAME_ROLE+4)
#define NUM_ENTRIES_ROLE (LIST_NAME_ROLE+5)

namespace Ui {
  class cap;
}

/**
 * @{ 
 ** 
 * @brief class cap provides support for concurrent analysis session items.
 *
 *  - establishes which of Elucid's products has been invoked, setting up the ability to identify product-specific behaviour
 *  - based on the product identity, establishing the regulatory status and labeling accordingly
 *  - implements the tabbed structure, with tab addition and removal
 *  - enters the various specific packages based on where a given tab is in its process flow
 *  - provides the resource file with the branding images/logos
 *
 *  \ingroup cap
 */
class cap : public QMainWindow
{
  Q_OBJECT

public:
  QString thisProduct;
  QString clinicalJurisdiction;
  QString token;
  bool userLoggedInToServer;
  bool quitting;

  //! system-wide access to standardized directory names (makes full paths exist)
#if defined _MSC_VER
  // on Windows use C:/Users/Public/Documents instead of C:/Users/<USER>/Documents
  static QString getDocumentsDir() { QString dir = QString("C:/Users/Public/Documents"); QDir().mkdir(dir); return dir; }
#else
  static QString getDocumentsDir() { return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation); }
#endif
  static QString getCAPExamDataDir() { QString dir = getDocumentsDir() + EXAM_DATA_FOLDER; QDir().mkpath(dir); return dir; }
  static QString getImagesDir() { QString dir = getCAPExamDataDir() + IMAGES_FOLDER; QDir().mkpath(dir); return dir; }
  static QString getImagesQueryDir() { QString dir = getImagesDir() + QUERY_FOLDER; QDir().mkpath(dir); return dir; }
  static QString getImagesRetrieveDir() { QString dir = getImagesDir() + RETRIEVE_FOLDER; QDir().mkpath(dir); return dir; }
  static QString getConfigurationDir() { QString dir = getCAPExamDataDir() + CONFIGURATION_FOLDER; QDir().mkpath(dir); return dir; }
  static QString getConfigurationFileSystemSourcesDir() { QString dir = getConfigurationDir() + FILE_SYSTEM_SOURCES_FOLDER; QDir().mkpath(dir); return dir; }
  static QString getConfigurationPACSSourcesDir() { QString dir = getConfigurationDir() + PACS_SOURCES_FOLDER; QDir().mkpath(dir); return dir; }
  
  explicit cap(QMainWindow *p = 0, QString product = "");
  ~cap();
  void enableCAP(bool enabled);
  bool CAPisEnabled();

  //!@{ \name Initialization
  void setProduct(const char *applicationPath);
  void setCAPgraphConfig();
  void setLogs(QString out, QString err);
  QString getUserName();
  systemPreferences *systemPreferencesObject;

  //!@}

  //!@{ \name Image Database
  ImageDatabase *getImageDatabase() { return &imageDatabase; }
  
  //!@}
  
  //!@{ \name Work Items
  QList<workItemListEntry> *workItemList() { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return &workItemListEntries; }
  QString getListDirectory() { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return listDirectory; }
  void setListDirectory(QString directory) { ebLog eblog(Q_FUNC_INFO); eblog << directory.toStdString() << std::endl;  listDirectory = directory; }
  bool getUnsavedChangesFlag() { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ return unsavedChanges; }
  void setUnsavedChangesFlag(bool flag) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ unsavedChanges = flag; }
  workItemListProvenance *getListProvenance() {/*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ return &listProvenance; }
  void setListProvenance(workItemListProvenance prov) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ listProvenance = prov;}
  int getOpenAnalysisCount() { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ return openAnalysisCount; }
  void setOpenAnalysisCount(int count) { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ openAnalysisCount = count; }
  void initializeWorkItemList();
  bool saveWorkItemDataToDisk(QString name, saveFormat saveFormat, bool fullSave);
  void writeWorkItemList(QJsonObject &json);
  void selectWorkItemForAnalysis(workItemListEntry *wi, int sessionItemIndex);
  QStackedWidget *getWorkItemProcess(int sessionItemIndex) const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return sessionItems[sessionItemIndex].workItemProcess; }
  workItem *getWorkItem(int sessionItemIndex) const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return sessionItems[sessionItemIndex].select; }
  seriesSurvey *getSeriesSurvey(int sessionItemIndex) const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return sessionItems[sessionItemIndex].survey; }
  targetDefine *getTargetDefine(int sessionItemIndex) const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return sessionItems[sessionItemIndex].define; }
  patientAnalyze *getPatientAnalyze(int sessionItemIndex) const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return sessionItems[sessionItemIndex].analyze; }
  patientReport *getPatientReport(int sessionItemIndex) const { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/  return sessionItems[sessionItemIndex].report; }

  //!@}

  //!@{ \name CAPgraph
  void connectToCAPgraph();
  QString getCAPgraphHost() const { return CAPgraph_host; }

  //!@}
  std::string encode(std::string text);
  QString getISODateTime() { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;*/ 
    QDateTime dt = QDateTime::currentDateTime();
    int offset = dt.offsetFromUtc();
    dt.setOffsetFromUtc(offset);
    QString str = dt.toString(Qt::ISODate);
    return str;
  }

protected:
  QToolButton *listWorkItemsButton;
  ImageDatabase imageDatabase;

public slots:
  void save();
  bool saveAs();
  void userLoginStatusButton_clicked();
  void deleteTemporaryWorkitemFolder() { ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
      qInfo() << "deleting temporaryDir=" << cap::getCAPExamDataDir()+WORKING_STORAGE_FOLDER+TEMPORARY_WORKITEM_FOLDER;
      QDir temporaryDir(cap::getCAPExamDataDir()+WORKING_STORAGE_FOLDER+TEMPORARY_WORKITEM_FOLDER);
      temporaryDir.removeRecursively(); }

private slots:
  void listWorkItemsButton_clicked();
  void on_tabCloseRequested(int tabIndex);
  void on_currentChanged(int tabIndex);
  void on_cap_destroyed();
  void logUpdate(QString step, QString ID, stageParameters *stageParams, int sessionItemIndex);
  void catchStatusMessage(QString msg);
  void toggleNotifier(bool state, QSocketNotifier *notifier);
  void displayAboutPage();
  void displayUserGuidePage();
  void enterProductKey();
  void recheckLicense();
  void loginCompleted(QUrl);
  void parseSubscriptionRAD_86reply(QNetworkReply*);
  void parseWorkitemsReply(QNetworkReply*);
  void nullOutLoginView();
  void forceLogoutAfterInactivity();

private:
  //! data structure for recording analyses within the session.
  struct sessionItem {
    QWidget *analysis;
    QGridLayout *gridLayout_2;
    QStackedWidget *workItemProcess;
    workItem *select;
    seriesSurvey *survey;
    targetDefine *define;
    patientAnalyze *analyze;
    patientReport *report;
  };
  
  QWidget *owner;  // name changed from parent, which shadows a function in QObject
  Ui::cap *ui;
  QErrorMessage *message;
  StatusWatcher *statusWatcher;
  QThread *statusWatcherThread;
  int priorSessionItemIndex;
  QString serverIdentity;
  QWebView *loginView;
  std::string key;
  sessionItem sessionItems[MAXSESSIONITEMS];
  int numSessionItems;
  QPushButton *userLoginStatusButton;
  QMenu *fileMenu, *workItemMenu, *seriesSurveyMenu, *targetDefineMenu, *patientAnalyzeMenu, *patientReportMenu, *helpMenu;
  int openAnalysisCount;
  bool initialLoginAttempt;
  QNetworkAccessManager CAPgraphLoginSession;
  QNetworkAccessManager CAPgraphWorkitems;
  QNetworkReply *CAPgraphLoginSessionreply;
  QNetworkReply *CAPgraphWorkitemsreply;
  QTimer *loginTimer;
  QString CAPgraph_host;
  QCloseEvent capClose;
  QList<workItemListEntry> workItemListEntries;
  bool unsavedChanges;
  QString listDirectory; 
  workItemListProvenance listProvenance;
  void displayListWorkItemsButton();
  //virtual bool eventFilter(QObject *obj, QEvent *event) override;
  void setUserLoginStatus(QString identity, QString institution);
  QString getDefaultCAPgraphConfig(QString key);
  QString getCAPgraphConfig(QJsonObject config, QString key);
  virtual void closeEvent(QCloseEvent *event) override { ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; on_cap_destroyed(); }
  bool checkLicense(int nDaysBetweenChecks);
  bool promptForProductKey();
  bool getFeatureValues(uint32_t taHandle);
  void setRegulatoryLabel();
  void setLoginTimeout();

  QNetworkAccessManager downloadWorkitemData;
  void processWebError(QNetworkReply *errorCode);
  void parseWorkitemDataReply(QNetworkReply *reply);
  
signals:
  void updateWorkItemDisplaysGivenSave();
};
/** @} */

inline QString getLocalUserName()
{
    static const char HEX[] = "0123456789ABCDEF";

    //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
    QString str = qgetenv("USER");
    if (str.isEmpty())
        str = qgetenv("USERNAME");
    QByteArray str8 = str.toUtf8();
    QByteArray conv = QByteArray();

    for (int i = 0; i < str8.size(); i++) {
        int ch = 0x0ff & str8.at(i);
        if (0x80 > ch) {
            conv.append(ch);
        } else {
            conv.append('x');
            conv.append( HEX[ 0x0f & (ch >> 4) ] );
            conv.append( HEX[ 0x0f & ch ] );
        }
    }
    QString convstr = QString( conv );
    qInfo() << "Username " << convstr;
    return (str != convstr) ? convstr : str;
}

#endif // CAP_H
