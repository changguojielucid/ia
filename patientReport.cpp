// Copyright (c) Elucid Bioimaging

#include <QMessageBox>
#include <QNetworkReply>
#include <QWebPage>
#include <QWebInspector>
#include <QErrorMessage>

#include "patientReport.h"
#include "ui_patientReport.h"
#include "cap.h"

/**
 * \ingroup patientReport
 * @{
 *
 * See patientReport.h for description of the package purpose and contents.  This file has the member functions for classes in the package.
 */

patientReport::patientReport(QWidget *p, QMenu *m, bool masterWithRespectToMenu) :
  QWidget(p),
  ui(new Ui::patientReport)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner = dynamic_cast<cap *>(p);
  patientReportMenu = m;
  progressIndicator = NULL;

  if (masterWithRespectToMenu) {
    patientReportMenu->addSeparator();
    gotoPatientReportAction = new QAction(tr("Go to Patient Report"), this);
    gotoPatientReportAction->setObjectName("gotoPatientReport");
    patientReportMenu->addAction(gotoPatientReportAction);

    gotoPatientReportAction->setEnabled(false);
    patientReportMenu->setEnabled(false);
  } 
  else if (owner != 0) {
    foreach (QAction *a, patientReportMenu->actions()) {
      if (a->objectName() == "gotoPatientReport")
        gotoPatientReportAction = a;
      menuActionEnabledMap[a] = true;
    }
  }

  if ((owner != 0) && (masterWithRespectToMenu == false)) {
    ui->setupUi(this);  
    message = new QErrorMessage (this);
    message->setWindowFlags(message->windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint);
    patientReportMenu = m;
    QString buttonText = QChar(0x25C0);
    buttonText.append(tr(" Back to Analyze"));
    ui->backToAnalyzeButton->setText(buttonText);
    ui->backToAnalyzeButton->setEnabled(false);

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

patientReport::~patientReport()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  // if active, take down progress indicator that would otherwise be orphaned
  if (progressIndicator == NULL)
    delete progressIndicator;

  if (owner != 0) {
    delete message;
    delete ui;
  }
}

void patientReport::disconnectMenuActions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  disconnect(patientReportMenu, SIGNAL(triggered(QAction *)), this, SLOT(ensureOnPatientReportPage()));
  //disconnect(gotoPatientReportAction, SIGNAL(triggered()), this, SLOT(ensureOnPatientReportPage()));
  patientReportMenu->setEnabled(false);
}

void patientReport::connectMenuActions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  connect(patientReportMenu, SIGNAL(triggered(QAction *)), this, SLOT(ensureOnPatientReportPage()));
  //connect(gotoPatientReportAction, SIGNAL(triggered()), this, SLOT(ensureOnPatientReportPage()));
  patientReportMenu->setEnabled(true);
}

void patientReport::reconnectMenuActions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  connectMenuActions();
  for (auto actionEnabled : menuActionEnabledMap)
    actionEnabled.first->setEnabled(actionEnabled.second);
}

void patientReport::disableMenuActions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  gotoPatientReportAction->setEnabled(false);
  patientReportMenu->setEnabled(false);
}

void patientReport::enableMenuActions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  gotoPatientReportAction->setEnabled(true);
  patientReportMenu->setEnabled(true);
}

void patientReport::ensureOnPatientReportPage() 
{ 
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  switch (owner->getWorkItemProcess(sessionItemIndex)->currentIndex()) {
    case SERIESSURVEY:
      owner->getSeriesSurvey(sessionItemIndex)->on_proceedToAnalysisButton_clicked();

    case TARGETDEFINE:
      owner->getTargetDefine(sessionItemIndex)->on_continueWithAnalysisButton_clicked();

    case PATIENTANALYZE:
      owner->getPatientAnalyze(sessionItemIndex)->on_proceedToReportButton_clicked();
  }
}

void patientReport::preloadReport(QString product, int index, QList<workItemUpdate> *updateList)
{
  ebLog eblog(Q_FUNC_INFO); eblog << product.toStdString() << std::endl;
  thisProduct = product;
  sessionItemIndex = index;
  updates = updateList;
  ui->backToAnalyzeButton->setEnabled(true);
  connect(ui->webView->page(),SIGNAL(downloadRequested(QNetworkRequest)),this,SLOT(downloadReport(QNetworkRequest)));
}

void patientReport::resetWI(QList<workItemUpdate> *updateList)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  updates = updateList;
}

void patientReport::acceptScreenControl(QString ID, QUrl url, QProgressDialog *generateProgressIndicator)
{
  ebLog eblog(Q_FUNC_INFO); eblog << url.toString().toStdString() << std::endl;
  progressIndicator = generateProgressIndicator; // keep for later deleting
  this->setEnabled(false);
  qDebug() << "patientReport::acceptScreenControl, generateProgressIndicator is" << generateProgressIndicator;
  generateProgressIndicator->setMinimumDuration(0);
  generateProgressIndicator->show();
  this->repaint(); // ensure progress is shown
  qApp->processEvents();

  workItemID = ID;
  ui->webView->setUrl(url); 

  char *debug = getenv("WEB_DEBUG");
  if (NULL != debug) {
      QWebPage *page = ui->webView->page();
      page->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
      QWebInspector *inspector = new QWebInspector();
      inspector->setPage(page);
      inspector->show();
  }

  // clear out whatever updates were there from before (e.g., the default one from Qt Creator, or maybe the user had been in a reporting activity for this work item before)
  QLayoutItem *item;
  int numOldUpdatesToRemove = ui->verticalLayout->count()-1; //count-1 because want to leave the spring in (rather than deleting it with the other widgets)
  for (int i=0; i < numOldUpdatesToRemove; i++) { 
          item = ui->verticalLayout->takeAt(0);
          delete item->widget();
          delete item;
  }
  
  // display the updates
  QGroupBox *updateBox;
  QGridLayout *gridLayout_2;
  QFormLayout *formLayout;
  QLabel *stepLabel;
  QLabel *updateStep;
  QLabel *byLabel;
  QLabel *updatePerformer;
  for (int i=0; i < updates->size(); i++) {
          updateBox = new QGroupBox(ui->scrollAreaWidgetContents);
    updateBox->setObjectName(QStringLiteral("updateBox"));
    QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(updateBox->sizePolicy().hasHeightForWidth());
    updateBox->setSizePolicy(sizePolicy1);
    updateBox->setMinimumSize(QSize(0, 0));
    gridLayout_2 = new QGridLayout(updateBox);
    gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
    formLayout = new QFormLayout();
    formLayout->setObjectName(QStringLiteral("formLayout"));
    stepLabel = new QLabel(updateBox);
    stepLabel->setObjectName(QStringLiteral("stepLabel"));
    stepLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    formLayout->setWidget(0, QFormLayout::LabelRole, stepLabel);

    updateStep = new QLabel(updateBox);
    updateStep->setObjectName(QStringLiteral("updateStep"));

    formLayout->setWidget(0, QFormLayout::FieldRole, updateStep);

    byLabel = new QLabel(updateBox);
    byLabel->setObjectName(QStringLiteral("byLabel"));
    byLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    formLayout->setWidget(1, QFormLayout::LabelRole, byLabel);

    updatePerformer = new QLabel(updateBox);
    updatePerformer->setObjectName(QStringLiteral("updatePerformer"));

    formLayout->setWidget(1, QFormLayout::FieldRole, updatePerformer);

    gridLayout_2->addLayout(formLayout, 0, 0, 1, 1);
    updateBox->setTitle(updates->at(i).performDateTime);
    stepLabel->setText(QApplication::translate("patientReport", "Step:", 0));
    updateStep->setText(updates->at(i).step);
    byLabel->setText(QApplication::translate("patientReport", "By:", 0));
    updatePerformer->setText(updates->at(i).performer);

    ui->verticalLayout->insertWidget(0, updateBox);
  }

  // set the screen
  owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(PATIENTREPORT);
  generateProgressIndicator->setMaximum(100); // any value will take it down
  progressIndicator = NULL;
  delete generateProgressIndicator;
  enableMenuActions();
  this->setEnabled(true);
}

void patientReport::downloadReport(QNetworkRequest saveAsRequest)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  message->showMessage(tr("Report is being generated. You will receive notification when it is available."));
  this->repaint(); // ensure progress is shown
  qApp->processEvents();
  QNetworkRequest getTheDataRequest(saveAsRequest.url());
  XDSGetReportRAD_68Reply = XDSGetReportRAD_68.get(getTheDataRequest);
  connect(XDSGetReportRAD_68Reply, SIGNAL(finished()), this, SLOT(parseXDSGetReportRAD_68Reply()));
}

void patientReport::parseXDSGetReportRAD_68Reply()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (!XDSGetReportRAD_68Reply->error()) {
    // construct the file name, with location determined by work item folder
    workItemListEntry *wi = owner->getWorkItem(sessionItemIndex)->selectedWorkItemListEntryPtr;
    if (!wi->setWorkItemFolder()) { // this may be the first time an operation that requires the folder has been done
      QMessageBox::warning(this, tr("Error: Cannot save report."), QString("Cannot write work item folder %1.").arg(wi->getWorkItemFolder()));
      return;
    }
    QString reportFile = wi->getWorkItemFolder();
    reportFile.append("/");
    reportFile.append(workItemID);
    reportFile.append("_report__");
    reportFile.append( getLocalUserName() );
    reportFile.append("_");
    QDateTime dt = QDateTime::currentDateTime();
    QString dtStr = dt.toString("yyyyMMdd-hhmm");
    reportFile.append(".pdf");
  	reportOutput.setFileName(reportFile);
  	if (reportOutput.open(QIODevice::WriteOnly)) {
      reportOutput.write(XDSGetReportRAD_68Reply->readAll());
      if (reportOutput.pos() > 0)
        QMessageBox::information(this, tr("Report file has been downloaded and saved to disk"), QString("Please review %1 for accuracy.").arg(reportFile));
      else
        QMessageBox::warning(this, tr("Report file is empty"), QString("Cannot open %1 in write mode.").arg(reportFile));
    }
    else
            QMessageBox::warning(this, tr("Report may not be saved to file"), QString("Cannot open %1 in write mode.").arg(reportFile));
  }
  else
  	QMessageBox::warning(this, tr("Network error on attempting download of report file"), QString("Failed: %1").arg(XDSGetReportRAD_68Reply->errorString()));
  XDSGetReportRAD_68Reply->deleteLater();
}

void patientReport::on_backToAnalyzeButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QMessageBox msgBox;
  msgBox.setText(tr("Going back to Analyze prevents further action on the current report."));
  msgBox.setInformativeText(tr("Do you want to go back to Analyze anyway, leaving the current report in whatever status it presently is?"));
  msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  msgBox.setWindowFlags(message->windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
  int ret = msgBox.exec();
  if (ret == QMessageBox::No)
    return;
  // ok, user has cofirmed intent
  this->setEnabled(false);
  disableMenuActions();
  owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(PATIENTANALYZE);
  this->setEnabled(true);
}

/** @} */
