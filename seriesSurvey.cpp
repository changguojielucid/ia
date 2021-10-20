// Copyright (c) Elucid Bioimaging

#include <string>
#include <vector>

#include <QErrorMessage>
#include <QtDebug>
#include <QApplication>
#include <QMessageBox>
#include <QToolButton>
#include <QStandardPaths>

#include "ebiHelper.h"
#include "ebiImageReader.h"
#include "ebiThumbnailImageFilter.h"
#include "ebvEvent.h"
#include "ebiEventObject.h"

#include "seriesSurvey.h"
#include "ui_seriesSurvey.h"
#include "workItem.h"
#include "cap.h"
#include "ebvOutputWindow.h"

/**
 *  \ingroup seriesSurvey
 * @{
 *
 * See seriesSurvey.h for description of the package purpose and contents.  This file has member functions for classes in the package.
 */

seriesSurvey::seriesSurvey(QWidget *p, QMenu *m, bool masterWithRespectToMenu) :
    QWidget(p),
    ui(new Ui::seriesSurvey) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner = dynamic_cast<cap *>(p);
  seriesSurveyMenu = m;
  currentBackingSeries = NULL;
  viewers = NULL;
  images = NULL;
  preLoading = true;
  
  if ((owner != 0) && (masterWithRespectToMenu == false)) {
    ui->setupUi(this);

    // now tailor according to clinical jurisdiction
    if (owner->clinicalJurisdiction != "") { // any of the clinical editions
      ui->seriesType->clear();
      ui->seriesType->insertItems(0, QStringList()
        << tr("CT")
      );
      ui->modality->clear();
      ui->modality->insertItems(0, QStringList()
        << tr("CT")
      );
      ui->seriesAttributesPrompt->setVisible(false);
      ui->seriesAttributes->setVisible(false);
      ui->fieldStrengthPrompt->setVisible(false);
      ui->fieldStrength->setVisible(false);
      ui->sequencePrompt->setVisible(false);
      ui->sequence->setVisible(false);
    }

    // messages and buttons
    message = new QErrorMessage(this);
    message->setWindowFlags(message->windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint);
    message->setModal(true);
    ui->addSeriesButton->setEnabled(false); // disable the add series button from being hit while we're coming up
          QString buttonText = tr("Proceed to Define ");
          buttonText.append(QChar(0x25B6));
    ui->proceedToAnalysisButton->setText(buttonText);
    ui->proceedToAnalysisButton->setEnabled(false); // likewise the button to proceed

    // remove the placeholder series from the designer (as a reverse of the steps to add)
    ui->seriesSetBox->removeItem(ui->seriesWidget);
    ui->seriesWidget->removeWidget(ui->seriesThumb);
    delete ui->seriesLabel;
    delete ui->seriesThumb;
    delete ui->line_2;
    delete ui->seriesWidget;

    // set up the initial viewers
    viewers = vtkSmartPointer<ebvLinkedViewers2>::New();
    ida = viewers->AddViewer(ebvViewer::AXIAL,ui->workingViewRender0->GetRenderWindow());
    viewers->GetViewer(ida)->SetShowLogo(true);
    idc = viewers->AddViewer(ebvViewer::CORONAL,ui->orthogonalViewRender1->GetRenderWindow());
    viewers->GetViewer(idc)->SetShowLogo(true);
    ids = viewers->AddViewer(ebvViewer::SAGITTAL,ui->orthogonalViewRender2->GetRenderWindow());
    viewers->GetViewer(ids)->SetShowLogo(true);
    viewers->SetScreenScaleFitFactor2D(1.0);
    viewers->OptimizeScreenScale2D();
    vtkOutputWindow::SetInstance(ebvOutputWindow::New());
    viewers->SyncViewersToScene();
    viewers->Render();

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
  } // end-if owner is not 0
  else {
    // this is needed to make the dummy object to establish the menu have a place to add the tool buttons (which won't be used, but need for resusing fucntion)
    ui->workingViewFrame0ControlsBox = new QWidget(0);
  }
}

seriesSurvey::~seriesSurvey() 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (owner != 0) {
    viewers->RemoveViewer(ida);
    viewers->RemoveViewer(idc);
    viewers->RemoveViewer(ids);
    /*viewers = NULL;

    delete message;*/
    delete ui; 
  }
}

void seriesSurvey::establishTools(QString product, int index, bool masterWithRespectToMenu)
{
  // set up the tools, initally all disabled
  menuActionEnabledMap = new std::map<QAction *, bool>;
  tools = new capTools(owner, product, index, viewers, ida, ebvViewer::AXIAL, ui->workingViewFrame0ControlsBox, seriesSurveyMenu, masterWithRespectToMenu, "__PRIMARY__", menuActionEnabledMap);
  tools->disableToolButtons(); // start the buttons disabled
  HIDECONTROL(tools->ShowAsMovieTool, tools->ShowAsMovieAction);//PUT BACK IN WHEN IMPLEMENTED: if (owner != 0) ui->workingViewFrame0Controls->insertWidget(0, tools->ShowAsMovieTool);
  HIDECONTROL(tools->AnnotateTool, tools->AnnotateAction);//PUT BACK IN WHEN IMPLEMENTED: if (owner != 0) ui->workingViewFrame0Controls->insertWidget(0, tools->AnnotateTool);
  if (owner != 0) ui->workingViewFrame0Controls->insertWidget(0, tools->MeasureTool);
  HIDECONTROL(tools->ToggleLumenTool, tools->ToggleLumenAction);
  HIDECONTROL(tools->ToggleWallTool, tools->ToggleWallAction);
  HIDECONTROL(tools->MaxStenosisTool, tools->MaxStenosisAction);
  HIDECONTROL(tools->MaxDilationTool, tools->MaxDilationAction);
  HIDECONTROL(tools->MaxRRTool, tools->MaxRRAction);
  HIDECONTROL(tools->MaxWTTool, tools->MaxWTAction);
  HIDECONTROL(tools->ToggleCornerTool, tools->ToggleCornerAction);
  HIDECONTROL(tools->OptimizeSubvolumeTool, tools->OptimizeSubvolumeAction);
  if (owner != 0) ui->workingViewFrame0Controls->insertWidget(0, tools->ShapeIntensityTool);
  if (owner != 0) ui->workingViewFrame0Controls->insertWidget(0, tools->RestoreViewSettingsTool);
  if (owner != 0) ui->workingViewFrame0Controls->insertWidget(0, tools->SaveViewSettingsTool);
  HIDECONTROL(tools->MaxCalcTool, tools->MaxCalcAction);
  HIDECONTROL(tools->MaxLRNCTool, tools->MaxLRNCAction);
  HIDECONTROL(tools->MaxMATXTool, tools->MaxMATXAction);
  HIDECONTROL(tools->MaxIPHTool, tools->MaxIPHAction);
  HIDECONTROL(tools->MaxUlcTool, tools->MaxUlcAction);
  HIDECONTROL(tools->MaxThrTool, tools->MaxThrAction);
  HIDECONTROL(tools->TargetPathSwitchTool, tools->TargetPathSwitchAction);
  HIDECONTROL(tools->TargetCreateTool, tools->TargetCreateAction);
  HIDECONTROL(tools->TargetDeleteTool, tools->TargetDeleteAction);
  HIDECONTROL(tools->TargetModifyTool, tools->TargetModifyAction);
  HIDECONTROL(tools->ToggleObliqueTool, tools->ToggleObliqueAction);
  HIDECONTROL(tools->VolumeRenderTool, tools->VolumeRenderAction);
  HIDECONTROL(tools->ToggleCoronalTool, tools->ToggleCoronalAction);
  HIDECONTROL(tools->ToggleSagittalTool, tools->ToggleSagittalAction);
  HIDECONTROL(tools->ToggleAxialTool, tools->ToggleAxialAction);
  HIDECONTROL(tools->ToggleColorBarTool, tools->ToggleColorBarAction);
  if (owner != 0) ui->workingViewFrame0Controls->insertWidget(0, tools->CenterAtCursorTool);
  HIDECONTROL(tools->MoveProximalTool, tools->MoveProximalAction);
  HIDECONTROL(tools->MoveDistalTool, tools->MoveDistalAction);
  HIDECONTROL(tools->SaveToReportTool, tools->SaveToReportAction);    

  if (masterWithRespectToMenu) {
    seriesSurveyMenu->addSeparator();
    addSeriesAction = new QAction(tr("Add another image series to the analysis"), this);
    addSeriesAction->setObjectName("addSeries");
    seriesSurveyMenu->addAction(addSeriesAction);
    (*menuActionEnabledMap)[addSeriesAction] = false;

    specifySeriesAction = new QAction(tr("Specify Series"), this);
    specifySeriesAction->setObjectName("specifySeries");
    seriesSurveyMenu->addAction(specifySeriesAction);
    (*menuActionEnabledMap)[specifySeriesAction] = false;

    seriesSurveyMenu->addSeparator();
    gotoSeriesSurveyAction = new QAction(tr("Go to Series Survey"), this);
    gotoSeriesSurveyAction->setObjectName("gotoSeriesSurvey");
    seriesSurveyMenu->addAction(gotoSeriesSurveyAction);
    (*menuActionEnabledMap)[gotoSeriesSurveyAction] = false;

    for (auto actionEnabled : *menuActionEnabledMap)
      actionEnabled.first->setEnabled(false);
  } 
  else {
    foreach (QAction *a, seriesSurveyMenu->actions()) {
      if (a->objectName() == "addSeries")
        addSeriesAction = a;
      else if (a->objectName() == "specifySeries")
        specifySeriesAction = a;
      else if (a->objectName() == "gotoSeriesSurvey")
        gotoSeriesSurveyAction = a;
      (*menuActionEnabledMap)[a] = true;
    }
  }
  (*menuActionEnabledMap)[specifySeriesAction] = false;
  specifySeriesAction->setEnabled(false);
}

void seriesSurvey::disconnectMenuActions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  disconnect(seriesSurveyMenu, SIGNAL(triggered(QAction *)), this, SLOT(ensureOnSeriesSurveyPage()));
  //disconnect(gotoSeriesSurveyAction, SIGNAL(triggered()), this, SLOT(ensureOnSeriesSurveyPage()));
  disconnect(addSeriesAction, SIGNAL(triggered()), this, SLOT(on_addSeriesButton_clicked()));
  disconnect(specifySeriesAction, SIGNAL(triggered()), this, SLOT(specifySeriesCore()));
  tools->disconnectMenuActions();
  disableMenuActions();
}

void seriesSurvey::connectMenuActions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  connect(seriesSurveyMenu, SIGNAL(triggered(QAction *)), this, SLOT(ensureOnSeriesSurveyPage()));
  //connect(gotoSeriesSurveyAction, SIGNAL(triggered()), this, SLOT(ensureOnSeriesSurveyPage()));
  connect(addSeriesAction, SIGNAL(triggered()), this, SLOT(on_addSeriesButton_clicked()));
  connect(specifySeriesAction, SIGNAL(triggered()), this, SLOT(specifySeriesCore()));
  tools->connectMenuActions();
  enableMenuActions();
}

void seriesSurvey::disableMenuActions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  for (auto actionEnabled : *menuActionEnabledMap)
    actionEnabled.first->setEnabled(false);
  seriesSurveyMenu->setEnabled(false);
  owner->getTargetDefine(sessionItemIndex)->disableMenuActions(); // propagate it downstream
}

void seriesSurvey::enableMenuActions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  for (auto actionEnabled : *menuActionEnabledMap)
    actionEnabled.first->setEnabled(actionEnabled.second);
  seriesSurveyMenu->setEnabled(true);
  if (viableSeriesCount > 0)
    owner->getTargetDefine(sessionItemIndex)->enableMenuActions(); // propagate it downstream
  else
    owner->getTargetDefine(sessionItemIndex)->disableMenuActions(); // propagate it downstream
}

void seriesSurvey::ensureOnSeriesSurveyPage() 
{ 
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  this->setEnabled(false);
  if (owner->getWorkItemProcess(sessionItemIndex)->currentIndex() != SERIESSURVEY) {
    if (ui->seriesSelectionArea->count() == 0) {
      // if there are no series selector pages, then we have arrived here from a screen different from seriesSurvey, so reset
      ui->seriesSelectionArea->addWidget(ui->selectPage);
      ui->seriesSelectionArea->addWidget(ui->propertiesPage);
    }
    // compose the screen for surveying the series
    owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(SERIESSURVEY);
  }
  this->setEnabled(true);
}

void seriesSurvey::preloadSurvey(QString product, int index, ebiVesselPipeline::Pointer p, QList<imageSeries> *imageSeriesSet)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  thisProduct = product;
  sessionItemIndex = index;
  pipeline = p;
  images = imageSeriesSet; 
  ui->seriesSelectionArea->setCurrentWidget(ui->selectPage); // default to selectPage
  this->repaint(); // ensure progress is shown
  qApp->processEvents();
  establishTools(thisProduct, sessionItemIndex, false);
  qDebug() << "images->size() is " << images->size();

  // loop through initial description to setup predefined series each according to how well specified they are
  viableSeriesCount = 0;
  int i;
  imageSeries *series;
  for (i=0; i < images->size(); i++) {
    // go through multiple levels of check to see if the specification is complete enough to add
    if (images->at(i).seriesFolder != "") {
      // having established it has a folder, we next see if its type is valid
      series = &((*images)[i]);
      series->progressIndicator = NULL;
      int j;
      for (j=0; j < ui->seriesType->count(); j++) {
        if (images->at(i).seriesType == ui->seriesType->itemText(j))
          break;
      }
      if (j < ui->seriesType->count()) {
        if ((ui->seriesType->itemText(j) != "DCE") && (ui->seriesType->itemText(j) != "PC")) {
          // it has a folder, its type is valid, it is a supported type, so try to add it
          if ((QDir::current().isReadable()) && (QDir(images->at(i).seriesFolder).entryList(QDir::Files).count() > 0)) {
            addSeries(i, ((currentBackingSeries == NULL) ? true : false), series);
          } // end-if the specification is complete enough to add the series
          else {
            message->showMessage(tr("Error: no readable files for preload series in folder."));
            qWarning() << "no readable files for series in folder" << images->at(i).seriesFolder << ", skipping.";
          }
        } // end-if the specification is complete enough to add the series
        else
          qWarning() << "series in folder" << images->at(i).seriesFolder << "is a" << ui->seriesType->itemText(j) << "which is not yet supported, skipping.";
      } // end-if the series type is among the supported types
      else
        qWarning() << "series in folder" << images->at(i).seriesFolder << "has invalid type, skipping.";
    } // end-if the folder is not blank
    else 
      qWarning() << "series in list position" << i << "has no folder specified, skipping.";
  } // end-for each listed series

  // set up traps for the property widgets for purposes of confirming user intent
  ui->seriesType->installEventFilter(this);
  ui->anatomy->installEventFilter(this);
  ui->make->installEventFilter(this);
  ui->modality->installEventFilter(this);
  ui->fieldStrength->installEventFilter(this);
  ui->seriesAttributes->installEventFilter(this);
  ui->model->installEventFilter(this);
  ui->agent->installEventFilter(this);
  ui->thickness->installEventFilter(this);
  ui->kernel->installEventFilter(this);
  ui->kvp->installEventFilter(this);
  ui->mas->installEventFilter(this);
  ui->sequence->installEventFilter(this);

  pipeline->SelectImages();
  ENABLECONTROL(ui->addSeriesButton, addSeriesAction, tr("Press to add a series")); // ok for the add series button to be enabled again
  if (viableSeriesCount > 0) {
    (*menuActionEnabledMap)[specifySeriesAction] = true;
    specifySeriesAction->setEnabled(true);
    owner->getTargetDefine(sessionItemIndex)->enableMenuActions(); // propagate it downstream
  }
  else {
    (*menuActionEnabledMap)[specifySeriesAction] = false;
    specifySeriesAction->setEnabled(false);
    owner->getTargetDefine(sessionItemIndex)->disableMenuActions(); // propagate it downstream
  }
  if (currentBackingSeries != NULL) {
    on_proceedToAnalysisButton_clicked(); // save the user a key click if no need to wait
  }

  preLoading = false;
  gotoSeriesSurveyAction->setEnabled(true);
  seriesSurveyMenu->setEnabled(true);
}

void seriesSurvey::resetWI(ebiVesselPipeline::Pointer p, QList<imageSeries> *imageSeriesSet)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  pipeline = p;
  images = imageSeriesSet; 
}

void seriesSurvey::on_addSeriesButton_clicked() 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (images->size() > 0) {
    QMessageBox msgBox(this);
    msgBox.setText(tr("Adding an additional series is a significant change, resulting in computations on targets being reset."));
    msgBox.setInformativeText(tr("Do you want to add the series anyway, resetting analyses?"));
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    int ret = msgBox.exec();
    if (ret == QMessageBox::No) {
      // return without action except for reverting back to select page
      return; 
    }
  }

  // here either because it is the first series or the user confirms that resetting analyses is OK.  So proceed with the addition.
  DISABLECONTROL(ui->addSeriesButton, addSeriesAction, tr("Can't add another series while one is currently being added")); // first disable the button from being hit again while this is in progress
  ui->proceedToAnalysisButton->setEnabled(false); // likewise the button to proceed
  (*menuActionEnabledMap)[specifySeriesAction] = false;
  specifySeriesAction->setEnabled(false);
  owner->getTargetDefine(sessionItemIndex)->disableMenuActions();
  tools->disableToolButtons(); // in fact, all of them!

  QString seriesFolderAbsolute; 
  QDir seriesFolderDir;
  QString seriesFolder;
  while (true) {
    workItemListEntry *wi = owner->getWorkItem(sessionItemIndex)->selectedWorkItemListEntryPtr;
    if (!wi->setWorkItemFolder()) { // this may be the first time an operation that requires the folder has been done
      message->showMessage(tr("Error: Cannot write work item folder."));
      return;
    }
    QDir::setCurrent(owner->getListDirectory());
    seriesFolderAbsolute=QFileDialog::getExistingDirectory(this, tr("Series Folder"), "..", QFileDialog::ShowDirsOnly); 
    seriesFolderDir = owner->getListDirectory();
    seriesFolder = seriesFolderDir.relativeFilePath(seriesFolderAbsolute);
    if ((seriesFolderDir.isReadable()) && (QDir(seriesFolder).entryList(QDir::Files).count() > 0)) {
        // process the new series (or bypass without further action if user hit cancel)
      if (!seriesFolder.isNull()) {
        emit resetAllTargetsDueToChangesInImages(); // reset all target calculations
        ensureOnSeriesSurveyPage();
        imageSeries *newSeries = new imageSeries(); // set up the new series
        newSeries->progressIndicator = new QProgressDialog(tr("Reading in image header..."), tr("Cancel"), 0, 0, this);
        newSeries->progressIndicator->setWindowModality(Qt::NonModal);
        newSeries->progressIndicator->setMinimumDuration(0);
        newSeries->progressIndicator->setWindowFlags(newSeries->progressIndicator->windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::WindowStaysOnTopHint);
        newSeries->progressIndicator->setCancelButton(nullptr);   // no cancel button on dialog
        newSeries->progressIndicator->show();
        this->repaint(); // ensure progress is shown
        qApp->processEvents();
        newSeries->seriesFolder = seriesFolder;
        QUuid uid = QUuid::createUuid();
        newSeries->token = uid.toString();
        newSeries->seriesType = ""; //"CT"; // default to CT, the user can fix it later if incorrect
        images->insert(0, *newSeries);
        addSeries(0, true, &((*images)[0]));
        emit resetAllTargetsDueToChangesInImages();
        QString step = "seriesSurvey::addSeries"; // log the update
        emit logUpdate(step, newSeries->token, NULL, sessionItemIndex);
      }
      break;
    }
    else {
      QMessageBox msgBox(this);
      msgBox.setText(tr("The folder you selected has no readable files."));
      msgBox.setInformativeText(tr("Do you want to select a different folder? (Yes means select again, No means abort the add)"));
      msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      msgBox.setDefaultButton(QMessageBox::Yes);
      msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
      int ret = msgBox.exec();
      if (ret == QMessageBox::No)  
        break; 
    }
  }

  ENABLECONTROL(ui->addSeriesButton, addSeriesAction, tr("Press to add a(nother) series to the analysis")); // ok for it to be enabled again
  (*menuActionEnabledMap)[specifySeriesAction] = true;
  specifySeriesAction->setEnabled(true);
}

void seriesSurvey::addSeries(int index, bool makeBackingIfViable, imageSeries *series) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // start by setting UID, and defaulting the series viability to false
  QUuid uid = QUuid::createUuid();
  series->token = uid.toString();
  series->isViable = false;

  // this next code block copied from ui_seriesSurvey.cpp as means to establish the screen setup for the new series.
  // If the screen layouts are ammended in the designer tool, this needs to also be changed.
  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ui->workingView->sizePolicy().hasHeightForWidth());
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);

  series->seriesWidget = new QVBoxLayout();
        series->seriesWidget->setSpacing(0);
        series->seriesWidget->setObjectName(QStringLiteral("seriesWidget"));
        series->seriesWidget->setSizeConstraint(QLayout::SetFixedSize);
        series->seriesWidget->setContentsMargins(-1, 0, -1, 0);
        series->seriesLabel = new QLabel(ui->scrollAreaWidgetContents);
        series->seriesLabel->setObjectName(QStringLiteral("seriesLabel"));
        series->seriesLabel->setMaximumSize(QSize(16777215, 16));
        series->seriesLabel->setAutoFillBackground(false);
        series->seriesLabel->setStyleSheet(QStringLiteral("background-color: black"));
        series->seriesLabel->setAlignment(Qt::AlignCenter);

        series->seriesWidget->addWidget(series->seriesLabel);

        series->seriesThumb = new QWidget(ui->scrollAreaWidgetContents);
        series->seriesThumb->setObjectName(QStringLiteral("seriesThumb"));
        sizePolicy.setHeightForWidth(series->seriesThumb->sizePolicy().hasHeightForWidth());
        series->seriesThumb->setSizePolicy(sizePolicy);
        series->seriesThumb->setMinimumSize(QSize(0, 150));
        series->seriesThumb->setMaximumSize(QSize(16777215, 200));
        series->seriesThumb->setStyleSheet(QStringLiteral("background-image: ./MR-MONO2-8-16x-heart.jpg"));

        series->seriesWidget->addWidget(series->seriesThumb);

        series->line_2 = new QFrame(ui->scrollAreaWidgetContents);
        series->line_2->setObjectName(QStringLiteral("line_2"));
        series->line_2->setFrameShape(QFrame::HLine);
        series->line_2->setFrameShadow(QFrame::Sunken);

        series->seriesWidget->addWidget(series->line_2);

        ui->seriesSetBox->insertLayout(index, series->seriesWidget);
 
  // start a thread that will read in the image headers, initialize the metadata, and obtain and display the thumbnail
  //         /*FOR NOW ONLY LOAD CT (or one with blank type, as could be CT)... that is, don't preload MR*/if ((series->seriesType != "CT") && (series->seriesType != "")) return;
  series->imageID = pipeline->OpenImage(series->seriesFolder.toStdString());
  series->image4IDsurvey = viewers->AddImage4(pipeline->GetMultiImageReader(), series->imageID, series->seriesType.toStdString());
  series->makeBackingIfViable = makeBackingIfViable;

  // now get the data and establish the thumbnail
  whenSeriesHeadersIn(series->token);  
  vtkSmartPointer<UpdateSeriesInBackground> updateReaderPixels = vtkSmartPointer<UpdateSeriesInBackground>::New();
  updateReaderPixels->uid = series->token;
  updateReaderPixels->owner = this;
  viewers->AddObserver(ebvReadPixelsFinishedEvent(series->image4IDsurvey),updateReaderPixels);
}

void seriesSurvey::whenSeriesHeadersIn(QString seriesToken) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << seriesToken.toStdString() << std::endl;
  // loop over the set to determine which one this applies to
  imageSeries* series = Q_NULLPTR;
  for (int i=0; i < imageSeriesSet()->size(); i++) {
    if (seriesToken == imageSeriesSet()->at(i).token) {
      series = &((*imageSeriesSet())[i]);
      if (series->progressIndicator) 
        delete series->progressIndicator;
      // initialize the attributes if they are not already set
      if (series->seriesType == "") {
        int j;
        for (j=0; j < ui->seriesType->count(); j++) {
          if (ui->seriesType->itemText(j) == "CT")
            break;
        }
        if (j < ui->seriesType->count()) {
          ui->seriesType->setCurrentIndex(j);
          series->seriesType = ui->seriesType->itemText(j);
          message->showMessage(tr("Warning: Series type is defaulted to CT, please verify and change as necessary."));
        }
        else {
          ui->seriesType->setCurrentIndex(-1);
          series->seriesType = "";
          message->showMessage(tr("Warning: Series type setting is not valid, please reset."));
        }

        series->seriesAttributes = "";
        ui->seriesAttributes->setText(series->seriesAttributes);

        //   initialized from DICOM, but can be overridden by by user:
        series->anatomy = QString::fromStdString(pipeline->GetMultiImageReader()->GetDICOMMetaData(series->imageID, "0018|0015"));
        for (j=0; j < ui->anatomy->count(); j++) {
          if (series->anatomy.contains(ui->anatomy->itemText(j), Qt::CaseInsensitive))
            break;
        }
        if (j < ui->anatomy->count()) {
          ui->anatomy->setCurrentIndex(j);
          series->anatomy = ui->anatomy->itemText(j);
        }
        else {
          ui->anatomy->setCurrentIndex(-1);
          message->showMessage(tr("Warning: Scanned anatomy setting is not valid, please reset."));
        }

        series->make = QString::fromStdString(pipeline->GetMultiImageReader()->GetDICOMMetaData(series->imageID, "0008|0070"));
        for (j=0; j < ui->make->count(); j++) {
          if (series->make.contains(ui->make->itemText(j), Qt::CaseInsensitive))
            break;
        }
        if (j < ui->make->count()) {
          ui->make->setCurrentIndex(j);
          series->make = ui->make->itemText(j);
        }
        else {
          ui->make->setCurrentIndex(-1);
          message->showMessage(tr("Warning: Scanner make setting is not valid, please reset."));
        }

        series->model = QString::fromStdString(pipeline->GetMultiImageReader()->GetDICOMMetaData(series->imageID, "0008|1090"));
        ui->model->setText(series->model);

        series->contrastAgent = QString::fromStdString(pipeline->GetMultiImageReader()->GetDICOMMetaData(series->imageID, "0018|0010"));
        ui->agent->setText(series->contrastAgent);

        series->sliceThickness = QString::fromStdString(pipeline->GetMultiImageReader()->GetDICOMMetaData(series->imageID, "0018|0050"));
        ui->thickness->setText(series->sliceThickness);

        series->modality = QString::fromStdString(pipeline->GetMultiImageReader()->GetDICOMMetaData(series->imageID, "0008|0060"));
        for (j=0; j < ui->modality->count(); j++) {
          if (series->modality.contains(ui->modality->itemText(j), Qt::CaseInsensitive))
            break;
        }
        if (j < ui->modality->count()) {
          ui->modality->setCurrentIndex(j);
          series->modality = ui->modality->itemText(j);
        }
        else {
          ui->modality->setCurrentIndex(-1);
          message->showMessage(tr("Warning: Modality setting is not valid, please reset."));
        }

        series->convolutionKernel = QString::fromStdString(pipeline->GetMultiImageReader()->GetDICOMMetaData(series->imageID, "0018|9315"));
        ui->kernel->setText(series->convolutionKernel);

        series->kvp = QString::fromStdString(pipeline->GetMultiImageReader()->GetDICOMMetaData(series->imageID, "0018|0060"));
        ui->kvp->setText(series->kvp);

        series->mas = QString::fromStdString(pipeline->GetMultiImageReader()->GetDICOMMetaData(series->imageID, "0018|1152"));
        ui->mas->setText(series->mas);

        series->magneticFieldStrength = QString::fromStdString(pipeline->GetMultiImageReader()->GetDICOMMetaData(series->imageID, "0018|0087"));
        for (j=0; j < ui->fieldStrength->count(); j++) {
          if (series->magneticFieldStrength.contains(ui->fieldStrength->itemText(j), Qt::CaseInsensitive))
            break;
        }
        if (j < ui->fieldStrength->count()) {
          ui->fieldStrength->setCurrentIndex(j);
          series->magneticFieldStrength = ui->fieldStrength->itemText(j);
        }
        else if (series->modality == "MR") {
          ui->fieldStrength->setCurrentIndex(-1);
          message->showMessage(tr("Warning: Magnetic Field Strength setting is not valid, please reset."));
        }

        series->acquisitionContrast = QString::fromStdString(pipeline->GetMultiImageReader()->GetDICOMMetaData(series->imageID, "0008|9209"));
        ui->sequence->setText(series->acquisitionContrast);

        //   initialized from DICOM, only informational (not editable by user):
        series->dicomUID = QString::fromStdString(pipeline->GetMultiImageReader()->GetDICOMMetaData(series->imageID, "0020|000d")).trimmed();
        ui->dicomUID->setText(series->dicomUID);
        series->seriesUID = QString::fromStdString(pipeline->GetMultiImageReader()->GetDICOMMetaData(series->imageID, "0020|000e")).trimmed();

        series->acquisitionDate = QString::fromStdString(pipeline->GetMultiImageReader()->GetDICOMMetaData(series->imageID, "0008|0020")); // study date
        ui->dicomDate->setText(series->acquisitionDate);
          
        series->acquisitionTime = QString::fromStdString(pipeline->GetMultiImageReader()->GetDICOMMetaData(series->imageID, "0008|0030")); // study time 
        ui->dicomTime->setText(series->acquisitionTime);

        // if not already set, need to set workitem's applies date based on the image:
        workItemListEntry *wi = owner->getWorkItem(sessionItemIndex)->selectedWorkItemListEntryPtr;
        if (wi->getAppliesDate() == "") {
          if ((series->acquisitionDate.length() != 8) || (series->acquisitionTime.length() < 6))
            message->showMessage(tr("Warning: Image data/time is not valid to set workitem applies date/time, please set manually or add an image with proper date/time."));
          else {
            QString appliesDateString = series->acquisitionDate.left(4); // year
            appliesDateString.append("-");
            appliesDateString.append(series->acquisitionDate.mid(4,2)); // month
            appliesDateString.append("-");
            appliesDateString.append(series->acquisitionDate.right(2)); // day
            appliesDateString.append("T");
            appliesDateString.append(series->acquisitionTime.left(2)); // hour 
            appliesDateString.append(":");
            appliesDateString.append(series->acquisitionTime.mid(2,2)); // minute 
            appliesDateString.append(":");
            appliesDateString.append(series->acquisitionTime.mid(4,2)); // sec 
            wi->setAppliesDate(appliesDateString);
          }
        } 

      } // end-if series->seriesType == ""
      else // series type is non-blank, which indicates the properties are to be read in rather than initially set
        displaySeriesPropertyValues(series);
      connectSeriesProperties(series);
      
      series->isViable = true;
      viableSeriesCount++;
      if (series->makeBackingIfViable)
        // make this series be the backing series
        selectBackingSeries(series);  

      // set up an event filter for the new series, save it in the series set, and finish
      series->seriesThumb->installEventFilter(this);
      series->seriesLabel->setText(series->seriesType);
      ui->seriesSelectionArea->setCurrentWidget(ui->propertiesPage);

      // and now initialize the meta data and add this to selected images
      series->pushSeriesParametersToPipeline(pipeline->GetMultiImageReader());
      pipeline->SelectImages();
      std::string v = pipeline->VerifyImageQuality();
      if (!v.empty())
        message->showMessage(QString::fromStdString(v));
      return; 
    }
  }
}

void seriesSurvey::selectBackingSeries(imageSeries *series)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // set the previous image not visible
  if (currentBackingSeries != NULL)
    viewers->GetScene()->GetImage4(backingImageId)->SetVisibility(false);

  // establish the new backing as visbile
  currentBackingSeries = series;
  if (currentBackingSeries != NULL) {
    backingImageId = series->image4IDsurvey;
    viewers->GetScene()->GetImage4(backingImageId)->SetVisibility(true);
    viewers->GetScene()->SetSelectedImage4(backingImageId);
    if (currentBackingSeries->seriesType == "CT") {
      viewers->GetScene()->GetImage4(backingImageId)->SetIntensityUnitString("HU");
      viewers->GetScene()->GetSelectedImage4()->SetCTAColorAndOpacity();
    }
    else if (currentBackingSeries->seriesType == "MRA") {
      viewers->GetScene()->GetSelectedImage4()->SetCEMRAColorAndOpacity();
    }
    else {
      viewers->GetScene()->GetSelectedImage4()->SetCTAColorAndOpacity();
    }
    viewers->SetScreenScaleFitFactor2D(1.0);
    viewers->OptimizeScreenScale2D();
    if ((thisProduct == "vascuCAP") && (series->seriesType == "CT"))
      viewers->GetScene()->GetSelectedImage4()->SetWindowLevel(800, 200);
    else
      viewers->GetScene()->AutoWindowLevel();
    viewers->GetScene()->GetSelectedImage4()->GetWindowLevel(currentBackingSeries->window, currentBackingSeries->level);
    viewers->GetScene()->SetCursorPositionToSceneCenter();
    viewers->UpdateCameras();
    viewers->PanCamerasToCursor(true, true);
    
    QString label = series->seriesType;
    label.append(tr(" in folder "));
    label.append(series->seriesFolder);
    ui->workingViewLabel->setText(label);
    label = tr("is a ");
    label.append(series->modality);
    label.append(tr(" of the "));
    label.append(series->anatomy);
    label.append(tr(" acquired on "));
    label.append(series->acquisitionDate);
    ui->orthogonalView1Label->setText(label);
    label = tr("(right click thumbnail for more properties)");
    ui->orthogonalView2Label->setText(label);

    tools->setBackingImageId(backingImageId);

    currentBackingSeries->pushSeriesParametersToPipeline(pipeline->GetMultiImageReader());
    enableMenuActions();
  }
  else {
    QString label = tr("(no current backing series)");
    ui->workingViewLabel->setText(label);
    label = tr("(click Add Series to add one)");
    ui->orthogonalView1Label->setText(label);
    label = "";
    ui->orthogonalView2Label->setText(label);
    tools->disableToolButtons();
  }

  this->repaint(); // ensure labels are updated
  qApp->processEvents();
  viewers->SyncViewersToScene();
  viewers->InitializeCameras();
  viewers->Render();
}

void seriesSurvey::connectSeriesProperties(imageSeries *series)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  connect(ui->seriesType, SIGNAL(activated(QString)), this, SLOT(onSeriesType(QString)));
  connect(ui->anatomy, SIGNAL(activated(QString)), this, SLOT(onAnatomy(QString)));
  connect(ui->make, SIGNAL(activated(QString)), this, SLOT(onMake(QString)));
  connect(ui->modality, SIGNAL(activated(QString)), this, SLOT(onModality(QString)));
  connect(ui->fieldStrength, SIGNAL(activated(QString)), this, SLOT(onFieldStrength(QString)));
  connect(ui->seriesAttributes, SIGNAL(textEdited(QString)), this, SLOT(onSeriesAttributes(QString)));
  connect(ui->model, SIGNAL(textEdited(QString)), this, SLOT(onModel(QString)));
  connect(ui->agent, SIGNAL(textEdited(QString)), this, SLOT(onAgent(QString)));
  connect(ui->thickness, SIGNAL(textEdited(QString)), this, SLOT(onThickness(QString)));
  connect(ui->kernel, SIGNAL(textEdited(QString)), this, SLOT(onKernel(QString)));
  connect(ui->kvp, SIGNAL(textEdited(QString)), this, SLOT(onKVP(QString)));
  connect(ui->mas, SIGNAL(textEdited(QString)), this, SLOT(onMAS(QString)));
  connect(ui->sequence, SIGNAL(textEdited(QString)), this, SLOT(onSequence(QString)));
}

void seriesSurvey::disconnectSeriesProperties()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  disconnect(ui->seriesType, SIGNAL(activated(QString)), this, SLOT(onSeriesType(QString)));
  disconnect(ui->anatomy, SIGNAL(activated(QString)), this, SLOT(onAnatomy(QString)));
  disconnect(ui->make, SIGNAL(activated(QString)), this, SLOT(onMake(QString)));
  disconnect(ui->modality, SIGNAL(activated(QString)), this, SLOT(onModality(QString)));
  disconnect(ui->fieldStrength, SIGNAL(activated(QString)), this, SLOT(onFieldStrength(QString)));
  disconnect(ui->seriesAttributes, SIGNAL(textEdited(QString)), this, SLOT(onSeriesAttributes(QString)));
  disconnect(ui->model, SIGNAL(textEdited(QString)), this, SLOT(onModel(QString)));
  disconnect(ui->agent, SIGNAL(textEdited(QString)), this, SLOT(onAgent(QString)));
  disconnect(ui->thickness, SIGNAL(textEdited(QString)), this, SLOT(onThickness(QString)));
  disconnect(ui->kernel, SIGNAL(textEdited(QString)), this, SLOT(onKernel(QString)));
  disconnect(ui->kvp, SIGNAL(textEdited(QString)), this, SLOT(onKVP(QString)));
  disconnect(ui->mas, SIGNAL(textEdited(QString)), this, SLOT(onMAS(QString)));
  disconnect(ui->sequence, SIGNAL(textEdited(QString)), this, SLOT(onSequence(QString)));
}

void seriesSurvey::displaySeriesPropertyValues(imageSeries *series)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // verify the properites which need to conform to specific values
  int j;
  for (j=0; j < ui->seriesType->count(); j++) {
    if (series->seriesType == ui->seriesType->itemText(j))
      break;
  }
  if (j < ui->seriesType->count())
    ui->seriesType->setCurrentIndex(j);
  else
    ui->seriesType->setCurrentIndex(-1);

  for (j=0; j < ui->anatomy->count(); j++) {
    if (series->anatomy == ui->anatomy->itemText(j))
      break;
  }
  if (j < ui->anatomy->count())
    ui->anatomy->setCurrentIndex(j);
  else
    ui->anatomy->setCurrentIndex(-1);

  for (j=0; j < ui->make->count(); j++) {
    if (series->make == ui->make->itemText(j))
      break;
  }
  if (j < ui->make->count())
    ui->make->setCurrentIndex(j);
  else
    ui->make->setCurrentIndex(-1);

  for (j=0; j < ui->modality->count(); j++) {
    if (series->modality == ui->modality->itemText(j))
      break;
  }
  if (j < ui->modality->count())
    ui->modality->setCurrentIndex(j);
  else
    ui->modality->setCurrentIndex(-1);

  for (j=0; j < ui->fieldStrength->count(); j++) {
    if (series->magneticFieldStrength == ui->fieldStrength->itemText(j))
      break;
  }
  if (j < ui->fieldStrength->count())
    ui->fieldStrength->setCurrentIndex(j);
  else
    ui->fieldStrength->setCurrentIndex(-1);

  // and now just echo the free-form properties
  ui->seriesAttributes->setText(series->seriesAttributes);
  ui->model->setText(series->model);
  ui->agent->setText(series->contrastAgent);
  ui->thickness->setText(series->sliceThickness);
  ui->kernel->setText(series->convolutionKernel);
  ui->kvp->setText(series->kvp);
  ui->mas->setText(series->mas);
  ui->sequence->setText(series->acquisitionContrast); 
  ui->dicomUID->setText(series->dicomUID); 
  ui->dicomDate->setText(series->acquisitionDate); 
  ui->dicomTime->setText(series->acquisitionTime); 
}

void seriesSurvey::whenSeriesAllIn(QString seriesToken) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // loop over the set to determine which one this applies to
  imageSeries* series = Q_NULLPTR;
  for (int i=0; i < imageSeriesSet()->size(); i++) {
    if (seriesToken == imageSeriesSet()->at(i).token) {
      series = &((*imageSeriesSet())[i]);
      // Display the thumb
      QString thumbFile = imageSeriesSet()->at(i).seriesThumbFile;
      if (thumbFile == "") {
        // the thumb file doesn't exist yet; make it
        // setting the folder is more involved due to the need to carefully construct it
        workItemListEntry *wi = owner->getWorkItem(sessionItemIndex)->selectedWorkItemListEntryPtr;
        if (!wi->setWorkItemFolder()) { // this may be the first time an operation that requires the folder has been done
          message->showMessage(tr("Error: Cannot write work item folder."));
          return;
        }
        QStringList seriesFolderLevels = imageSeriesSet()->at(i).seriesFolder.split("/");
        thumbFile = wi->getWorkItemFolder();
        thumbFile.append("/");
        thumbFile.append(seriesFolderLevels.at(seriesFolderLevels.length()-1)); // take the last part
        thumbFile.append(".png");
        SaveThumbnailImage(pipeline->GetMultiImageReader()->GetImage<float, 3>(series->imageID), thumbFile.toStdString(), series->seriesThumb->width(), series->seriesThumb->height());
        QString relativeThumbFile = QDir::current().relativeFilePath(thumbFile);
        series->seriesThumbFile = relativeThumbFile;
      }
      QString thumbStyle = "border-image: url(";
      thumbStyle.append(thumbFile);
      thumbStyle.append(")");
      series->seriesThumb->setStyleSheet(thumbStyle);
      this->repaint(); // ensure labelse are updated
      qApp->processEvents();
      ui->proceedToAnalysisButton->setEnabled(true); 
      (*menuActionEnabledMap)[specifySeriesAction] = true;
      specifySeriesAction->setEnabled(true);
      owner->getTargetDefine(sessionItemIndex)->enableMenuActions(); // propagate it downstream
      if (preLoading && (series == currentBackingSeries)) {
        if (ui->addSeriesButton->isEnabled())
          on_proceedToAnalysisButton_clicked(); // save the user a key click if no need to wait
      }
      tools->enableToolButtons(); // all can be enabled...
      return;
    } // end-if this is the one
  } // end-for each in the set
}

void seriesSurvey::on_doneWithPropertiesButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  disconnectSeriesProperties();
  ui->seriesSelectionArea->setCurrentWidget(ui->selectPage);
}

void seriesSurvey::removeSeries() 
{ 
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QAction *act = qobject_cast<QAction *>(sender());
  if (act != 0) {
    QVariant data = act->data();
    for (int i=0; i < imageSeriesSet()->size(); i++) {
      if (data == imageSeriesSet()->at(i).token) {
        QMessageBox msgBox(this);
        msgBox.setText(tr("Removing a series is a significant change, resulting in computations on targets being reset."));
        msgBox.setInformativeText(tr("Do you want to remove the series anyway, causing any unsaved changes to be lost?"));
        msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
        int ret = msgBox.exec();
        if (ret == QMessageBox::No) {
          return;
        }
        // ok, user has cofirmed removal.  First, remove from selector
        ensureOnSeriesSurveyPage();
        imageSeries *series = &((*imageSeriesSet())[i]);
        // find and remove it from the series selector
        int j;
        for (j=0; j < ui->seriesSetBox->count(); j++) {
          if (ui->seriesSetBox->itemAt(j) == series->seriesWidget) {
            ui->seriesSetBox->takeAt(j);
            delete series->seriesLabel;
            delete series->seriesThumb;
            delete series->line_2;
            delete series->seriesWidget;
            break;
          }
        }
        if (series->isViable)
            --viableSeriesCount;
        this->repaint(); // make sure the page is shown
        qApp->processEvents();
        // save some of the data which we'll need later but which get deleted before we need them
        QString seriesToken = series->token;
        ebID image4IDsurvey = series->image4IDsurvey;
        ebID imageID = series->imageID;
        imageSeriesSet()->removeAt(i);
        // need to handle the case where the deleted one is the current one
        if (series == currentBackingSeries) {
          int j;
          for (j=0; j < imageSeriesSet()->size(); j++) {
            if (imageSeriesSet()->at(j).isViable)
              break;
          }
          if (j < imageSeriesSet()->size()) {
            // good news!  we have another viable one.  Make it the new current backing
            message->showMessage(tr("Current backing series reset, please verify that it is the one you want."));
            selectBackingSeries(&((*imageSeriesSet())[j]));
            emit backingSeriesChanged(&((*imageSeriesSet())[j])); // flush information forward if needed
          }
          else {
            // no other viable series to make current.  Prompt user to make one 
            ui->proceedToAnalysisButton->setEnabled(false);
            owner->getTargetDefine(sessionItemIndex)->disableMenuActions();
            (*menuActionEnabledMap)[specifySeriesAction] = false;
            specifySeriesAction->setEnabled(false);
            message->showMessage(tr("Warning: No other viable series, please add or specify one to enable analysis."));
            selectBackingSeries(NULL);
            emit backingSeriesChanged(NULL);
          }
        } // end-if the one being deleted had been the current backing
        viewers->RemoveImage4(image4IDsurvey);
        pipeline->GetMultiImageReader()->CloseImage(imageID);
        emit resetAllTargetsDueToChangesInImages();
        pipeline->SelectImages();
      	QString step = "seriesSurvey::removeSeries"; // log the update
        emit logUpdate(step, seriesToken, NULL, sessionItemIndex);
      } // end-if this is the thumb
    } // end-for each of our thumbs
  } // end-if action has a sender() 
  
}

void seriesSurvey::specifySeries() 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QAction *act = qobject_cast<QAction *>(sender());
  if (act != 0) {
    QVariant data = act->data();
    for (int i=0; i < imageSeriesSet()->size(); i++) {
      if (data == imageSeriesSet()->at(i).token) {
        ensureOnSeriesSurveyPage();
        if (currentBackingSeries != &((*imageSeriesSet())[i]))
          selectBackingSeries(&((*imageSeriesSet())[i])); // must select this series when specifying
        specifySeriesCore();
      } // end-if this is the thumb
    } // end-for each of our thumbs
  } // end-if action has a sender() 
}

void seriesSurvey::specifySeriesCore() 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // display the properties for the current backing series
  displaySeriesPropertyValues(currentBackingSeries);
  connectSeriesProperties(currentBackingSeries);
  ui->seriesSelectionArea->setCurrentWidget(ui->propertiesPage);
  ui->seriesSelectionArea->setFocus(); // to ensure focus is not on any of the line edits
}

bool seriesSurvey::confirmPropertyChange() 
{ 
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  /*QMessageBox msgBox(this);
  msgBox.setText(tr("Changing the properties of a series is a significant change, resulting in computations on targets being reset."));
  msgBox.setInformativeText(tr("Do you want to change the property anyway, resetting analyses?"));
  msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
  int ret = msgBox.exec();
  if (ret == QMessageBox::No) {
    // return without action except for reverting back to select page
    ui->seriesSelectionArea->setCurrentWidget(ui->selectPage); // revert to selectPage, effectively canceling the change
    return true; // true in this context means that the filter will stop it, not pass it on
  }
  else {
    return false; // false in this context means that the filter will pass it on to the original widget, i.e., the user really wants to change it
  }*/
  message->showMessage(tr("Changing the properties of a series is a significant change, resulting in computations on targets being reset."));
  return false; // false in this context means that the filter will pass it on to the original widget, i.e., the user really wants to change it
}

void seriesSurvey::on_proceedToAnalysisButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  this->setEnabled(false);
  if (viableSeriesCount) {
    owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
    viewers->GetScene()->GetSelectedImage4()->GetWindowLevel(currentBackingSeries->window, currentBackingSeries->level);
    emit giveScreenControlToDefineFromSurvey(ui->seriesSelectionArea, currentBackingSeries);
  } 
  else 
    message->showMessage(tr("Warning: Need at least one viable series to proceed with analysis."));
  this->setEnabled(true);
}

bool seriesSurvey::eventFilter(QObject *obj, QEvent *event) 
{ 
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // first see if the event is associated with one of the thumbnails
  if (images != NULL) {
    for (int i=0; i < imageSeriesSet()->size(); i++) {
      if ((obj == imageSeriesSet()->at(i).seriesThumb) && (event->type() == QEvent::MouseButtonRelease)) {
        if (((QMouseEvent*)event)->button() == Qt::LeftButton) {
          selectBackingSeries(&((*imageSeriesSet())[i]));
          emit backingSeriesChanged(&((*imageSeriesSet())[i])); // flush information forward if needed
          return true;
        }
        else if (((QMouseEvent*)event)->button() == Qt::RightButton) {
          QVariant uid;
          uid.setValue((&((*imageSeriesSet())[i]))->token);
          QAction *specifyAct = new QAction(tr("Specify this series"), this);
          QAction *removeAct = new QAction(tr("Remove this series from the analysis"), this);
          QMenu menu(this);
          specifyAct->setData(uid);
          menu.addAction(specifyAct);
          connect(specifyAct, SIGNAL(triggered()), this, SLOT(specifySeries()));
          removeAct->setData(uid);
          menu.addAction(removeAct);
          connect(removeAct, SIGNAL(triggered()), this, SLOT(removeSeries()));
          menu.exec(((QMouseEvent*)event)->globalPos());
          return true;
        }
      } // end-if this one's for one of our thumbs and its a mouse release
    } // end-for each of our series' thumbs
  }

  // reach here because it wasn't any of the thumbs, so now check if it was one of the property values
  if (((obj == ui->seriesType) 
      || (obj == ui->anatomy)
      || (obj == ui->make)
      || (obj == ui->modality)
      || (obj == ui->fieldStrength)
      || (obj == ui->seriesAttributes)
      || (obj == ui->model)
      || (obj == ui->agent)
      || (obj == ui->thickness)
      || (obj == ui->kernel)
      || (obj == ui->kvp)
      || (obj == ui->mas)
      || (obj == ui->sequence))
    && (event->type() == QEvent::MouseButtonPress)) {
    if (((QMouseEvent*)event)->button() == Qt::LeftButton) {
      return confirmPropertyChange();
    }
  }

  // reach here because it wasn't one of the properties either, so indicate it wasn't an event we are looking for in this filter
  return false;
}
