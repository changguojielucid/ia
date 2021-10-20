// Copyright (c) Elucid Bioimaging
#include "cap.h"
#include "ui_mainWindow.h"
#include "capTools.h"
#include "CAP license.h"
#include "ebSystem.h"
#include "Base64.h"

#include <QDesktopWidget>
#include <QDesktopServices>
#include <QDirIterator>
#include <QLabel>
#include <QPushButton>
#include <QErrorMessage>
#include <QMessageBox>
#include <QProgressDialog>
#include <QRegExp>
#include <QTextEdit>
#include <QFont>
#include <QTimer>
#include <QProcess>

#include <iostream>

/**
 * \ingroup cap
 * @{
 *
 * See cap.h for description of the CAP package purpose and contents.  This file has member functions for classes in the package.
 */

// the following is needed for the LimeLM licensing
#include <stdio.h>

// Support Unicode compilation and non-Windows compilation
#ifdef _WIN32
  #include <tchar.h>
#else
  #define _T(x) x
  typedef char    TCHAR;
#endif

// To use the static version of TurboActivate then uncomment the next line
//#define TURBOACTIVATE_STATIC

// Include the correct library on Windows
#ifdef TURBOACTIVATE_STATIC
  #ifdef _DEBUG
    #ifdef _DLL
      #pragma comment(lib, "TurboActivate-MDd.lib")
    #else
      #pragma comment(lib, "TurboActivate-MTd.lib")
    #endif
  #else
    #ifdef _DLL
      #pragma comment(lib, "TurboActivate-MD.lib")
    #else
      #pragma comment(lib, "TurboActivate-MT.lib")
    #endif
  #endif
#else
  #pragma comment (lib, "TurboActivate.lib")
#endif

#include "TurboActivate.h"

// paste your Version GUID here.
#if defined _MSC_VER
  #define TA_GUID L"2403c0955751bff5d49d09.25094934"
  #define CLINICAL_EDITION_FEATURE L"Clinical Edition"
#else
  #define TA_GUID _T("2403c0955751bff5d49d09.25094934")
  #define CLINICAL_EDITION_FEATURE _T("Clinical Edition")
#endif


class wi_download_data : public QObject
{
#define WI_DL_START   1
#define WI_DL_DICOM   2
#define WI_DL_FILE    3

public:
    wi_download_data()
    {
        dlState = WI_DL_START;
    }

    wi_download_data(wi_download_data *cb)
    {
        dlState = cb->dlState;
        url = cb->url;
        ident = cb->ident;
    }

    QString getStateStr() { return StateStr[ dlState ]; }

    void setReply(QNetworkReply *reply) { netReply = reply; }
    QNetworkReply *getReply() { return netReply; }

    void setState(int state) { dlState = state; }
    int getState() { return dlState; }

    void setUrl(QString str) { url = str; }
    QString getUrl() { return url; }

    void setIdentifier(QString str) { ident = str; }
    QString getIdentifier() { return ident; }

    void setFilename(QString str) { filename = str; }
    QString getFilename() { return filename; }

    void setExisting(bool exists) { existing = exists; }
    bool isExisting() { return existing; }

    void setWIEntry(wientry *ent) { entry = ent; }
    wientry *getWIEntry() { return entry; }

    void setTimestamp(double ts) { timestamp = ts; };
    double getTimestamp() { return timestamp; };

private:
    int dlState;
    QString url;
    QString ident;
    QString filename;
    QNetworkReply *netReply;
    static QString StateStr[WI_DL_FILE + 1];
    wientry *entry = NULL;
    double timestamp;

    bool existing = false;
};

QString wi_download_data::StateStr[] = {
    QString(""),
    QString("START"),
    QString("DICOM"),
    QString("FILE"),
};


bool cap::checkLicense(int nDaysBetweenChecks)
{

  return 1;

  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  /* //BEGIN TEMPORARY FIX FOR PROBLEMATIC LIMELM
  qInfo() << "CAP is activated in Research Edition";
  clinicalJurisdiction = ""; // default to Research Edition, i.e., no clinical jurisdiction
  setRegulatoryLabel();
  return true;
  //END TEMPORARY FIX FOR PROBLEMATIC LIMELM */

  // first see if this is one of the permanently authorized systems
  if (ebSystem::VerifySystem()) {
    qInfo() << "CAP is activated based on this computer being on the permanent list!";
    clinicalJurisdiction = ""; // default to Research Edition, i.e., no clinical jurisdiction
    setRegulatoryLabel();
    return true;
  }

  // [VASCUCAP-641]: TurboActivate LimeLM Library Updates
  uint32_t taHandle = TA_GetHandle(TA_GUID);
  if (taHandle <= 0)
  {
	  qWarning() << "Unable to get the Handle for checking the License: handle(%d) \n" << taHandle;
	  return false;
  }

  // LimeLM for licensing. Look in TurboActivate.h for what the error codes mean.
  GENUINE_OPTIONS opts;
  opts.nLength = sizeof(GENUINE_OPTIONS);

  // How often to verify with the LimeLM servers (90 days)
  opts.nDaysBetweenChecks = nDaysBetweenChecks; // setting to zero should allow for immediate effect if revoke a license, but doesn't always seem to work

  // The grace period if TurboActivate couldn't connect to the servers.  After the grace period is over IsGenuinEx() will return TA_FAIL instead of TA_E_INET or TA_E_INET_DELAYED
  opts.nGraceDaysOnInetErr = 14;

  // In this example we won't show an error if the activation was done offline by passing the TA_SKIP_OFFLINE flag
  opts.flags = TA_SKIP_OFFLINE;

  HRESULT hr = TA_IsGenuineEx(taHandle, &opts);
  printf("hr=%d\n", hr);
  if (hr == TA_OK || hr == TA_E_FEATURES_CHANGED || hr == TA_E_INET || hr == TA_E_INET_DELAYED) {
    qInfo() << "CAP is activated and genuine!";
    if (hr == TA_E_INET || hr == TA_E_INET_DELAYED) {
      // the IsGenuineEx() failed to connect to the LimeLM servers.
      message->showMessage(tr("CAP is temporarily activated, but it failed to verify the activation online. Contact Elucid if this notification persists. Code: %1").arg(QString(hr)));
      this->repaint(); // ensure message is shown
      qApp->processEvents();
      clinicalJurisdiction = ""; // trials are Research Edition
      setRegulatoryLabel();
      return true;
    }
    return getFeatureValues(taHandle);
  }
  else if (hr == TA_E_REVOKED) {
    ui->analyses->setEnabled(false);
    QMessageBox msgBox(this);
    msgBox.setText(tr("Your product key can not be authorized."));
    msgBox.setInformativeText(tr("Would you like to enter a new key now?  Yes means enter key, No means exit program."));
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    int ret = msgBox.exec();
    if (ret == QMessageBox::No)
      exit(1);
    return false;
  }
  else { // not activated or genuine but not revoked either - so give trial
    uint32_t trialDays = 60;
	uint32_t trialFlags = TA_VERIFIED_TRIAL | TA_USER;

    // Start or re-validate the trial if it has already started.  This need to be called at least once before you can use any other trial functions.
	hr = TA_UseTrial(taHandle, trialFlags, NULL);

    if (hr == TA_OK) {
      // Get the number of trial days remaining.
      hr = TA_TrialDaysRemaining(taHandle, trialFlags, &trialDays);

      if (hr == TA_OK) {
        QThread::msleep(1000);
        if (trialDays > 0) {
          //printf("Trial days remaining: %d\n", trialDays);
          QString trialMessage = tr("Activating CAP on a trial basis. ") + QString::number(trialDays) + tr(" Trial days remaining.");
          message->showMessage(trialMessage);
          this->repaint(); // ensure message is shown
          qApp->processEvents();
          clinicalJurisdiction = ""; // trials are Research Edition
          setRegulatoryLabel();
          return true;
        }
        else {
          ui->analyses->setEnabled(false);
          QMessageBox msgBox(this);
          msgBox.setText(tr("CAP trial period ended."));
          msgBox.setInformativeText(tr("Would you like to enter a product key now?  Yes means enter key, No means exit program."));
          msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
          msgBox.setDefaultButton(QMessageBox::Yes);
          msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
          int ret = msgBox.exec();
          if (ret == QMessageBox::No)
            exit(2);
          return false;
        }
      }
      else {
        ui->analyses->setEnabled(false);
        qWarning() << "Code from licensing server when seeking to get trial days remaining was " << hr;
        QMessageBox msgBox(this);
        msgBox.setText(tr("Failed to get the trial days remaining, which is required for operation."));
        msgBox.setInformativeText(tr("The most common cause for this is if the internet connection is not active to allow access to the license servers.  Contact Elucid if problem persists."));
        msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
        msgBox.exec();
        exit(3);
        return false; // doesn't do anything, but just here if for whatever reason the exit is reconsidered in future.
      }
    }
	else if (hr == TA_E_TRIAL_EXPIRED || hr == TA_E_NO_MORE_TRIALS_ALLOWED)
	{
		//[VASCUCAP-655] Prompt for product key
		ui->analyses->setEnabled(false);
		QMessageBox msgBox(this);
		msgBox.setText(tr("CAP trial period ended."));
		msgBox.setInformativeText(tr("Would you like to enter a product key now?  Yes means enter key, No means exit program."));
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		msgBox.setDefaultButton(QMessageBox::Yes);
		msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
		int ret = msgBox.exec();
		if (ret == QMessageBox::No)
			exit(10);
		return false;

	} else {
      ui->analyses->setEnabled(false);
      qWarning() << "Code from licensing server when seeking to UseTrial was " << hr;
      QMessageBox msgBox(this);
      msgBox.setText(tr("Failed to UseTrial, which is required for operation."));
      msgBox.setInformativeText(tr("The most common cause for this is if the internet connection is not active to allow access to the license servers.  Contact Elucid if problem persists."));
      msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
      msgBox.exec();
      exit(4);
      return false;
    }
  }
}

bool cap::getFeatureValues(uint32_t taHandle)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  // See: http://wyday.com/limelm/help/license-features/
  HRESULT hr = TA_GetFeatureValue(taHandle, CLINICAL_EDITION_FEATURE, 0, 0); // this call returns the size needed in hr, including the terminating \0
  if (hr == 1) {
    clinicalJurisdiction = "";
    qInfo() << "Clinical jurisdiction feature obtained from license server is blank (indicating Research Edition)";
    ui->analyses->setEnabled(true);  // will not accept license key without this set 
  }
  else {
#ifdef _WIN32
          STRTYPE jurisdiction = (STRTYPE)malloc(hr * sizeof(WCHAR));
#else
    TCHAR *jurisdiction = (TCHAR *)malloc(hr * sizeof(TCHAR)); 
#endif
    hr = TA_GetFeatureValue(taHandle, CLINICAL_EDITION_FEATURE, jurisdiction, hr); // this call returns value in "jurisdiction" and the status in hr
    if (hr == TA_OK) {
                // non-Qt way to print out value:
            //#ifdef _WIN32
                //wprintf(L"Feature value: %s\n", featureValue);
            //#else
                //printf("Feature value: %s\n", featureValue);
            //#endif
#ifdef _WIN32
      clinicalJurisdiction = QString::fromStdWString(jurisdiction);
#else
      clinicalJurisdiction = QString::fromStdString(jurisdiction);
#endif
      qInfo() << "Clinical jurisdiction feature obtained from license server is " << jurisdiction << " (" << clinicalJurisdiction << ")";
      ui->analyses->setEnabled(true);
      free(jurisdiction);
    }
    else {
      ui->analyses->setEnabled(false);
      qWarning() << "Code from licensing server when seeking to get feature was " << hr;
      QMessageBox msgBox(this);
      msgBox.setText(tr("Could not identify regulatory classification, which is required for operation."));
      msgBox.setInformativeText(tr("The most common cause for this is if the internet connection is not active to allow access to the license servers.  Contact Elucid if problem persists."));
      msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
      msgBox.exec();
      exit(5);
    }
  }
  setRegulatoryLabel();
  return true;
}

void cap::enterProductKey()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  while (!promptForProductKey()) 
    ;
}

bool cap::promptForProductKey()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // prompt for a product key (given that it's not present)
              /* THIS WAS THE WAY IT WAS DONE WHEN IN cap_main, without benefit of Qt being up yet:
              printf("Please enter product key: ");
              #if defined _MSC_VER
                wchar_t productKey[100];
                wscanf(L"%ls", productKey);
              #else
                char productKey[100];
                scanf("%s", productKey);
              #endif*/
  // ...and this is the way here in cap, since Qt is up
  bool ok;
  QString code=QInputDialog::getText(this, tr("CAP Product Activation Code"), tr("Enter code:"), QLineEdit::Normal, "", &ok);
  if (!ok || code.isEmpty()) {
    qWarning() << "User cancelled product key entry";
    QMessageBox msgBox(this);
    msgBox.setText(tr("No product key entered, which is required for operation."));
    msgBox.setInformativeText(tr("If you hadn't intended to cancel, please start program again."));
    msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    msgBox.exec();
    exit(6);
  }

  uint32_t taHandle = TA_GetHandle(TA_GUID);
  if (taHandle <= 0)
  {
	  QString msgStr = QString::asprintf("Unable to contact License Library.\n");
	  qWarning() <<  msgStr;

	  QMessageBox msgBox(this);
	  msgBox.setText(tr(msgStr.toLatin1()));
	  msgBox.setInformativeText(tr("Please restart vascuCAP application and try again."));
	  msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
	  msgBox.exec();
	  exit(9);
  }

  // since we got a key, deactivate now so that the new one can be used
  HRESULT hr = TA_Deactivate(taHandle, 0); // 0 means, leave the key in place in case user wants to revert without re-entry
  if (hr == TA_OK) {
    qInfo() << "Prior key de-activated successfully as preparation for new key. Code: " << hr;
  }
  else {
    qWarning() << "Deactivation of prior key failed. Code: " << hr;
    if (ebSystem::VerifySystem()) {
      // the deactivation may have failed because they never used one before, in which case no problem
      qInfo() << "Prior key de-activated failure likely because permanent system had never used a key before, so will continue assuming no problem";
    }
    else {
      // since they were not a permanent system, they must have had a prior key
      qWarning() << "Not a permanent system, but will continue assuming no problem";
      /*QMessageBox msgBox(this);
      msgBox.setText(tr("Deactivation of prior key failed."));
      msgBox.setInformativeText(tr("While this disallows entry of new key, it will not effect operation under the prior key."));
      msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
      msgBox.exec();
      return true;*/
    }
  }

  // save the product key and try to activation.  Note we're using the TA_USER flag.  If we used the TA_SYSTEM flag, this would mean the
  // the activation would be system-wide, but not only would using the TA_SYSTEM flag (the first time only) require system-admin privileges,
  // but it wouldn't match our policy that the license is per-user.
#ifdef _WIN32
  hr = TA_CheckAndSavePKey(taHandle, code.toStdWString().c_str(), TA_USER);
#else
  hr = TA_CheckAndSavePKey(taHandle, code.toStdString().c_str(), TA_USER);
#endif
  if (hr == TA_OK) {
    qInfo() << "Product key saved successfully. Code: " << hr;
	ACTIVATE_OPTIONS activateOptions;
	memset(&activateOptions, 0, sizeof(activateOptions));
	activateOptions.nLength = sizeof(activateOptions);

    hr = TA_Activate(taHandle, &activateOptions);
    if (hr == TA_OK) {
      getFeatureValues(taHandle);
      return true;
    }
    else {
      QMessageBox msgBox(this);
      msgBox.setText(tr("CAP could not activate."));
      msgBox.setInformativeText(tr("Would you like to enter a new key now?  Yes means enter key, No means exit program."));
      msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      msgBox.setDefaultButton(QMessageBox::Yes);
      msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
      int ret = msgBox.exec();
      if (ret == QMessageBox::No)
        exit(7);
      ui->analyses->setEnabled(false);
      return false;
    }
  }
  else {
    qInfo() << "Product key failed to save. Code: " << hr;
    QMessageBox msgBox(this);
    msgBox.setText(tr("Product key failed to save."));
    msgBox.setInformativeText(tr("Would you like to enter a new key now?  Yes means enter key, No means exit program."));
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    int ret = msgBox.exec();
    if (ret == QMessageBox::No)
      exit(8);
    ui->analyses->setEnabled(false);
    return false;
  }
}

/**
 * @page cap member functions
 */
cap::cap(QMainWindow *p, QString product) :
  QMainWindow(p),
  ui(new Ui::cap) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << product.toStdString() << std::endl;
  quitting = false;
  owner = p;
  token = "";
  thisProduct = product;
  message = new QErrorMessage (this);
  message->setWindowFlags(message->windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint);
  statusWatcher = NULL;
  statusWatcherThread = NULL;
  priorSessionItemIndex = -1;
  qInfo() << "priorSessionItemIndex initialized as " << priorSessionItemIndex;
  userLoggedInToServer = false;
  loginTimer = NULL;
  clinicalJurisdiction = ""; // default to Research Edition, i.e., no clinical jurisdiction
  serverIdentity = "";
  loginView = NULL;
  key = "!@#$VERYsecRet)(";
  initializeWorkItemList();  
  
  for (int i=0; i < MAXSESSIONITEMS; i++) {
    sessionItems[i].select = NULL;
    sessionItems[i].survey = NULL;
    sessionItems[i].define = NULL;
    sessionItems[i].analyze = NULL;
    sessionItems[i].report = NULL;
    sessionItems[i].workItemProcess = NULL;
    sessionItems[i].gridLayout_2 = NULL;
    sessionItems[i].analysis = NULL;
  }

  // setup the menus which do not depend on what the product is
  fileMenu = new QMenu(tr("File"), this);
  QAction *saveAction = new QAction(tr("Save"), this);
  fileMenu->addAction(saveAction);
  connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

  QAction *saveAsAction = new QAction(tr("Save As"), this);
  fileMenu->addAction(saveAsAction);
  connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

  QAction *userLoginStatusAction = new QAction(tr("Log on to CAPgraph (or otherwise change logon status)"), this);
  fileMenu->addAction(userLoginStatusAction);
  connect(userLoginStatusAction, SIGNAL(triggered()), this, SLOT(userLoginStatusButton_clicked()));

  systemPreferencesObject = new systemPreferences(this);
  QAction *preferencesAction = new QAction(tr("Preferences"), this);
  fileMenu->addAction(preferencesAction);
  connect(preferencesAction, SIGNAL(triggered()), systemPreferencesObject, SLOT(displayPreferences()));

  QAction *quitAction = new QAction(tr("Quit"), this);
  fileMenu->addAction(quitAction);
  connect(quitAction, SIGNAL(triggered()), this, SLOT(on_cap_destroyed())); // this handles the user's intent to close as expressed by using menu action
  capClose.accept(); // this sets up the ability to trap when user hits X to close rather than using the menu action

  workItemMenu = new QMenu(tr("WorkItems"), this);
  workItem *dummyWorkItemToEstablishMenu = new workItem(0, 0, workItemMenu, true);

  // having done so, create the preliminary ui objects
  ui->setupUi(this);

  // statusbar
  userLoginStatusButton = new QPushButton(ui->statusbar);
  setUserLoginStatus("", "");
  connect(userLoginStatusButton, SIGNAL(clicked()), this, SLOT(userLoginStatusButton_clicked()));
  ui->statusbar->addPermanentWidget(userLoginStatusButton);

  resize(QDesktopWidget().availableGeometry(this).size());
  connect(ui->analyses, SIGNAL(tabCloseRequested(int)), this, SLOT(on_tabCloseRequested(int)));
  connect(ui->analyses, SIGNAL(currentChanged(int)), this, SLOT(on_currentChanged(int)));

  // set up trap which will be sued for special case of pulling back from report to analyze
  ui->analyses->installEventFilter(this);

  initialLoginAttempt = false;
  CAPgraphLoginSessionreply = nullptr;
  
  // connect for the login reply
  connect(&CAPgraphLoginSession, SIGNAL(finished(QNetworkReply*)), this, SLOT(parseSubscriptionRAD_86reply(QNetworkReply*)));
  // connect for the workitems reply
  connect(&CAPgraphWorkitems, SIGNAL(finished(QNetworkReply*)), this, SLOT(parseWorkitemsReply(QNetworkReply*)));

  // if no command line arguments, default to state as if work items should be listed for selection:
  numSessionItems = 0;
  openAnalysisCount = 0;
  listWorkItemsButton_clicked();

  // now setup the menus (some of which will depend on what product we are running)
  ui->menubar->addMenu(fileMenu);
  ui->menubar->addMenu(workItemMenu);

  seriesSurveyMenu = ui->menubar->addMenu(tr("Survey"));
  seriesSurvey *dummySeriesSurveyToEstablishMenu = new seriesSurvey(0, seriesSurveyMenu, true);
  dummySeriesSurveyToEstablishMenu->establishTools(thisProduct, 0, true);

  targetDefineMenu = ui->menubar->addMenu(tr("Define"));
  targetDefine *dummyTargetDefineToEstablishMenu = new targetDefine(0, targetDefineMenu, true);
  dummyTargetDefineToEstablishMenu->establishTools(thisProduct, 0, true);

  patientAnalyzeMenu = ui->menubar->addMenu(tr("Analyze"));
  patientAnalyze *dummyPatientAnalyzeToEstablishMenu = new patientAnalyze(0, patientAnalyzeMenu, true);
  dummyPatientAnalyzeToEstablishMenu->establishTools(thisProduct, 0, true);

  patientReportMenu = ui->menubar->addMenu(tr("Report"));
  patientReport *dummyPatientReportToEstablishMenu = new patientReport(0, patientReportMenu, true);

  // the following are not product dependent, but need to be added last since it is the right-most position
  helpMenu = ui->menubar->addMenu(tr("Help"));
  QAction *aboutAction = new QAction(tr("About"), this);
  helpMenu->addAction(aboutAction);
  connect(aboutAction, SIGNAL(triggered()), this, SLOT(displayAboutPage()));
  QAction *productKeyAction = new QAction(tr("Enter Product Key"), this);
  helpMenu->addAction(productKeyAction);
  connect(productKeyAction, SIGNAL(triggered()), this, SLOT(enterProductKey()));
  QAction *recheckLicenseAction = new QAction(tr("Re-check License"), this);
  helpMenu->addAction(recheckLicenseAction);
  connect(recheckLicenseAction, SIGNAL(triggered()), this, SLOT(recheckLicense()));
  /*QAction *userGuideAction = new QAction(tr("User Guide"), this);
  helpMenu->addAction(userGuideAction);
  connect(userGuideAction, SIGNAL(triggered()), this, SLOT(displayUserGuidePage()));*/

}

cap::~cap() 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  // determine whether there are changes that need to be saved before closing, provided this is a list from which an analysis hadn't been started.  (If it was, then 
  // start with a pass to see if any of the work items need saving
  bool atLeastOneWiHasItsFlagSet = false;
  for (int i=0; i < workItemList()->count(); i++) {
    workItemListEntry *wi = &(*workItemList())[i];
    if (wi->unsavedChanges == true)
      atLeastOneWiHasItsFlagSet = true;
  }
  // the user would already have been asked whether they want to save.  As such, this is understand as a "last chance" to save before application exit.)
  if (getUnsavedChangesFlag() && atLeastOneWiHasItsFlagSet) {
    if (systemPreferencesObject->getPromptBeforeSave() || (getListDirectory() == "")) {
      // either the user wants to be prompted or the list hasn't been named yet so they can't avoid being prompted
      QMessageBox msgBox(this);
      msgBox.setText(tr("There are unsaved changes."));
      msgBox.setInformativeText(tr("Do you want to save changes before exit? Yes means save before exit, No means abandon the changes."));
      msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      msgBox.setDefaultButton(QMessageBox::No);
      msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
      int ret = msgBox.exec();
      if (ret == QMessageBox::Yes) {
        save();
        setUnsavedChangesFlag(false);
      }
    }
    else { // the case when user doesn't want to be prompted but there is an open list
      save();
    }
  }

  // now start to take everything down
  if (loginView != NULL) {
    loginView->close();
    delete loginView;
  }
  delete systemPreferencesObject;
  for (int i=0; i<ui->analyses->count()-1; i++)
    on_tabCloseRequested(i); // need to close open tabs first
  ui->workItemProcess->removeWidget(ui->survey);
  delete ui->survey;
  ui->workItemProcess->removeWidget(ui->define);
  delete ui->define;
  ui->workItemProcess->removeWidget(ui->analyze);
  delete ui->analyze;
  ui->workItemProcess->removeWidget(ui->report);
  delete ui->report;
  ui->workItemProcess->removeWidget(ui->select);
  delete ui->select;  
  delete ui; // the rest
  /*
  if (statusWatcherThread) {
    statusWatcherThread->terminate();
    delete statusWatcherThread;
  }
  */
  deleteTemporaryWorkitemFolder();
}

void cap::initializeWorkItemList()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  setListDirectory(cap::getCAPExamDataDir()+WORKING_STORAGE_FOLDER); // starting default
  setUnsavedChangesFlag(false);
  setOpenAnalysisCount(0);
  workItemList()->clear();
  getListProvenance()->initializeWorkItemListProvenance();
}

QString cap::getUserName() {
  //ebLog eblog(Q_FUNC_INFO); eblog << serverIdentity.toStdString() << std::endl;
  if (serverIdentity != "")
    return serverIdentity;
  else
    return getLocalUserName();
}

std::string cap::encode(std::string text) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  std::string encoded_string;
  std::string ret;
  unsigned char key_c;
  unsigned char encoded_c;
  Base64 b64;

  for (int i=0; i<text.length(); i++) {
    key_c = key[i % key.length()];
    encoded_c = text[i] + key_c % 256;
    encoded_string += encoded_c;
  }
  ret = b64.base64_encode(encoded_string);
  return ret;
}

void cap::forceLogoutAfterInactivity()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  userLoginStatusButton_clicked();
  delete loginTimer;
  loginTimer = NULL;
  //message->showMessage(tr("Logged out of CAPgraph due to a period of inactivity. Please use button at lower right to begin a new session."));
}

void cap::setLoginTimeout()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (loginTimer != NULL) {
    disconnect(loginTimer,SIGNAL(timeout()),this,SLOT(forceLogoutAfterInactivity()));
    loginTimer->stop();
    delete loginTimer;
  }
  loginTimer = new QTimer(this);
  loginTimer->setSingleShot(true);
  connect(loginTimer,SIGNAL(timeout()),this,SLOT(forceLogoutAfterInactivity()));
  loginTimer->start(3*60*60*1000); // 3 hours (server side allows 4, but make sure that client logs out before it does)
}

void cap::setUserLoginStatus(QString identity, QString institution)
{
   ebLog eblog(Q_FUNC_INFO); eblog << identity.toStdString() << std::endl;
   QString userLoginStatus = tr("Identity used for provenance: ");
   serverIdentity = identity;
  if (serverIdentity != "") {
    userLoggedInToServer = true;
    userLoginStatus.append(serverIdentity);
    userLoginStatus.append("/"+institution);
    userLoginStatusButton->setStyleSheet("QPushButton {background-color: #449659; color: white; padding: 6px; margin: 2px; border: none;}"
      "QPushButton:pressed {background-color: #444444; color: white; padding: 6px; border-radius: 5px; margin: 2px; border: none;}");
    userLoginStatus.append(tr(" (logged in to CAPgraph, press to logout)"));
    // also need to force a logout if too long a period of inactivity
    setLoginTimeout();
  }
  else {
    userLoggedInToServer = false;
    userLoginStatus.append(getLocalUserName());
    userLoginStatusButton->setStyleSheet("QPushButton {background-color: #7f7d39; color: white; padding: 6px; margin: 2px; border: none;}"
      "QPushButton:pressed {background-color: #444444; color: white; padding: 6px; border-radius: 5px; margin: 2px; border: none;}");
    userLoginStatus.append(tr(" (local ID, press to log in to CAPgraph)"));

    for (int i=0; i < workItemList()->size(); i++) {
      workItemListEntry *wi = &(*(workItemList()))[i];
      wi->dataSentToServer = false;
    }
  }
  userLoginStatusButton->setText(userLoginStatus);
}

void cap::displayUserGuidePage()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  QDesktopServices::openUrl(QUrl(":/cap/vascuCAP User Guide - RE, 8-Mar-2017.pdf"));
}

void cap::displayAboutPage()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QGridLayout *gridLayout;
  QDialogButtonBox *buttonBox;
  QTextEdit *licenseText;
  QLabel *buildLabel;
  QLabel *addressLabel;
  QTextEdit *descriptionText;
  QDialog Dialog;

  // create dialog
  Dialog.resize(700, 700);
  gridLayout = new QGridLayout(&Dialog);
  gridLayout->setObjectName(QStringLiteral("gridLayout"));
  buttonBox = new QDialogButtonBox(&Dialog);
  buttonBox->setObjectName(QStringLiteral("buttonBox"));
  buttonBox->setOrientation(Qt::Horizontal);
  //buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
  gridLayout->addWidget(buttonBox, 3, 0, 1, 1);

  // create label for build version string
  buildLabel = new QLabel(QString("Build " CAP_VERSION " " __DATE__ " " __TIME__), &Dialog);
  gridLayout->addWidget(buildLabel, 0, 0, 1, 1);

  // create label for Company Address string
  addressLabel = new QLabel(QString(ELUCID_COMPANY_ADDRESS), &Dialog);
  gridLayout->addWidget(addressLabel, 1, 0, 1, 1);

  // create text edit for EULA
  licenseText = new QTextEdit(&Dialog);
  licenseText->setObjectName(QStringLiteral("licenseText"));
  licenseText->setReadOnly(true);
  licenseText->setTextInteractionFlags(Qt::NoTextInteraction);
  gridLayout->addWidget(licenseText, 2, 0, 1, 1);
  QString eulaHtml;
  for (int i=0; i < CAP_license_html_len; ++i)
    eulaHtml.append(CAP_license_html[i]);
  licenseText->insertHtml(eulaHtml);
  licenseText->setReadOnly(true);

  // create text cursor
  QTextCursor licenseCursor(licenseText->textCursor());
  licenseCursor.movePosition(QTextCursor::Start);
  licenseText->setTextCursor(licenseCursor);

  Dialog.exec();
}

void cap::setCAPgraphConfig()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QFile file(cap::getCAPExamDataDir()+CONFIGURATION_FOLDER+CAPGRAPH_CONFIG_FILE);
  QByteArray val;
  QJsonDocument config_json;
  QJsonObject config;

  // qInfo() << "reading" << QFileInfo(file).absoluteFilePath();
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    val = file.readAll();
    file.close();
    config_json = QJsonDocument::fromJson(val);
    config = config_json.object();
  }
  else {
    qInfo() << "No CAPgraph config file found";
  }

  CAPgraph_host = getCAPgraphConfig(config, "CAPgraph.host");
}

QString cap::getDefaultCAPgraphConfig(QString key)
{
  ebLog eblog(Q_FUNC_INFO); eblog << key.toStdString() << std::endl;
  QString ret = QString();
  QStringList keys = key.split('.');
 
  if (keys[0] == "CAPgraph")
    if (keys[1] == "host")
      return DEFAULT_CAPGRAPH_HOST;
    else
      return ret;
  else
      return ret;
}

// right now this only supports a 2 level look up
// e.g. host.port
// It can be expaned if needed
QString cap::getCAPgraphConfig(QJsonObject config, QString key)
{
  ebLog eblog(Q_FUNC_INFO); eblog << key.toStdString() << std::endl;

  QString ret = QString();
  if (key.isNull() || key.isEmpty())
    return ret;

  QStringList keys = key.split('.');
  if (keys.length() != 2)
    return ret;

  if (config.isEmpty()) {
    qInfo() << "CAPgraph config file is empty";
    return getDefaultCAPgraphConfig(key);
  }

  QJsonObject obj = config.value(keys[0]).toObject();
  QJsonDocument config_json(obj);
  qInfo() << "object read from CAPgraph config file:" << config_json.toJson(QJsonDocument::Compact);
  if (obj.isEmpty()) {
    qInfo() << "object is empty";
    return getDefaultCAPgraphConfig(key);
  }

  if (obj.contains(keys[1])) 
    return obj.value(keys[1]).toString();
  else {
    qWarning() << "object " << QString(keys[1]) << " not found in config object";
    return getDefaultCAPgraphConfig(key);
  }
}

void cap::setRegulatoryLabel()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  QString titleText = "Computer-Aided Phenotyping ";
  
  if (thisProduct == "vascuCAP")
    titleText.append("for Vasculopathy");
  else if (thisProduct == "heartCAP")
    titleText.append("for Cardiology");
  else if (thisProduct == "kidneyCAP")
    titleText.append("for Renal Disease");
  else if (thisProduct == "liverCAP")
    titleText.append("for Hepatic Disease");
  else if (thisProduct == "lungCAP")
    titleText.append("for Pulmonary Disease");

  QString activationMessage = tr("Activating functionality ");
  if (clinicalJurisdiction == "") {
    titleText.append(" -- INVESTIGATIONAL DEVICE NOT FOR CLINICAL USE");
    activationMessage.append(tr("for Research Edition"));
  }
  else {
    titleText.append(" (" + clinicalJurisdiction + ")");
    activationMessage.append(tr("consistent with ") + clinicalJurisdiction + tr(" regulations"));
  }
  this->setWindowTitle(titleText);    
  if (clinicalJurisdiction != "") {
    QMessageBox *noticeBox = new QMessageBox();
    noticeBox->setText(activationMessage+"   ");
    noticeBox->setWindowFlags(noticeBox->windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint);
    noticeBox->setModal(true);
    noticeBox->show();
    int waitFactor = 1;
    if (ebSystem::NoWait())
      waitFactor = 0;
    QTimer::singleShot(3000*waitFactor, noticeBox, SLOT(close()));
  }
  this->repaint(); // ensure it is shown
  qApp->processEvents();
}

void cap::setLogs(QString outLog, QString errLog)
{
  QString logs = outLog; logs.append("|"); logs.append(errLog);
  ebLog eblog(Q_FUNC_INFO); eblog << logs.toStdString() << std::endl;
  /*
  statusWatcher = new StatusWatcher(this, outLog, errLog);
  statusWatcherThread = new QThread(this);
  statusWatcher->moveToThread(statusWatcherThread);
  statusWatcherThread->start();
  connect(statusWatcher, SIGNAL(statusMessage(QString)), this, SLOT(catchStatusMessage(QString)));
  connect(statusWatcher, SIGNAL(toggleNotifier(bool, QSocketNotifier*)), this, SLOT(toggleNotifier(bool, QSocketNotifier*)));
  */
}

void cap::toggleNotifier(bool state, QSocketNotifier *notifier)
{
  notifier->setEnabled(state);
}

void cap::catchStatusMessage(QString msg)
{
        ui->statusbar->showMessage(msg);
  this->repaint(); // ensure progress is shown
  qApp->processEvents();
}

void cap::displayListWorkItemsButton() 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  this->listWorkItemsButton = new QToolButton();
#ifndef _WIN32
  QFont font = this->listWorkItemsButton->font(); // the font of a tool button is smaller than we need, so next 3 lines address that
  font.setPointSize(13);
  this->listWorkItemsButton->setFont(font);
#endif
  this->listWorkItemsButton->setText(tr("Work Item List"));
  this->listWorkItemsButton->setAutoRaise(true);
  connect(this->listWorkItemsButton, SIGNAL(clicked()), this, SLOT(listWorkItemsButton_clicked()));
  connect(workItemMenu, SIGNAL(triggered(QAction *)), this, SLOT(listWorkItemsButton_clicked()));
  workItemMenu->setEnabled(true);

  /*begin signal block (to avoid superfluous "current changed" calls)...*/
    const bool isBlocked1 = ui->analyses->blockSignals(true);
    ui->analyses->addTab(new QLabel(tr("You can select a work item for analysis by pressing <b>\"Work Item List\" tab</b>")), QString());
    ui->analyses->setTabEnabled(ui->analyses->count() - 1, true);
    ui->analyses->tabBar()->setTabButton(ui->analyses->count() - 1, QTabBar::RightSide, this->listWorkItemsButton);
    ui->analyses->blockSignals(isBlocked1);
  /*...end signal block*/
}

void cap::nullOutLoginView()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  loginView = NULL;
}

void cap::userLoginStatusButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QString urlStr;
  QString CAPgraph_host = getCAPgraphHost();
  if (!userLoggedInToServer) {
    ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; 
    if (loginView == NULL) {
      loginView = new QWebView(owner);
      connect(loginView, SIGNAL(destroyed()), this, SLOT(nullOutLoginView()));
      loginView->setObjectName(QStringLiteral("webView"));
      urlStr = CAPgraph_host + "/vascuCAP_login/";
      urlStr.append(QString::fromStdString(ebSystem::GetSystemDiskUUID())).append("/").append(getLocalUserName()).append("/").append(clinicalJurisdiction).append("/");
      urlStr.append("?token=" + token);
      loginView->setUrl(QUrl(urlStr));
      connect(loginView, SIGNAL(urlChanged(QUrl)), this, SLOT(loginCompleted(QUrl)));
    }
    loginView->show();
    loginView->raise();
    loginView->activateWindow();
  }
  else {
    urlStr = CAPgraph_host + "/vascuCAP_logout/";
    urlStr.append(QString::fromStdString(ebSystem::GetSystemDiskUUID())).append("/").append(getLocalUserName()).append("/");
    qInfo() << "........logging out using url" << urlStr;
    QUrl logoutUrl(urlStr);
    QNetworkRequest request(logoutUrl);
    CAPgraphLoginSessionreply = CAPgraphLoginSession.get(request);
    setUserLoginStatus("", "");
  }
}

void cap::loginCompleted(QUrl url)
{
  ebLog eblog(Q_FUNC_INFO); eblog << url.toString().toStdString() << std::endl;
  if (! url.toString().startsWith(CAPgraph_host + "/accounts/login/?next=/vascuCAP_login/")) {
    connectToCAPgraph();
  }
}

void cap::connectToCAPgraph()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  QString CAPgraph_host = cap::getCAPgraphHost();
  QString urlStr = CAPgraph_host + "/check_authorization/";
  urlStr.append(QString::fromStdString(ebSystem::GetSystemDiskUUID())).append("/").append(getLocalUserName()).append("/").append(clinicalJurisdiction).append("/");
  urlStr.append("?token=" + token);
  QUrl transferUrl(urlStr);
  QNetworkRequest request(transferUrl);
  CAPgraphLoginSessionreply = CAPgraphLoginSession.get(request);
  initialLoginAttempt = true;
}

void cap::recheckLicense()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  // this is the means to ask the license server directly (rather than the usual which only asks the license server after an interval)
  // the most specific purpose of this is if a new feature has been added to the license, to obtain it, which otherwise wouldn't appear 
  // until sometime later due to the itnerval.
    /* (0) is what causes the direct check*/
  ui->analyses->setEnabled(checkLicense(0));
  while (!ui->analyses->isEnabled()) {
    enterProductKey();
  }
}

void cap::parseSubscriptionRAD_86reply(QNetworkReply *reply)  
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  // first, see if we are activated already or bring up the prompt for code
  /* (15) is what allows the check to not require internet connection every time*/
  ui->analyses->setEnabled(checkLicense(15));
  while (!ui->analyses->isEnabled()) {
    enterProductKey();
  }

  // now that we know we're activated
  CAPgraphLoginSessionreply->deleteLater();
  if (reply->error() == QNetworkReply::NoError) {
    QByteArray byteArray = reply->readAll();
      if (byteArray.size() > 0) {
      QJsonParseError err;
      QJsonDocument doc = QJsonDocument::fromJson(byteArray, &err);
      //qInfo() << "byteArray=" << byteArray << ", err=" << err.errorString() << " and doc is" << doc;
      if (doc.isObject()) {
        QString institution = "";
        QJsonObject obj = doc.object();
        QJsonObject::iterator itr = obj.find("institution");
        if (itr != obj.end()) {
          institution = obj["institution"].toString(); 
        }
        itr = obj.find("token");
        if (itr != obj.end()) {
          token = obj["token"].toString();
        }
        itr = obj.find("identity");
        if (itr != obj.end()) {
          QString identity = obj["identity"].toString(); 
          setUserLoginStatus(identity, institution);
          if (loginView != NULL) {
            loginView->hide();
            delete loginView;
            loginView = NULL;
          }
          QString urlStr = CAPgraph_host + "/wientries/";
          urlStr.append("?user=" + getUserName() + "&token=" + token);
          QUrl reqUrl(urlStr);
          QNetworkRequest request( reqUrl );
          request.setOriginatingObject( NULL );
          CAPgraphWorkitemsreply = CAPgraphWorkitems.get(request);
          return;
        } 
        else {
          qWarning() << "Error: Network transfer failed, response did not contain an identity, skipping";
          userLoginStatusButton_clicked();
        }
      } // if response cannot be parsed as a json doc
      else
        QMessageBox::warning(this, tr("Error parsing server return"), QString("Code: %1").arg(err.errorString()));
    } // skip empty replies
    else
      qInfo() << "Reply with zero bytes received, skipping";
  } // if network error
  else {
    if (initialLoginAttempt) {
      // log an informational string to log, but this isn't an error, and then raise the login pane if they want to do it now if they like
      qInfo() << "user was not logged in yet, giving them chance to do so now";
      userLoginStatusButton_clicked();
      initialLoginAttempt = false;
    }
    else {
      // give a more firm message, as their attempted login didn't work
      QMessageBox::warning(this, tr("Download error"), QString(tr("Login attempt failed: %1")).arg(reply->errorString()));
      userLoginStatusButton_clicked();
    }
  }
}

void cap::processWebError(QNetworkReply *errorCode)
{
    qInfo() << "Network error: " << errorCode;
    qInfo() << "Error details: " << errorCode->errorString();
    qInfo() << QString::fromStdString(errorCode->readAll().toStdString());
}

void writeFile(QNetworkReply *reply, QFile &newfile)
{
    qint64 rsz, sz = 8 * 4096;
    char *data = new char[8 * 4096];
    while (0 < sz && 0 < (rsz = reply->read(data, sz))) {
        qint64 ptr = 0, wsz;
        while (0 < rsz) {
            if (0 > (wsz = newfile.write(data + ptr, rsz))) {
                sz = 0;
                break;
            }
            rsz -= wsz;
            ptr += rsz;
        }
    }
    delete[](data);
}

void cap::parseWorkitemDataReply(QNetworkReply *reply)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  wi_download_data *cbdata = ((wi_download_data *)reply->request().originatingObject());
  cbdata->getReply()->deleteLater();
  // workItemListEntry *wi = cbdata->entry();

  if (reply->error() != QNetworkReply::NoError) {
    eblog << "Download error. parseWorkitemDataReply errorString=" << reply->errorString().toStdString() << std::endl;
    eblog << reply->readAll().toStdString() << std::endl;
    delete cbdata;
    return;
  }

    qInfo() << "webReply downloadState: " << cbdata->getStateStr() << " " << cbdata->getIdentifier();

    if (WI_DL_START == cbdata->getState()) {
        QByteArray byteArray = reply->readAll();
        QJsonParseError err;
        QJsonDocument doc = QJsonDocument::fromJson(byteArray, &err);

        if (!doc.isObject()) {
          QMessageBox::warning(this, tr("Error parsing server response"), QString("parseWorkitemDataReply errorString: %1").arg(err.errorString()));
          return;
        }

        QJsonObject root = doc.object();

        for (const auto& sect : root.keys()) {
            if (sect == "dicom") {
                QJsonObject obj = root["dicom"].toObject();
                for ( const auto& key : obj.keys() ) {
                    qInfo() << "webReply key: " << key << " value: " << obj[key].toString();
                    wi_download_data *ndata = new wi_download_data(cbdata);
                    ndata->setState(WI_DL_DICOM);
                    ndata->setFilename( obj[key].toString() );
                    // queue up request
                    ndata->setUrl( cbdata->getUrl() + obj[key].toString() 
                        + "?identifier=" + ndata->getIdentifier()
                        + "&token=" + token );
                    QNetworkRequest request( QUrl( ndata->getUrl() ) );
                    request.setOriginatingObject( ndata );
                    QNetworkReply *reply = CAPgraphWorkitems.get(request);
                    ndata->setReply( reply );
                }
            }
            if (sect == "target") {
                QJsonObject target = root["target"].toObject();
                wientry *entry = cbdata->getWIEntry();
                for ( const auto& key : target.keys() ) {
                    QJsonObject fileinfo = target[key].toObject();
                    int fsize = fileinfo["size"].toInt();
                    double fdate = fileinfo["date"].toDouble();
                    qInfo() << "webReply target: " << key << " value: " << QString::number(fdate, 'f') << " " << QString::number(fsize);

                    QString ppath = entry->getWorkItemFolder() + "/" + key.split(":").join("/");
                    QFile fsfile( ppath );
                    if (fsize != fsfile.size() || fdate > QT_DATETIME_SECS( QT_FILETIME(fsfile) ) ) {
                        qInfo() << "size " << QString::number(fsize) << " " << QString::number(fsfile.size()) 
                                << " " << fsfile.exists() 
                                << " fdate " << QString::number(fdate, 'f') 
                                << " " << QString::number( QT_DATETIME_SECS( QT_FILETIME(fsfile) ), 'f' )
                                << " " << ppath ;
                        wi_download_data *ndata = new wi_download_data(cbdata);
                        ndata->setState(WI_DL_FILE);
                        ndata->setFilename( ppath );
                        ndata->setTimestamp( fdate );
                        // queue up request
                        ndata->setUrl( cbdata->getUrl() + key.toLatin1()
                            + "?identifier=" + ndata->getIdentifier()
                            + "&token=" + token );
                        QNetworkRequest request( QUrl( ndata->getUrl() ) );
                        request.setOriginatingObject( ndata );
                        QNetworkReply *reply = CAPgraphWorkitems.get(request);
                        ndata->setReply( reply );
                    }
                }
            }
        }
        delete cbdata;
        return;
    }

    if (WI_DL_DICOM == cbdata->getState()) {
        // store file
        QDir dirpath(cap::getImagesRetrieveDir() + "/" + cbdata->getIdentifier());
        if (!dirpath.exists())
            QDir().mkdir(dirpath.path());
        QString path = dirpath.filePath( cbdata->getFilename() ); 
        QFile newfile( path );
        if (!newfile.open(QIODevice::WriteOnly)) {
            qInfo() << "Error: cannot open output for write " << newfile.fileName();
        } else {
            writeFile(reply, newfile);
            newfile.close();
            if (1 == getImageDatabase()->ImportImages(path, false, false, false, false, false)) {
                newfile.remove();
            }
            QDir().rmdir( dirpath.path() );             // remove dir if empty
        }
    }
    if (WI_DL_FILE == cbdata->getState()) {
        QFile fsfile( cbdata->getFilename() );
        if (!fsfile.open(QIODevice::WriteOnly)) {
            qInfo() << "cannot open output for write " << fsfile.fileName();
        } else {
            writeFile(reply, fsfile);
            setFileTime(fsfile, cbdata->getTimestamp());
            qInfo() << "wrote updated workitem file " << fsfile.fileName();
            fsfile.close();
        }
    }
    delete cbdata;
}

wilist * readList(cap *p, QString wipath, KeyListMap &kmap)
{
  wilist *wilistitem = getWIlist(p, wipath);
  if (!wilistitem->trylock()) {
    qInfo() << "ui has workitem queue locked: " << wipath;
    return NULL;
    // this reply func unexpectedly looks to be called from the main ui thread, which I suppose makes
    // locking the element unecessary.   however, should it become possible to perform this task in 
    // the back-ground, the lock will be necessary.
  }
  wilistitem->readList(kmap);

  return wilistitem;
}

wilist * findMatch(wilistentry *list, wientry *entry)
{
    QString current = entry->getWorkItemFolder();
    while (NULL != list) {
        wientry *lentry = list->getValue()->get(entry->getWorkItemID());
        if (current == lentry->getWorkItemFolder()) 
            return list->getValue();
        list = list->getNext();
    }
    return NULL;
}

void cap::parseWorkitemsReply(QNetworkReply *reply)  
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  reply->deleteLater();

  wi_download_data *cbdata = ((wi_download_data *)reply->request().originatingObject());
  if (NULL != cbdata) {
    parseWorkitemDataReply(reply);
    return;
  }

  if (reply->error() != QNetworkReply::NoError) {
    eblog << "Download error cannot get list of workitems" << reply->errorString().toStdString() << std::endl;
    eblog << reply->readAll().toStdString() << std::endl;
    return;
  }

  QByteArray byteArray = reply->readAll();
  QJsonParseError err;
  QJsonDocument doc = QJsonDocument::fromJson(byteArray, &err);

  if (!doc.isObject()) {
    eblog << "Download error parsing server return: " << err.errorString().toStdString() << std::endl;
    return;
  }
  QJsonObject root = doc.object();

  qInfo() << "workItemsReply size: " << root.size();

  KeyListMap keymap = QMap<QString, wilistentry *>();

  QString winame = "/wilist_" + getUserName() + "_queue" ;
  QString wipath = getCAPExamDataDir() + "/" + WORKING_STORAGE_FOLDER + winame + winame + ".json";
  QList<wilist *> wilists;

  wilist *wilistorder = readList(this, wipath, keymap);

  if (NULL != wilistorder)
    wilists.append(wilistorder);

  QString datapath = getCAPExamDataDir();
  QDirIterator it(datapath, QStringList() << "wilist*.json", QDir::Files, QDirIterator::Subdirectories);

  while (it.hasNext()) {
    QString fullpath = it.next();
    QStringList names = fullpath.split("/");

    if (names.last().contains(QRegExp("[a-zA-Z0-9]_20[0-9]{6}-[0-9]{4}"))) 
        continue;

    wilist *rlist = readList(this, fullpath, keymap);
    if (NULL != rlist)
        wilists.append(rlist);
  }

  //  queue state       action
  //  AB (abandoned)    existingOrder -> delete
  //  DN (done)         existingOrder -> delete
  //  IN (queued)       existingOrder -> delete |  ignore
  //  PQ (passed qa)    new -> append      |  ignore
  //  TD (targets def)  new -> append      |  ignore
  //  EA (report ready) new -> append      |  ignore
  //  RG (reviewed)     new -> append      |  ignore

  bool newItem = false;
  for ( const auto& key : root.keys() ) {
    // if ("TD" == key || "EA" == key || "RG" == key) continue;
    QJsonObject queue = root[key].toObject();
    bool existDeleteState = ("AB" == key || "DN" == key || "IN" == key);

    for (const auto& wikey : queue.keys() ) {
        qInfo() << "processing workitem key " << wikey;
        wilistentry *keyentry = getListEntry(keymap, wikey);
        bool existing = NULL != keyentry;
        bool existingOrder = (NULL != wilistorder) && wilistorder->wiExists(wikey);

        if (existDeleteState) {
            if (existingOrder)
                wilistorder->remove(wikey);
            continue;
        }

        wientry *incoming = wilistorder->parse( queue[wikey].toString() );

        QString urlStr = CAPgraph_host + "/downloads/?user=" + getUserName()
            + "&identifier=" + wikey + "&token=" + token;
        QUrl turl(urlStr);

        wi_download_data *cbdata = NULL;

        if (existingOrder) {
            wientry *entry = wilistorder->get(incoming->getWorkItemID());
            cbdata = new wi_download_data();
            cbdata->setWIEntry(incoming);
            wilistorder->update(entry, incoming);
        } else if (!existing && incoming->getWorkItemFolder().isEmpty()) {
            if (NULL == wilistorder) {
                qInfo() << "WARNING: cannot update new workitem into work orders: " << wikey;
                continue;
            }
            qInfo() << "new workitem key " << wikey;
            wilistorder->add(queue[wikey].toString());
            cbdata = new wi_download_data();
            cbdata->setWIEntry(incoming);
            incoming->setWorkItemFolder( wilistorder->getWIDir()->filePath(wikey) );
        } else if (!incoming->getWorkItemFolder().isEmpty()) {  // check for differences and update
            wilist *existingList = findMatch(keyentry, incoming);
            if (NULL == existingList) {
                qInfo() << "ERROR: cannot find matching workitem: " << wikey << "path: " << incoming->getWorkItemFolder();
                delete incoming;
                continue;
            }
            wientry *entry = existingList->get(incoming->getWorkItemID());

            cbdata = new wi_download_data();
            cbdata->setWIEntry(incoming);
            
            existingList->update(entry, incoming);
        } else {
            qInfo() << "skipping non-matching workitem: " << wikey << "path: " << incoming->getWorkItemFolder();
        }
        if (NULL != cbdata) {
            cbdata->setUrl( getCAPgraphHost() + "/download/" );
            cbdata->setIdentifier( wikey );

            QNetworkRequest request( turl );
            request.setOriginatingObject( cbdata );
            QNetworkReply *reply = CAPgraphWorkitems.get(request);
            cbdata->setReply( reply );
        } else {
            delete incoming;
        }
     }
  }

  foreach (wilist *li, wilists) {
      if (li->isUpdated()) {
        li->writeList(getUserName(), getISODateTime());
      }
      li->clear();
      li->unlock();
  }
  wilists.clear();
}

void cap::listWorkItemsButton_clicked() 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  qInfo("numSessionItems=%d, priorSessionItemIndex=%d", numSessionItems, priorSessionItemIndex);
  if (numSessionItems >= MAXSESSIONITEMS) {
    message->showMessage(tr("Warning: Cannot process another item in this session"));
    return;
  }
  for (int i=0; i < numSessionItems; i++) {
    qInfo() << "  for sessionitem " << i;
    qInfo() << "    select:" << sessionItems[i].select;
    qInfo() << "    survey:" << sessionItems[i].survey;
    qInfo() << "    define:" << sessionItems[i].define;
    qInfo() << "    analyze:" << sessionItems[i].analyze;
    qInfo() << "    report:" << sessionItems[i].report;
    qInfo() << "    workItemProcess:" << sessionItems[i].workItemProcess;
    qInfo() << "    gridLayout2:" << sessionItems[i].gridLayout_2;
    qInfo() << "    analysis:" << sessionItems[i].analysis;
  }

  int indexOfNewItem = numSessionItems++;
  
  /*begin signal block (to avoid superfluous "current changed" calls)...*/const bool isBlocked2 = ui->analyses->blockSignals(true);
    ui->analyses->removeTab(ui->analyses->count() - 1); //this removes the tool button
  /*...end signal block*/ui->analyses->blockSignals(isBlocked2);
  
  // modelled after ui_cap.h, but modified to allow multiples:
  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  sessionItems[indexOfNewItem].analysis = new QWidget(ui->analyses);
  sessionItems[indexOfNewItem].analysis->setObjectName(QStringLiteral("analysis"));
  sizePolicy.setHeightForWidth(sessionItems[indexOfNewItem].analysis->sizePolicy().hasHeightForWidth());
  sessionItems[indexOfNewItem].analysis->setSizePolicy(sizePolicy);
  sessionItems[indexOfNewItem].gridLayout_2 = new QGridLayout(sessionItems[indexOfNewItem].analysis);
  sessionItems[indexOfNewItem].gridLayout_2->setSpacing(0);
  sessionItems[indexOfNewItem].gridLayout_2->setContentsMargins(11, 11, 11, 11);
  sessionItems[indexOfNewItem].gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
  sessionItems[indexOfNewItem].gridLayout_2->setContentsMargins(0, 0, 0, 0);

  sessionItems[indexOfNewItem].workItemProcess = new QStackedWidget(sessionItems[indexOfNewItem].analysis);
  sessionItems[indexOfNewItem].workItemProcess->setObjectName(QStringLiteral("workItemProcess"));

  sessionItems[indexOfNewItem].select = new workItem(this, indexOfNewItem, workItemMenu, false);
  sessionItems[indexOfNewItem].select->setObjectName(QStringLiteral("select"));
  connect(this, SIGNAL(updateWorkItemDisplaysGivenSave()), sessionItems[indexOfNewItem].select, SLOT(updateWorkItemDisplaysGivenSave()));
  sessionItems[indexOfNewItem].workItemProcess->addWidget(sessionItems[indexOfNewItem].select);

  sessionItems[indexOfNewItem].gridLayout_2->addWidget(sessionItems[indexOfNewItem].workItemProcess, 0, 0, 1, 1);

  // disconnect prior menu actions (if there were any) and connect new one
  if (numSessionItems > 1) {
    // out with the prior
    if (priorSessionItemIndex >= 0) {
      if (sessionItems[priorSessionItemIndex].select != NULL) {
        sessionItems[priorSessionItemIndex].select->disconnectMenuActions();
        if (sessionItems[priorSessionItemIndex].select->selectedWorkItemValid) {
          sessionItems[priorSessionItemIndex].survey->disconnectMenuActions();
          sessionItems[priorSessionItemIndex].define->disconnectMenuActions();
          sessionItems[priorSessionItemIndex].analyze->disconnectMenuActions();
          sessionItems[priorSessionItemIndex].report->disconnectMenuActions();
          sessionItems[priorSessionItemIndex].analyze->dismissCompositionControls();
        }
      }
    }
    disconnect(workItemMenu, SIGNAL(triggered(QAction *)), this, SLOT(listWorkItemsButton_clicked()));

    // in with the new
    sessionItems[indexOfNewItem].select->connectMenuActions();
  }

  // and update the prior index for next time
  qInfo() << "priorSessionItemIndex was " << priorSessionItemIndex << " and will now be " << indexOfNewItem;
  priorSessionItemIndex = indexOfNewItem;

  /*begin signal block (to avoid superfluous "current changed" calls)...*/const bool isBlocked3 = ui->analyses->blockSignals(true);
    ui->analyses->addTab(sessionItems[indexOfNewItem].analysis, QString(tr("Work Item List")));
    ui->retranslateUi(this);
  
    // initialize the processing step:
    sessionItems[indexOfNewItem].workItemProcess->setCurrentIndex(WORKITEM);
    sessionItems[indexOfNewItem].select->setOpenAnalysisCount(openAnalysisCount); 

    // make this one current, and free the memory of the button which got us here:
    ui->analyses->setCurrentWidget(sessionItems[indexOfNewItem].analysis);
  /*...end signal block*/ui->analyses->blockSignals(isBlocked3);
}

void cap::logUpdate(QString step, QString ID, stageParameters *stageParams, int sessionItemIndex) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  workItemListEntry* wi = Q_NULLPTR;
  for (int i=0; i < workItemList()->size(); i++) {
    if (((step.contains("seriesSurvey")) && (workItemList()->at(i).getSurvey() == sessionItems[sessionItemIndex].survey))
      || ((step.contains("targetDefine")) && (workItemList()->at(i).getDefine() == sessionItems[sessionItemIndex].define))
      || ((step.contains("patientAnalyze")) && (workItemList()->at(i).getAnalyze() == sessionItems[sessionItemIndex].analyze))
      || ((step.contains("patientReport")) && (workItemList()->at(i).getReport() == sessionItems[sessionItemIndex].report))) {
      wi = &((*workItemList())[i]);
      break;
    }
  }
  if (wi != Q_NULLPTR)
    wi->logWorkItemListEntryUpdate(step, ID, stageParams, sessionItems[sessionItemIndex].select);
}

void cap::selectWorkItemForAnalysis(workItemListEntry *wi, int sessionItemIndex) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // search to see if this one is already open
  for (int i=0; i<sessionItemIndex; i++) {
    if (sessionItems[i].select != NULL) {
      if (sessionItems[i].select->selectedWorkItemValid) {
        if (sessionItems[i].select->selectedWorkItemListEntryPtr == wi) {
          // its already open - position on that tab and return with an indication that don't need to open a new analysis
          /*begin signal block (to avoid superfluous "current changed" calls)...*/const bool isBlocked5 = ui->analyses->blockSignals(true);
            ui->analyses->setCurrentIndex(ui->analyses->indexOf(sessionItems[i].analysis));
          /*...end signal block*/ui->analyses->blockSignals(isBlocked5);
          return;
        }
      }
    }
  }
  sessionItems[sessionItemIndex].select->selectedWorkItemListEntryPtr = wi;
  sessionItems[sessionItemIndex].select->selectedWorkItemValid = true;

  // if reach here, must need to convert the current tab to be an analysis of the selected wi
  // this is a complex sequence and cannot be interupted by further gui requests until it is done
  this->setEnabled(false);
  ui->centralWidget->setEnabled(false);
  wi->pipeline = ebiVesselPipeline::New();
  wi->pipeline->GetMultiImageReader()->SetUseThread(true);
  wi->pushWorkItemParametersToPipeline(clinicalJurisdiction);
  wi->setSessionItemIndex(sessionItemIndex);
  QString entryLabel;
  switch (systemPreferencesObject->getLabelTabsWith()) {
    case WORKITEMID: entryLabel = wi->getWorkItemID(); break;
    case PATIENTID: entryLabel = wi->getIndividualID(); break;
    case PATIENTNAME: entryLabel = wi->getPatientName(); break;
    case SUBJECTID: entryLabel = wi->getSubjectID(); break;
    case ALSOKNOWNAS: entryLabel = wi->getAlsoKnownAs(); break;
  }
  if (entryLabel.trimmed().isEmpty())
    entryLabel = "EMPTY"; // avoid tabs without space for the label, which in essence makes them unselectable unless we make a blank label
  
  // now we have to look whether this individual has other timepoints and/or acqreps, so that we can add designators to the tab label
  for (int i=0; i < workItemList()->size(); i++) {
    if ((workItemList()->at(i).getIndividualID() == wi->getIndividualID()) && (workItemList()->at(i).getWorkItemID() != wi->getWorkItemID())) {
      // since we found this individual again, we have timepoints
      entryLabel.append(" (");
      entryLabel.append(QString::number(wi->getTimepoint()));
      if (wi->getAcqrep() != 0) {
        entryLabel.append(",");
        entryLabel.append(QString::number(wi->getAcqrep()));
      }
      entryLabel.append(")");
      break;
    }
  }
  wi->setTabLabel(entryLabel);

  // construct and record the classes
  // NOTE: CONSTRUCTION ORDER MUST BE SAME AS SEQUENCE LISTED IN WORKITEMLISTFILETOKEN.H
  sessionItems[sessionItemIndex].survey = new seriesSurvey(this, seriesSurveyMenu, false);
  sessionItems[sessionItemIndex].survey->setObjectName(QStringLiteral("survey"));
  sessionItems[sessionItemIndex].workItemProcess->addWidget(sessionItems[sessionItemIndex].survey);

  sessionItems[sessionItemIndex].define = new targetDefine(this, targetDefineMenu, false);
  sessionItems[sessionItemIndex].define->setObjectName(QStringLiteral("define"));
  sessionItems[sessionItemIndex].workItemProcess->addWidget(sessionItems[sessionItemIndex].define);

  sessionItems[sessionItemIndex].analyze = new patientAnalyze(this, patientAnalyzeMenu, false);
  sessionItems[sessionItemIndex].analyze->setObjectName(QStringLiteral("analyze"));
  sessionItems[sessionItemIndex].workItemProcess->addWidget(sessionItems[sessionItemIndex].analyze);

  sessionItems[sessionItemIndex].report = new patientReport(this, patientReportMenu, false);
  sessionItems[sessionItemIndex].report->setObjectName(QStringLiteral("report"));
  sessionItems[sessionItemIndex].workItemProcess->addWidget(sessionItems[sessionItemIndex].report);

  // connect the signals
  connect(sessionItems[sessionItemIndex].survey, SIGNAL(giveScreenControlToDefineFromSurvey(QStackedWidget *, imageSeries *)), sessionItems[sessionItemIndex].define, SLOT(acceptScreenControlFromSurvey(QStackedWidget *, imageSeries *)));
  connect(sessionItems[sessionItemIndex].survey, SIGNAL(resetAllTargetsDueToChangesInImages()), sessionItems[sessionItemIndex].define, SLOT(resetAllTargetsDueToChangesInImages()));
  connect(sessionItems[sessionItemIndex].survey, SIGNAL(resetAllTargetsDueToChangesInImages()), sessionItems[sessionItemIndex].define, SLOT(resetAllTargetsDueToChangesInImages()));
  connect(sessionItems[sessionItemIndex].analyze, SIGNAL(giveScreenControlToDefineFromAnalyze(QStackedWidget *, imageSeries *, targetDef *)), sessionItems[sessionItemIndex].define, SLOT(acceptScreenControlFromAnalyze(QStackedWidget *, imageSeries *, targetDef *)));
  connect(sessionItems[sessionItemIndex].analyze, SIGNAL(packageDataAndTransitionToReport()), sessionItems[sessionItemIndex].select, SLOT(packageDataAndTransitionToReport()));
  connect(sessionItems[sessionItemIndex].select, SIGNAL(giveScreenControlToReport(QString, QUrl, QProgressDialog *)), sessionItems[sessionItemIndex].report, SLOT(acceptScreenControl(QString, QUrl, QProgressDialog *)));
  connect(sessionItems[sessionItemIndex].define, SIGNAL(giveScreenControlToAnalyze(QStackedWidget *, imageSeries *)), sessionItems[sessionItemIndex].analyze, SLOT(acceptScreenControl(QStackedWidget *, imageSeries *)));
  connect(sessionItems[sessionItemIndex].define, SIGNAL(setCurrentTarget(targetDef *)), sessionItems[sessionItemIndex].analyze, SLOT(setCurrentTarget(targetDef *)));
  connect(sessionItems[sessionItemIndex].survey, SIGNAL(backingSeriesChanged(imageSeries *)), sessionItems[sessionItemIndex].define, SLOT(resetBackingSeries(imageSeries *)));
  connect(sessionItems[sessionItemIndex].survey, SIGNAL(backingSeriesChanged(imageSeries *)), sessionItems[sessionItemIndex].analyze, SLOT(resetBackingSeries(imageSeries *)));
  connect(sessionItems[sessionItemIndex].survey, SIGNAL(logUpdate(QString, QString, stageParameters *, int)), this, SLOT(logUpdate(QString, QString, stageParameters *, int)));
  connect(sessionItems[sessionItemIndex].define, SIGNAL(logUpdate(QString, QString, stageParameters *, int)), this, SLOT(logUpdate(QString, QString, stageParameters *, int)));
  connect(sessionItems[sessionItemIndex].analyze, SIGNAL(logUpdate(QString, QString, stageParameters *, int)), this, SLOT(logUpdate(QString, QString, stageParameters *, int)));
  connect(sessionItems[sessionItemIndex].report, SIGNAL(logUpdate(QString, QString, stageParameters *, int)), this, SLOT(logUpdate(QString, QString, stageParameters *, int)));
  connect(sessionItems[sessionItemIndex].define, SIGNAL(processCompositionSettingsChange()), sessionItems[sessionItemIndex].analyze, SLOT(processCompositionSettingsChange()));
  
  // label the tab and initialize the session item
  ui->analyses->setTabText(ui->analyses->currentIndex(), wi->getTabLabel());
  sessionItems[sessionItemIndex].select->setOpenAnalysisCount(++openAnalysisCount); 
  wi->setSurvey(sessionItems[sessionItemIndex].survey);
  wi->setDefine(sessionItems[sessionItemIndex].define);
  wi->setAnalyze(sessionItems[sessionItemIndex].analyze);
  wi->setReport(sessionItems[sessionItemIndex].report);

  // realize each window
  QProgressDialog preloadProgressIndicator(tr("Preloading Data..."), tr("Cancel"), 0, 0, owner);
  preloadProgressIndicator.setWindowModality(Qt::ApplicationModal);
  preloadProgressIndicator.setMinimumDuration(0);
  preloadProgressIndicator.setWindowFlags(preloadProgressIndicator.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::WindowStaysOnTopHint);
  preloadProgressIndicator.setCancelButton(nullptr);   // no cancel button on dialog
  preloadProgressIndicator.show();
  
  sessionItems[sessionItemIndex].workItemProcess->setCurrentIndex(PATIENTANALYZE);
  sessionItems[sessionItemIndex].workItemProcess->setCurrentIndex(TARGETDEFINE);
  sessionItems[sessionItemIndex].workItemProcess->setCurrentIndex(SERIESSURVEY);

  // go through the preload sequence
  sessionItems[sessionItemIndex].report->preloadReport(thisProduct, sessionItemIndex, wi->getWorkItemUpdates());
  sessionItems[sessionItemIndex].define->preloadDefinePre(thisProduct, sessionItemIndex, wi->pipeline, wi->getImageSeriesSet(), wi->getTargetDefs());
  sessionItems[sessionItemIndex].analyze->preloadAnalyze(thisProduct, sessionItemIndex, wi->pipeline, wi->getImageSeriesSet(), wi->getTargetDefs());
  sessionItems[sessionItemIndex].survey->preloadSurvey(thisProduct, sessionItemIndex, wi->pipeline, wi->getImageSeriesSet());
  wi->pipeline->GetMultiImageReader()->SetUseThread(true); // can now start to use threads for loading images
  sessionItems[sessionItemIndex].define->preloadDefinePost();
  
  // disconnect work item list and connect the menu actions
  sessionItems[sessionItemIndex].select->disconnectMenuActions();
  sessionItems[sessionItemIndex].select->disableMenuActions();
  connect(workItemMenu, SIGNAL(triggered(QAction *)), this, SLOT(listWorkItemsButton_clicked()));
  workItemMenu->setEnabled(true);
  // Go in reverse order so upstream has alst say on what is enabled vs. downstream.
  sessionItems[sessionItemIndex].report->connectMenuActions();
  sessionItems[sessionItemIndex].analyze->connectMenuActions();
  sessionItems[sessionItemIndex].define->connectMenuActions();
  sessionItems[sessionItemIndex].survey->connectMenuActions();

  // provide means to access the work item list to select another
  displayListWorkItemsButton();
  
  // as appropriate, proceed to analyze
  /*if ((wi->getImageSeriesSet()->size() > 0) && (wi->getTargetDefs()->size() > 0))
    sessionItems[sessionItemIndex].define->on_continueWithAnalysisButton_clicked();*/

  // now that preloading is done, take down the progress indicator and enable the gui again
  preloadProgressIndicator.setMaximum(100); // any value will take it down
  preloadProgressIndicator.hide();
  this->setEnabled(true);
  ui->centralWidget->setEnabled(true);
  return;
}

void cap::on_tabCloseRequested(int tabIndex) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // switch to the one being closed so that the logic below works
  /*begin signal block (to avoid superfluous "current changed" calls)...*/const bool isBlocked4 = ui->analyses->blockSignals(true);
    ui->analyses->setCurrentIndex(tabIndex);
  /*...end signal block*/ui->analyses->blockSignals(isBlocked4);

  // now that we've switched to it, determine what this tab was for
  int i;
  for (i=0; i<MAXSESSIONITEMS; i++)
    if (sessionItems[i].analysis == ui->analyses->currentWidget())
      break;
  
  if ((i >= MAXSESSIONITEMS) && (tabIndex != 0)) {
    message->showMessage(tr("Error: Can't find current session item."));
    return;
  }
  
  if (i < MAXSESSIONITEMS) {
     // handle the corner case of this tab being only a "+" sign
     // (now re-labelled "Work Item List" to look the same as when a Work Item List tab has been opened),
     // i.e., not even having the stack determine the nature of the tab so that the right action can be taken
    if (sessionItems[i].workItemProcess->currentIndex() == WORKITEM) {
      // we'll need to provide means to access the work item list to select another
      displayListWorkItemsButton();
    }
    else {
      // otherwise, it was an analysis.  
      workItemListEntry *wi = &((*workItemList())[sessionItems[i].select->selectedWorkItem]);
      wi->setSurvey(NULL);
      wi->setDefine(NULL);
      wi->setAnalyze(NULL);
      wi->setReport(NULL);
      
      // determine whether there are changes that need to be saved before closing
      if (workItemList()->at(sessionItems[i].select->selectedWorkItem).unsavedChanges) {
        if ((systemPreferencesObject->getPromptBeforeSave()) || (sessionItems[i].select->getCurrentlyOpenWorkItemListItem() == NULL)) {
          // either the user wants to be prompted or the list hasn't been named yet so they can't avoid being prompted
          QMessageBox msgBox(this);
          msgBox.setText(tr("There are unsaved changes in the work item associated with the analysis tab."));
          msgBox.setInformativeText(tr("Do you want to save changes before closing the analysis?"));
          msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
          msgBox.setDefaultButton(QMessageBox::Yes);
          msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
          int ret = msgBox.exec();
          if (ret == QMessageBox::Yes) {
            save();
            wi->closeOutTargetsFromCompletedSession();
          }
          else {
            sessionItems[i].select->setEntryUnsavedChangesFlag(0, false);
            // need to revert work item (unless the list hadn't even been named yet, in which case there is no revert)
            // 1. Save a session backup 
            // 2. Retrieve the saved json object from last time it had been read from file.
            // 3. Remove the prior abandoned entry.
            // 4. Load the saved object into a new entry.
            // 5. reset the list's unsavedChanges flag accordingly.
            if (sessionItems[i].select->getListFileName() != "") {
              saveWorkItemDataToDisk(sessionItems[i].select->getListFileName(), Json, false); // last arg signifies to save the backup but not the list
              wi->closeOutTargetsFromCompletedSession();
              QJsonObject *wiObject = NULL;
              if (wi->wiObjectFromFile) {
                wiObject = new QJsonObject(*(wi->wiObjectFromFile));
              }
              sessionItems[i].select->on_deleteWorkItemButton_clicked(); // should delete the selected one, which should be the one we started from
              if (wiObject) {
                // note that this combined with the handling of json object before the delete, will result in previously existing wi's beign reverted whereas new wi's being just abandoned (which is what is needed)
                workItemListEntry *newEntry = new workItemListEntry(this);
                newEntry->wiObjectFromFile = wiObject; // in case it needs to be reverted again
                newEntry->readWorkItemListEntry(*wiObject);
                // need to reset the wi data in the peer classes (since the wi they had has now been changed, there destructor methods may fail with bad pointers unless we do this)
                sessionItems[i].report->resetWI(wi->getWorkItemUpdates());
                sessionItems[i].define->resetWI(nullptr, wi->getImageSeriesSet(), wi->getTargetDefs());
                sessionItems[i].analyze->resetWI(nullptr, wi->getImageSeriesSet(), wi->getTargetDefs());
                sessionItems[i].survey->resetWI(nullptr, wi->getImageSeriesSet());
                // now put it in the list
                workItemList()->insert(0, *newEntry);
                emit sessionItems[i].select->addWorkItemToModel();
                sessionItems[i].select->setEntryUnsavedChangesFlag(0, false);
              }
              else {
                // need to reset the wi data in the peer classes (since the wi they had has now been changed, there destructor methods may fail with bad pointers unless we do this)
                // in the above cases, there was a wiObject to revert to, and as such, values can be taken from its constructor.  but in this case, they are null.
                sessionItems[i].report->resetWI(nullptr);
                sessionItems[i].define->resetWI(nullptr, NULL, NULL);
                sessionItems[i].analyze->resetWI(nullptr, NULL, NULL);
                sessionItems[i].survey->resetWI(nullptr, NULL);
              }
              setUnsavedChangesFlag(false);
              for (int j=0; j < workItemList()->size(); j++) {
                if (workItemList()->at(j).unsavedChanges == true) {
                  setUnsavedChangesFlag(true);
                }
              }
            }
          }
        }
        else if (sessionItems[i].select->getCurrentlyOpenWorkItemListItem() != NULL) { // the case when user doesn't want to be prompted but there is an open list
          save();
          wi->closeOutTargetsFromCompletedSession();
        }
        sessionItems[i].survey->disconnectMenuActions();
        delete sessionItems[i].survey;
        sessionItems[i].define->disconnectMenuActions();
        delete sessionItems[i].define;
        sessionItems[i].analyze->disconnectMenuActions();
        delete sessionItems[i].analyze;
        sessionItems[i].report->disconnectMenuActions();
        delete sessionItems[i].report;
      }
      else {
        sessionItems[i].survey->disconnectMenuActions();
        sessionItems[i].define->disconnectMenuActions();
        sessionItems[i].analyze->disconnectMenuActions();
        sessionItems[i].report->disconnectMenuActions();
        wi->closeOutTargetsFromCompletedSession();
      }
      
      // decrement the count
      --openAnalysisCount; 
    }

    // remove its tab as no longer used, delete the objects, NULL out the pointers in the session item data structure
    /*begin signal block (to avoid superfluous "current changed" calls)...*/const bool isBlocked0 = ui->analyses->blockSignals(true);
      ui->analyses->removeTab(tabIndex);
    /*...end signal block*/ui->analyses->blockSignals(isBlocked0);
    sessionItems[i].select->disconnectMenuActions();
    delete sessionItems[i].select;  
    delete sessionItems[i].workItemProcess;
    delete sessionItems[i].gridLayout_2;
    delete sessionItems[i].analysis;
    sessionItems[i].report = NULL;
    sessionItems[i].analyze = NULL;
    sessionItems[i].define = NULL;
    sessionItems[i].survey = NULL;
    sessionItems[i].select = NULL;
    sessionItems[i].workItemProcess = NULL;
    sessionItems[i].gridLayout_2 = NULL;
    sessionItems[i].analysis = NULL;
    
    // finish by updating the analysis count for any other open tabs; also, connect the menu actions if there is a workitem list tab
    for (i=0; i<MAXSESSIONITEMS; i++) {
      if (sessionItems[i].analysis != NULL) {
        sessionItems[i].select->setOpenAnalysisCount(openAnalysisCount);
        if (sessionItems[i].survey == NULL)
          sessionItems[i].select->connectMenuActions();
      }
    }
  } // endif sessionItems[i].workItemProcess isn't null

  return;
}

void cap::on_currentChanged(int tabIndex) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  int i;
  for (i=0; i<MAXSESSIONITEMS; i++) {
    if (sessionItems[i].analysis == ui->analyses->currentWidget())
      break;
  }
  if ((i >= MAXSESSIONITEMS) && (tabIndex != 0)) {
    message->showMessage(tr("Error: Can't find current session item."));
    return;
  }

  // disconnect/reconnect
  if (sessionItems[priorSessionItemIndex].select != NULL) {
    sessionItems[priorSessionItemIndex].select->disconnectMenuActions();
    if (sessionItems[priorSessionItemIndex].select->selectedWorkItemValid) {
      if (sessionItems[priorSessionItemIndex].survey) sessionItems[priorSessionItemIndex].survey->disconnectMenuActions();
      if (sessionItems[priorSessionItemIndex].define) sessionItems[priorSessionItemIndex].define->disconnectMenuActions();
      if (sessionItems[priorSessionItemIndex].analyze) sessionItems[priorSessionItemIndex].analyze->disconnectMenuActions();
      if (sessionItems[priorSessionItemIndex].report) sessionItems[priorSessionItemIndex].report->disconnectMenuActions();
      if (sessionItems[priorSessionItemIndex].analyze) sessionItems[priorSessionItemIndex].analyze->dismissCompositionControls();
    }
  }    

  // in with the new.  
  sessionItems[i].select->connectMenuActions();
  if ((sessionItems[i].workItemProcess->currentIndex() != WORKITEM) && sessionItems[i].select->selectedWorkItemValid) {
    // Go in reverse order so upstream has last say on what is enabled vs. downstream.
    sessionItems[i].report->connectMenuActions();
    sessionItems[i].analyze->connectMenuActions();
    sessionItems[i].define->connectMenuActions();
    sessionItems[i].survey->connectMenuActions();
  }

  // and update the prior index for next time
  qInfo() << "priorSessionItemIndex was " << priorSessionItemIndex << " and will now be " << i;
  priorSessionItemIndex = i;
  return;
}

void cap::save()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (!getUnsavedChangesFlag()) {
    QMessageBox msgBox(this);
    msgBox.setText(tr("There are no unsaved changes which need to be saved."));
    msgBox.setInformativeText(tr("Do you want to save anyway? Yes means perform a save, No means don't perform a save."));
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    int ret = msgBox.exec();
    if (ret == QMessageBox::No) {
      return; // leave things as they are if user says no
    }
  }

  // next, check if there isn't an established list yet, in which case we need to use on_createWorkItemListButton_clicked() instead
  if ((getListDirectory().endsWith(WORKING_STORAGE_FOLDER)) || (getListDirectory().endsWith(TEMPORARY_WORKITEM_FOLDER))) {
    (void) saveAs();
  }
  else {
    if (!saveWorkItemDataToDisk(getListProvenance()->getListFileName(), Json, true)) {
      return; // leave things as they are if user cancels
    }
  }
  if (!getUnsavedChangesFlag()) // presumably this will have been cleared if everything worked above, and we only want to emit if it was all good
    emit updateWorkItemDisplaysGivenSave();
}

bool cap::saveAs()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  bool ok;
  QString prompt;
  if ((getListDirectory().endsWith(WORKING_STORAGE_FOLDER)) || (getListDirectory().endsWith(TEMPORARY_WORKITEM_FOLDER))) {
    prompt = tr("List not named yet, please enter desired list name:");
  }
  else {
    prompt = tr("Desired list name:");
  }
  QString listName=QInputDialog::getText(this, tr("Work Item List"), prompt, QLineEdit::Normal, "", &ok);
  if (ok && !listName.isEmpty()) {
    QString workItemListFileName = cap::getCAPExamDataDir()+WORKING_STORAGE_FOLDER;
    QString prefixedListName = "/wilist_"+listName;
    workItemListFileName.append(prefixedListName);

    // this can take some time, disable while it takes place
    this->setEnabled(false);

    // then determine if this is working from temporary, rather than an already established list, in which case a rename is needed
    if (getListDirectory().endsWith(TEMPORARY_WORKITEM_FOLDER)) {
      // folder already exists, but called temporary.  Re-name it.
      QString workingStoragePath = cap::getCAPExamDataDir()+WORKING_STORAGE_FOLDER;
      QString temporaryPath = workingStoragePath+TEMPORARY_WORKITEM_FOLDER;
      QDir::setCurrent(workingStoragePath);
      QDir currDir(QDir::current());
      if (!currDir.rename(temporaryPath, workItemListFileName)) {
        message->showMessage(tr("Error: Cannot rename from temporary (contact Elucid if condition persists)"));
        this->setEnabled(true);
        return false;
      }
      QDir::setCurrent(workItemListFileName); // ...still just the dir name
    }
    else {
      // need to make the folder
      QDir currDir(QDir::current());
      // first ensure that it doesn't already exist
      if (currDir.exists(workItemListFileName)) {
        message->showMessage(tr("Error: Cannot use Save As to an already existing folder (either use Save to update current list, or Save As to a new name)"));
        this->setEnabled(true);
        return false;
      }
      // having established that it doesn't already exist, make it
      if (!currDir.mkpath(workItemListFileName)) {
        message->showMessage(tr("Error: Cannot create list folder (contact Elucid if condition persists)"));
        this->setEnabled(true);
        return false;
      }
    }
    workItemListFileName.append(prefixedListName+".json"); // ...now adds the list json to it
    getListProvenance()->setListFileName(workItemListFileName);

    // now reset the provenance fields, we're starting from scratch
    QString name = getUserName();
    getListProvenance()->setLastModifier(name);
    QString str = getISODateTime();
    getListProvenance()->setPerformer(name); // ...originator
    getListProvenance()->setPerformDateTime(str);
    getListProvenance()->setLastModifier(name); // ...last modifier same as originator at this time
    getListProvenance()->setLastModifiedDateTime(str);

    // finally do the save itself
    if (!saveWorkItemDataToDisk(workItemListFileName, Json, true)) {
      this->setEnabled(true);
      return true; // leave things as they are if user cancels
    }
    emit updateWorkItemDisplaysGivenSave();
  }
  this->setEnabled(true);
  return true;
}

QString repointedFile(QString input, unsigned piecesToRemove)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QString file = QDir::current().relativeFilePath(input);
  QStringList pieces = file.split("/");
  QString result = pieces.value(0);
  for (int k=1; k<pieces.length()-piecesToRemove; k++) {
    result.append("/");
    result.append(pieces.value(k));
  }
  return result;
}

bool cap::saveWorkItemDataToDisk(QString listFilePath, saveFormat format, bool fullSave)
{
  ebLog eblog(Q_FUNC_INFO); eblog << listFilePath.toStdString() << std::endl;

  //QFile saveFile(format == Json
  //  ? QStringLiteral(fileName)
  //  : QStringLiteral("junk_test_wilist.dat"));
  QFileInfo proposedSaveFile(listFilePath);
  QString fileName = proposedSaveFile.fileName();
  QDir proposedListDirectoryDir = proposedSaveFile.dir();

  // verify that the name meets conventions
  while (!fileName.startsWith("wilist") || !fileName.endsWith(".json")) {
    QMessageBox msgBox(owner);
    msgBox.setText(tr("The file name does not meet conventions which will assist in opening the file later."));
    msgBox.setInformativeText(tr("Would you like to us to make the name conform? (Yes means we will make it conform, No means you wish to type a new name yourself)"));
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    int ret = msgBox.exec();
    if (ret == QMessageBox::Yes)  
      break; 
    else {
      listFilePath=QFileDialog::getSaveFileName(owner, tr("Work Item List File (wilist*.json)"), getListProvenance()->getListFileName(), tr("Work Item List Files (wilist*.json)"));
      if (listFilePath == "")
        return false; // user cancelled
      proposedSaveFile = QFileInfo(listFilePath);
      fileName = proposedSaveFile.fileName();
      proposedListDirectoryDir = QDir(proposedSaveFile.dir());
    }
  }

  // now enforce the conventions
  if (!fileName.startsWith("wilist"))
    fileName.prepend("wilist_");
  if (!fileName.endsWith(".json"))
    fileName.append(".json");

  // now that we are guarranteed that the name meets conventions, reset the objects
  listFilePath = proposedListDirectoryDir.path();
  listFilePath.append("/");
  listFilePath.append(fileName);
  QFile saveFile(listFilePath);
  QDir listDirectoryDir(listFilePath);
  getListProvenance()->setListFileName(listFilePath);
  QFileInfo listFileInfo(listFilePath);
  setListDirectory(listFileInfo.dir().absolutePath());

  // also automatically make a backup
  QString sessionBackupPath = cap::getCAPExamDataDir()+SESSION_BACKUPS_FOLDER+"/"+fileName;
  sessionBackupPath.replace(".json","");
  sessionBackupPath.append("_");
  QDateTime dt = QDateTime::currentDateTime();
  sessionBackupPath.append(dt.toString("yyyyMMdd-hhmm"));
  sessionBackupPath.append(".json");
  QFile backupFile(sessionBackupPath);
  QDir sessionBackupDir(sessionBackupPath);

  // check that the files will work
  if ((getListProvenance()->getListFileName() != listFilePath) && (saveFile.exists())) {
    QMessageBox msgBox(owner);
    msgBox.setText(tr("The file name you specified already exists."));
    msgBox.setInformativeText(tr("Would you like to over-write the file? (Yes means the older file will be replaced by the new, No means you wish to keep the existing file unchanged)"));
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    int ret = msgBox.exec();
    if (ret == QMessageBox::No)  
      return false; // leave things as they are if user cancels
  }
  if (!backupFile.open(QIODevice::WriteOnly)) {
      message->showMessage(tr("Error: Cannot open session backup file for saving."));
      return false;
  }
  if (fullSave) {
    if (!saveFile.open(QIODevice::WriteOnly)) {
      message->showMessage(tr("Error: Cannot open work item list file for saving."));
      return false;
    }
  }

  // we have enough confidence now to put up a progress dialog
  QProgressDialog progressDialog;
  if (fullSave) {
    progressDialog.setWindowModality(Qt::NonModal);
    progressDialog.setMinimum(0);  // for spinning bar since number not known in advance
    progressDialog.setMaximum(0);  // for spinning bar since number not known in advance
    progressDialog.setCancelButton(nullptr);   // no cancel button on dialog
    progressDialog.setWindowFlags(progressDialog.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    progressDialog.setLabelText(tr("Saving List Objects..."));
    progressDialog.show();
    this->repaint(); // ensure progress is shown
    qApp->processEvents();
  }

  // we'll need to re-point the image series' folders, and to facilitate this, make them absolute now (ultimately to be relative to new location later)
  for (int i=0; i < workItemList()->size(); i++) {
    workItemListEntry *wi = &(*(workItemList()))[i];
    for (int j=0; j < wi->images_cg.size(); j++) {
      imageSeries *series = &((wi->images_cg)[j]);
      QDir seriesFolderDir(series->seriesFolder);
      if (!series->seriesFolder.isEmpty() && seriesFolderDir.exists()) {
          QFileInfo imageFileInfo = seriesFolderDir.entryInfoList().last(); // note that in this temporary state it will actually name one of the files, not the folder
          QString imageFilePath = imageFileInfo.absoluteFilePath();
          series->seriesFolder = imageFilePath;
      }
    }
  }

  // switch to the directory to facilitate repointing of files as necessary
  QString oldDir = QDir::currentPath(); // keep it to use below
  QDir::setCurrent(QFileInfo(listDirectoryDir.path()).absolutePath());
  QString newDir = QDir::currentPath();
  
  // with knowledge that the save file for the list will work, first save the derived data items for the targets
  for (int i=0; i < workItemList()->size(); i++) {
    workItemListEntry *wi = &(*(workItemList()))[i];
    if (!wi->setWorkItemFolder()) { // this may be the first time an operation that requires the folder has been done
      progressDialog.setValue(0);  // make dialog go away
      message->showMessage(tr("Error: Cannot write work item folder (contact Elucid if condition persists)"));
      return false;
    }

    // need to repoint the image file references with respect to location of this list file
    for (int j=0; j < wi->images_cg.size(); j++) {
      imageSeries *series = &((wi->images_cg)[j]);
      if (!series->seriesFolder.isEmpty()) {
          series->seriesFolder = repointedFile(series->seriesFolder, 1/*don't include file name*/);
      }
      series->seriesThumbFile = ""; // had been repointedFile(series->seriesThumbFile, 0/*leave file name*/); but better to make it blank to force it to be re-generated in new place as needed
    }

    // now save the target derived data
    for (int k=0; k < wi->targets.size(); k++) {
      targetDef *def = &((wi->targets)[k]);
      if (newDir != oldDir) { // start with new basis if needed
        QString newTargetFolder = QDir::cleanPath(newDir + QLatin1Char('/') + def->getTargetFolder());
        QDir currDir(QDir::current());
        if (!currDir.mkpath(newTargetFolder)) {
          progressDialog.setValue(0);  // make dialog go away
          message->showMessage(tr("Error: Cannot create target copy folder (contact Elucid if condition persists)"));
          return false;
        }
        QString srcFilePath = QDir::cleanPath(oldDir + QLatin1Char('/') + def->getTargetFolder());
        QDir sourceDir(srcFilePath);
        QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        foreach (const QString &fileName, fileNames) {
          const QString newSrcFilePath = srcFilePath + QLatin1Char('/') + fileName;
          const QString newTgtFilePath = newTargetFolder + QLatin1Char('/') + fileName;
          if (!QFile::copy(newSrcFilePath, newTgtFilePath)) {
            progressDialog.setValue(0);  // make dialog go away
            message->showMessage(tr("Error: Cannot copy files from ")+newSrcFilePath+tr(" to new list folder ")+newTgtFilePath+tr(" (contact Elucid if condition persists)"));
            return false;
          }
        }
      }
      def->saveTargetDerivedDataChanges(newDir); // then follow up with saving changes, if there are any
    } // end-for each target
    wi->unsavedChanges = false;
  } // end-for each work item

  // first save the backup
  QJsonObject wiListObject;
  writeWorkItemList(wiListObject);
  QJsonDocument saveDoc(wiListObject);
  backupFile.write(format == Json
    ? saveDoc.toJson()
    : saveDoc.toBinaryData());
  backupFile.flush();
  backupFile.close();

  // return early if it isn't a full save request
  if (!fullSave) {
    return true;
  }

  // now the workitem list itself
  saveFile.write(format == Json
    ? saveDoc.toJson()
    : saveDoc.toBinaryData());
  saveFile.flush();
  saveFile.close();

  setUnsavedChangesFlag(false);
  progressDialog.setValue(0);  // make dialog go away
  this->repaint(); // ensure progress is shown
  qApp->processEvents();

  // if the extract script is present, call it too
  QString extractScriptName = cap::getCAPExamDataDir()+"/wilistSaveExtractScript.py";
  QFileInfo extractScript(extractScriptName);
  if (extractScript.exists() && extractScript.isFile() && extractScript.isReadable()) {
    QProcess *extractProcess = new QProcess(owner);
    try {
#ifdef _WIN32
      QString program = "C:\\Python27\\python";
#else
      QString program = "/usr/bin/python2.7";
#endif
      QStringList arguments;
      arguments << extractScriptName << "--inputList" << listFilePath.replace(" ", "%20");
      if (systemPreferencesObject->getDisplayExtractMessages()) {
        QString informationalMessage = "wilistSaveExtractScript present, so will call ";
        informationalMessage.append(extractScriptName);
        informationalMessage.append(" --inputList ");
        informationalMessage.append(listFilePath.replace(" ", "%20"));
        QMessageBox::information(owner, tr("wilistSaveExtractScript present, so will call"), informationalMessage);
      }
      extractProcess->start(program, arguments);
    } catch (std::exception &e) {
      eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
      QMessageBox::warning(owner, tr("wilistSaveExtractScript caused exception"), QString(tr("wilistSaveExtractScript caused exception (should not effect normal CAP operation)")));
    } catch (...) {
      eblog << "EXCEPTION CAUGHT: non-standard exception" << std::endl;
      QMessageBox::warning(owner, tr("wilistSaveExtractScript caused exception"), QString(tr("wilistSaveExtractScript caused exception (should not effect normal CAP operation)")));
    }
    int wait = extractProcess->waitForFinished(100000);
    int estat = extractProcess->exitStatus();
    int ecode = extractProcess->exitCode();
    if (!wait || (QProcess::NormalExit != estat) || (0 != ecode)) {
      eblog << "NOTICE: wilistSaveExtractScript (wait, status, code): " << wait << " " << estat << " " << ecode << " wilist: " << listFilePath.toStdString() << std::endl;
    }
  }
  return true;
}

void cap::writeWorkItemList(QJsonObject &json)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QJsonObject listProvenanceObject;
  getListProvenance()->writeWorkItemListProvenance(listProvenanceObject);
  json[listProvenance_token] = listProvenanceObject;
  QJsonArray wiArray;
  for (int i=0; i < workItemList()->count(); i++) {
    workItemListEntry *wi = &(*workItemList())[i];
    QJsonObject wiObject;
    wi->writeWorkItemListEntry(wiObject);
    wiArray.append(wiObject);
  }
  json[workItemList_token] = wiArray;
}



/*virtual * / bool cap::eventFilter(QObject *obj, QEvent *event) / *override* /
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // Original purpose: see if the event indicates that a reporting activity wants to pull back to analyze
  // See note at center of function for current thinking on use of this event filter.
  if ((obj == ui->analyses) && (event->type() == QEvent::MouseButtonPress)) {
    int i;
    for (i=0; i<MAXSESSIONITEMS; i++) {
      if (sessionItems[i].analysis == ui->analyses->currentWidget()) {
        if (sessionItems[i].workItemProcess->currentIndex() == PATIENTREPORT) {
          if (((QMouseEvent*)event)->button() == Qt::LeftButton) {
            * This was first thought at pulling back to Analyze fromn Report, before the decision to re-organize the report screen to have an 
             * explict button. But this code is left here, as it may provide a useful way to implement a hook for gathering diagnostic information,
             * e.g., by re-arranging the tests that are here at the moment and dumping out a data set under various conditions, or other needs that
             * arise as product is rolled-out.  So this is just a placeholder now.
            sessionItems[i].workItemProcess->setCurrentIndex(PATIENTANALYZE);
            return true*;
          } // end-if left mouse button
        } // end-if the current tab is on the report widget
      } // end-if found the current analysis in the list
      if (sessionItems[i].select != NULL) {
        qInfo() << "for sessionitem " << i;
        qInfo() << "select:" << sessionItems[i].select;
        qInfo() << "survey:" << sessionItems[i].survey; if (sessionItems[i].survey != NULL) std::cerr << *(sessionItems[i].survey->viewers);
        qInfo() << "define:" << sessionItems[i].define; if (sessionItems[i].define != NULL) std::cerr << *(sessionItems[i].define->viewers);
        qInfo() << "analyze:" << sessionItems[i].analyze; if (sessionItems[i].analyze != NULL) std::cerr << *(sessionItems[i].analyze->viewers);
        qInfo() << "report:" << sessionItems[i].report;
        qInfo() << "workItemProcess:" << sessionItems[i].workItemProcess;
        qInfo() << "gridLayout2:" << sessionItems[i].gridLayout_2;
        qInfo() << "analysis:" << sessionItems[i].analysis;
      }
    } // end-for each session item that might be this one
  } // end-if this is a mouse event for the high-level "analyses" object

  // reach here because it wasn't one of the conditions we were looking for in this filter
  return false;
};*/

void cap::on_cap_destroyed()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  quitting = true;
  qApp->quit();
}

void cap::enableCAP(bool enabled) { ui->analyses->setEnabled(enabled); }
bool cap::CAPisEnabled() { return ui->analyses->isEnabled(); }

/** @} */
