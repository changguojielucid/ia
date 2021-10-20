// Copyright (c) Elucid Bioimaging

#include "ebAssert.h"
#include <QToolButton>
#include <QTableWidget>
#include <QDesktopWidget>
#include <QLabel>
#include <QMessageBox>

#include <string.h>
#include <iostream>

#include "patientAnalyze.h"
#include "ui_patientAnalyze.h"
#include "cap.h"
#include "ebvOutputWindow.h"

/**
 * \ingroup patientAnalyze
 * @{
 *
 * See patientAnalyze.h for description of the package purpose and contents.  This file has the member functions for classes in the package.
 */

readingsSelector::readingsSelector(QWidget *owner, QString m, vtkSmartPointer<ebvLinkedViewers2> v, ebID c) :
  QWidget(owner)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  measurandCategory = m;
  if (measurandCategory == tr("Structure")) {
    availableMeasurands = QList<QString>()
     << tr("Lumen")
     << tr("LumenAndWall")
     << tr("Wall")
     ;
    availableScaleBases = QList<QString>()
     << tr("Area")
     << tr("AreaRatio")
     << tr("Ecc")
     << tr("EccRatio")
     << tr("MaD")
     << tr("MaDRatio")
     << tr("MeD")
     << tr("MeDRatio")
     << tr("MiD")
     << tr("MiDRatio")
     ;
  } 
  else if (measurandCategory == tr("Composition")) {
    availableMeasurands = QList<QString>()
      << tr("CALC")
      << tr("LRNC")
      << tr("IPH")
      << tr("MATX")
      << tr("PVAT")
      ;
    
    availableScaleBases = QList<QString>()
     << tr("Area")
     << tr("AreaProp")
     ;
  } 
  else {
    qWarning() << "Measurand category" << measurandCategory << "not recognized, skipping.";
    return;
  }

  viewers = v;
  chartId = c;

  Dialog = new QDialog(owner);
  Dialog->setObjectName(QStringLiteral("readingsSelector"));
  QString title = tr("Selector for ");
  title.append(measurandCategory);
  title.append(tr(" graph"));
  Dialog->setWindowTitle(title);
  Dialog->resize(326, 128);
  QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  sizePolicy.setHorizontalStretch(0);
  sizePolicy.setVerticalStretch(0);
  sizePolicy.setHeightForWidth(Dialog->sizePolicy().hasHeightForWidth());
  Dialog->setSizePolicy(sizePolicy);
  gridLayout_3 = new QGridLayout(Dialog);
  gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));

  measurandsBox = new QGroupBox(Dialog);
  measurandsBox->setTitle(tr("Measurand"));
  measurandsBox->setObjectName(QStringLiteral("groupBox"));
  sizePolicy.setHeightForWidth(measurandsBox->sizePolicy().hasHeightForWidth());
  measurandsBox->setSizePolicy(sizePolicy);
  measurands = new QVBoxLayout(measurandsBox);
  measurands->setObjectName(QStringLiteral("gridLayout_2"));

  for (int i=0; i < availableMeasurands.size(); i++) {
    QCheckBox *checkBox = new QCheckBox(measurandsBox);
    checkBox->setText(availableMeasurands.at(i));
    checkBox->setObjectName(QStringLiteral("measurand"));
    if ((availableMeasurands.at(i) == "Lumen") || (availableMeasurands.at(i) == "Wall")) // defaulted on
      checkBox->setChecked(true);
    if ((availableMeasurands.at(i) == "CALC") || (availableMeasurands.at(i) == "MATX") || (availableMeasurands.at(i) == "IPH") || (availableMeasurands.at(i) == "LRNC") || (availableMeasurands.at(i) == "PVAT")) // defaulted on
      checkBox->setChecked(true);
    measurands->addWidget(checkBox, i, 0);
  }
  measurands->addStretch(1);
  gridLayout_3->addWidget(measurandsBox, 0, 0, 1, 1);

  scaleBaseBox = new QGroupBox(Dialog);
  scaleBaseBox->setTitle(tr("Scale Basis"));
  scaleBaseBox->setObjectName(QStringLiteral("groupBox_2"));
  sizePolicy.setHeightForWidth(scaleBaseBox->sizePolicy().hasHeightForWidth());
  scaleBaseBox->setSizePolicy(sizePolicy);
  scaleBases = new QVBoxLayout(scaleBaseBox);
  scaleBases->setObjectName(QStringLiteral("gridLayout"));

  for (int i=0; i < availableScaleBases.size(); i++) {
    QRadioButton *radioButton = new QRadioButton(scaleBaseBox);
    radioButton->setText(availableScaleBases.at(i));
    radioButton->setObjectName(QStringLiteral("scale basis"));
    if (availableScaleBases.at(i) == "Area") // defaulted on
      radioButton->setChecked(true);
    scaleBases->addWidget(radioButton, i, 0);
  }
  scaleBases->addStretch(1);
  gridLayout_3->addWidget(scaleBaseBox, 0, 1, 1, 1);

  buttonBox = new QDialogButtonBox(Dialog);
  buttonBox->setObjectName(QStringLiteral("buttonBox"));
  buttonBox->setOrientation(Qt::Horizontal);
  buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

  gridLayout_3->addWidget(buttonBox, 1, 0, 1, 2);
  
  QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(acceptReadingsSelection()));
  QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(cancelReadingsSelection()));
  QMetaObject::connectSlotsByName(Dialog);
}

void readingsSelector::setTargetId(ebID t)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  targetId = t;
  acceptReadingsSelection();
}

void readingsSelector::presentDialog()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  Dialog->show();
}

void readingsSelector::acceptReadingsSelection()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QString scaleBaseSuffix;
  for (int i=0; i < availableScaleBases.size(); i++) {
    QRadioButton *scaleBase = (QRadioButton *) scaleBases->itemAt(i)->widget();
    if (scaleBase->isChecked()) {
      scaleBaseSuffix = availableScaleBases.at(i);
      break; // only one will be true
    }
  }
  std::vector<std::string> readings;
  for (int i=0; i < availableMeasurands.size(); i++) {
    QCheckBox *measurand = (QCheckBox *) measurands->itemAt(i)->widget();
    if (measurand->isChecked()) {
      QString readingAbbreviation = availableMeasurands.at(i);
      readingAbbreviation.append(scaleBaseSuffix);
      readings.push_back(readingAbbreviation.toStdString());
    }
  }
  viewers->GetViewerChart(chartId)->SetReadings(readings);
  viewers->SyncViewersToScene();
  viewers->Render();
  Dialog->hide();
}

void readingsSelector::cancelReadingsSelection()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // just take the dialog down (the user may have just wanted to review the settings, or otherwise decided not to change them)
  Dialog->hide();
}

patientAnalyze::patientAnalyze(QWidget *p, QMenu *m, bool masterWithRespectToMenu) :
  QWidget(p),
  ui(new Ui::patientAnalyze)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner = dynamic_cast<cap *>(p);
  patientAnalyzeMenu = m;
  currentBackingSeries = NULL;
  currentTarget = NULL;
  viewers = NULL;
  images = NULL;
  targets = NULL;
  structureSel = NULL;
  compositionSel = NULL;
  newTargetReadingsFlag = false;
  newLesionReadingsFlag = false;
  markingLesionFlag = false;

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
    message->setWindowFlags(message->windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    message->setModal(true);
    ui->backToDefineButton->setText(QChar(0x25C0));
          QString buttonText = tr("Proceed to Report ");
          buttonText.append(QChar(0x25B6));
    ui->proceedToReportButton->setText(buttonText);
    ui->proceedToReportButton->setEnabled(false);

    // remove the placeholder series selection pages from the designer (as a reverse of the steps to add)
    ui->seriesSelectionArea->removeWidget(ui->seriesSelectionArea->widget(1));
    ui->seriesSelectionArea->removeWidget(ui->seriesSelectionArea->widget(0));

    // set up the linked viewers
    viewers = vtkSmartPointer<ebvLinkedViewers2>::New();
    id3d = viewers->AddViewer(ebvViewer::THREED,ui->summaryAreaRender0->GetRenderWindow());
    viewers->GetViewer(id3d)->SetShowLogo(true);
    // planes set for initial condition of oblique OFF
    viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::AXIAL, true);
    viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::CORONAL, true);
    viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::SAGITTAL, true);
    viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::OBLIQUE, false);
    viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
    viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::CPR, false);
    viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::OBLIQUEPERP2, false);
    
    ebvIdSummaryAreaGraph1 = viewers->AddViewer(ebvViewer::CHART, ui->summaryAreaGraph1->GetRenderWindow());
    ebvIdSummaryAreaGraph2 = viewers->AddViewer(ebvViewer::CHART, ui->summaryAreaGraph2->GetRenderWindow());
    sliceAreaRender0Type = ebvViewer::AXIAL;
    ebvIdSliceAreaRender0 = viewers->AddViewer(sliceAreaRender0Type,ui->sliceAreaRender0->GetRenderWindow());
    viewers->GetViewer(ebvIdSliceAreaRender0)->SetShowLogo(true);
    sliceAreaRender1Type = ebvViewer::CORONAL;
    ebvIdSliceAreaRender1 = viewers->AddViewer(sliceAreaRender1Type,ui->sliceAreaRender1->GetRenderWindow());
    viewers->GetViewer(ebvIdSliceAreaRender1)->SetShowLogo(true);
    sliceAreaRender2Type = ebvViewer::SAGITTAL;
    ebvIdSliceAreaRender2 = viewers->AddViewer(sliceAreaRender2Type,ui->sliceAreaRender2->GetRenderWindow());
    viewers->GetViewer(ebvIdSliceAreaRender2)->SetShowLogo(true);

    viewers->GetViewer3D(id3d)->SetShowSceneCornerAnnotations(true);
    viewers->GetViewer2D(ebvIdSliceAreaRender0)->SetShowSceneCornerAnnotations(true);
    viewers->GetViewer2D(ebvIdSliceAreaRender1)->SetShowSceneCornerAnnotations(false);
    viewers->GetViewer2D(ebvIdSliceAreaRender2)->SetShowSceneCornerAnnotations(false);
    addViewersLegend();
    viewers->SyncViewersToScene();
    viewers->SetScreenScaleFitFactor2D(1.0);
    viewers->OptimizeScreenScale2D();
    
    vtkOutputWindow::SetInstance(ebvOutputWindow::New());

    // set up traps for the small viewports 
    ui->summaryAreaGraph1->installEventFilter(this);
    ui->summaryAreaGraph2->installEventFilter(this);
    ui->sliceAreaRender1->installEventFilter(this);
    ui->sliceAreaRender2->installEventFilter(this);

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
  } // end-if owner isn't 0
  else {
    // this is needed to make the dummy object to establish the menu have a place to add the tool buttons (which won't be used, but need for resusing fucntion)
    ui->summaryAreaControlsBox = new QWidget(0);
    ui->sliceAreaControlsBox = new QWidget(0);
  }
}

patientAnalyze::~patientAnalyze()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (owner != 0) {
    viewers->RemoveViewer(id3d);
    viewers->RemoveViewer(ebvIdSummaryAreaGraph1);
    viewers->RemoveViewer(ebvIdSummaryAreaGraph2);
    viewers->RemoveViewer(ebvIdSliceAreaRender0);
    viewers->RemoveViewer(ebvIdSliceAreaRender1);
    viewers->RemoveViewer(ebvIdSliceAreaRender2);

    // take down any of the composition controls that would otherwise be orphaned
    if (targets) {
      for (int i=0; i < targets->size(); i++) {
        targetDef *def = &((*targets)[i]);
        if (def->parameters != NULL) {
          if (def->parameters->compositionControlCurrentlyInUse)
            if (def->parameters->compositionControl != NULL)
              def->parameters->compositionControl->cancelCompositionSetting();
        }
      }
    }
    /*images = NULL;
    targets = NULL;
    viewers = NULL;
    delete message;*/
    delete ui; 
  }
}

void patientAnalyze::on_proceedToReportButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  dismissCompositionControls();

  // workItem will do all the work, just signal it to do so
  emit packageDataAndTransitionToReport();
}

void patientAnalyze::enableControlsAsAppropriateWhenNoTargetFocus()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // in this situation, most summary controls need to be disabled, with exceptions enabled, however, the converse is true for the slice controls.
  // as a result, the approach is to do the group and handle the exceptions, as simplest in each respective case.
  
  // summary: start by disabling all, then enable the exceptions
  summaryTools->disableToolButtons();    
  ENABLECONTROL(summaryTools->ToggleCoronalTool, summaryTools->ToggleCoronalAction, tr("Press to toggle coronal plane"));
  ENABLECONTROL(summaryTools->ToggleSagittalTool, summaryTools->ToggleSagittalAction, tr("Press to toggle sagittal plane"));
  ENABLECONTROL(summaryTools->ToggleAxialTool, summaryTools->ToggleAxialAction, tr("Press to toggle axial plane"));
  ENABLECONTROL(summaryTools->AnnotateTool, summaryTools->AnnotateAction, tr("Press to annotate"));
  summaryTools->AnnotateTool->setChecked(false);
  ENABLECONTROL(summaryTools->ShowAsMovieTool, summaryTools->ShowAsMovieAction, tr("Press to show as movie"));
    
  // slice: start by enabling all, then disable the exceptions
  sliceTools->enableToolButtons();  
  DISABLECONTROL(sliceTools->ToggleCornerTool, sliceTools->ToggleCornerAction, tr("ToggleCorner is disabled (select a target to enable it)"));
  sliceTools->ToggleCornerTool->setChecked(false);
  DISABLECONTROL(sliceTools->TargetDeleteTool, sliceTools->TargetDeleteAction, tr("TargetDelete is disabled (select a target to enable it)"));
  DISABLECONTROL(sliceTools->TargetModifyTool, sliceTools->TargetModifyAction, tr("TargetModify is disabled (select a target to enable it)"));
  sliceTools->TargetModifyTool->setChecked(false);
  DISABLECONTROL(sliceTools->SaveToReportTool, sliceTools->SaveToReportAction, tr("Save to Report is disabled (select a target to enable it)"));
  DISABLECONTROL(sliceTools->MoveProximalTool, sliceTools->MoveProximalAction, tr("MoveProximal is disabled (select a target to enable it)"));
  DISABLECONTROL(sliceTools->MoveDistalTool, sliceTools->MoveDistalAction, tr("MoveDistal is disabled (select a target to enable it)"));
}

void patientAnalyze::enableControlsAsAppropriateGivenATargetFocus()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // in this case, we ensure that those explcitly disabled when there is no target focus are now enabled. This need not be the complete set,
  // as a number are already enabled regardless of a current target focus. I.e., just enable the ones that aren't already enabled.
  ENABLECONTROL(summaryTools->MaxStenosisTool, summaryTools->MaxStenosisAction, tr("Press to snap to cross section of greatest stenosis"));
  ENABLECONTROL(summaryTools->MaxDilationTool, summaryTools->MaxDilationAction, tr("Press to snap to cross section of greatest dilation"));
  ENABLECONTROL(summaryTools->MaxRRTool, summaryTools->MaxRRAction, tr("Press to snap to cross section of greatest remodeling ratio"));
  ENABLECONTROL(summaryTools->MaxWTTool, summaryTools->MaxWTAction, tr("Press to snap to cross section of greatest wall thickness"));
  ENABLECONTROL(summaryTools->MaxCalcTool, summaryTools->MaxCalcAction, tr("Press to snap to cross section of maximum calcification"));
  ENABLECONTROL(summaryTools->MaxLRNCTool, summaryTools->MaxLRNCAction, tr("Press to snap to cross section of maximum lipid core"));
  ENABLECONTROL(summaryTools->MaxIPHTool, summaryTools->MaxIPHAction, tr("Press to snap to cross section of maximum hemorrhage"));
  ENABLECONTROL(summaryTools->MaxMATXTool, summaryTools->MaxMATXAction, tr("Press to snap to cross section of maximum matrix"));
  ENABLECONTROL(summaryTools->MaxUlcTool, summaryTools->MaxUlcAction, tr("Press to snap to cross section of maximum ulceration"));
  ENABLECONTROL(summaryTools->MaxThrTool, summaryTools->MaxThrAction, tr("Press to snap to cross section of maximum thrombosis"));
  ENABLECONTROL(summaryTools->SaveToReportTool, summaryTools->SaveToReportAction, tr("Press to save view as a key image at the target level"));  
  ENABLECONTROL(sliceTools->ToggleCornerTool, sliceTools->ToggleCornerAction, tr("Press to toggle corner annotations"));
  sliceTools->ToggleCornerTool->setChecked(true);
  ENABLECONTROL(sliceTools->TargetDeleteTool, sliceTools->TargetDeleteAction, tr("Press to delete current target"));
  ENABLECONTROL(sliceTools->TargetModifyTool, sliceTools->TargetModifyAction, tr("Press to modify current target"));
  sliceTools->TargetModifyTool->setChecked(false);
  ENABLECONTROL(sliceTools->MoveProximalTool, sliceTools->MoveProximalAction, tr("Press to move proximally"));
  ENABLECONTROL(sliceTools->MoveDistalTool, sliceTools->MoveDistalAction, tr("Press to move distally"));
  ENABLECONTROL(sliceTools->SaveToReportTool, sliceTools->SaveToReportAction, tr("Press to save view as a key image at the cross-section level"));  
}

void patientAnalyze::addViewersLegend()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // display legend box
  auto viewer = viewers->GetViewer(ebvIdSliceAreaRender0);
  ebAssert(viewer);
  ebAssert(viewer->GetLegendBox());
  ebAssert(viewer->GetBalloonWidget());
  viewer->SetShowLegendBox(true);
  auto dict = viewers->GetScene()->GetColorDictionary();
  // NOTE: size of image doesn't matter, gets scaled by legend box actor
  vtkSmartPointer<vtkImageData> calcImage = NewImageData(VTK_UNSIGNED_CHAR,3,20,20);
  vtkSmartPointer<vtkImageData> lrncImage = NewImageData(VTK_UNSIGNED_CHAR,3,20,20);
  vtkSmartPointer<vtkImageData> matxImage = NewImageData(VTK_UNSIGNED_CHAR,3,20,20);
  vtkSmartPointer<vtkImageData>  iphImage = NewImageData(VTK_UNSIGNED_CHAR,3,20,20);
  vtkSmartPointer<vtkImageData> pvatImage = NewImageData(VTK_UNSIGNED_CHAR,3,20,20);
  FillImageData(calcImage,3,dict->GetColor("CALC"),255);
  FillImageData(lrncImage,3,dict->GetColor("LRNC"),255);
  FillImageData(iphImage,3,dict->GetColor("IPH"),255);
  FillImageData(matxImage,3,dict->GetColor("MATX"),255);
  FillImageData(pvatImage,3,dict->GetColor("PVAT"),255);
  double textColor[3] = { 1,1,1 };
  viewer->GetLegendBox()->SetNumberOfEntries(5);
  viewer->GetLegendBox()->SetEntry(0,calcImage,"CALC",textColor);
  viewer->GetLegendBox()->SetEntry(1,lrncImage,"LRNC",textColor);
  viewer->GetLegendBox()->SetEntry(2,iphImage,"IPH",textColor);
  viewer->GetLegendBox()->SetEntry(3,matxImage,"MATX",textColor);
  viewer->GetLegendBox()->SetEntry(4,pvatImage,"PVAT",textColor);
  viewer->GetLegendBox()->UseBackgroundOn();
  double bkgd[5] = { 53.0/255.0,53.0/255.0,53.0/255.0,53.0/255.0,1.0 };
  viewer->GetLegendBox()->SetBackgroundColor(bkgd);
  viewer->SetLegendBelowLogo(65,60);
  // add balloon hover widget text
  QString analyteStr = tr("\
CALC - Calcification\n\
    Thought to be the physiologic defensive process of\n\
    attempting to stabilize plaque, which has\n\
    a mechanism akin to bone formation. It\n\
    is an accumulation of calcium salts in\n\
    vascular tissue.\n\
\n\
LRNC - Lipid Core\n\
    Thought to be the pathologic retention of lipids by\n\
    intimal/medial cells leading to progressive\n\
    cell loss, cell death, degeneration, and\n\
    necrosis. It is a mixture of lipid, cellular\n\
    debris in various concentrations.\n\
\n\
IPH - Intra-plaque Hemorrhage\n\
    Thought to be the presence of an accumulation of erythrocytes\n\
    in the deeper regions of the plaque, with\n\
    or without communication to the lumen or \n\
    neovasculature.\n\
    \n\
MATX - Matrix\n\
    The organization of macromolecules (such\n\
    as collagen, elastin, glycoproteins, and\n\
    proteoglycans) thought to provide structural\n\
    support, tensile strength, elasticity to the\n\
    arterial wall.\n\
    \n\
PVAT - Perivascular Adipose Tissue surrounding blood vessels.\n\
    Thought to be a sign of pathologic inflammation.");
  viewer->GetBalloonWidget()->AddBalloon(viewer->GetLegendBox(),analyteStr.toStdString().c_str(),nullptr);
  viewers->GetViewer(ebvIdSliceAreaRender0)->GetBalloonWidget()->SetEnabled(1);
}

void patientAnalyze::establishTools(QString product, int index, bool masterWithRespectToMenu)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // set up the tools, initally all disabled
  menuActionEnabledMap = new std::map<QAction *, bool>;
  summaryTools = new capTools(owner, product, index, viewers, id3d, ebvViewer::THREED, ui->summaryAreaControlsBox, patientAnalyzeMenu, masterWithRespectToMenu, "__SUMMARY__", menuActionEnabledMap);
  summaryTools->disableToolButtons();
  summaryTools->markLesionEnableState = true; // always have the needed context for this in patientAnalyze
  summaryTools->clearLesionsEnableState = true; // always have the needed context for this in patientAnalyze
  summaryTools->editSegmentationEnableState = false; // unlike in targetDefine where this state is always disabled because patientAnalyze lacks the needed context
  summaryTools->editSegmentationActionLabel = tr("(Edit Segmentation only accessible via Define)");
  HIDECONTROL(summaryTools->ToggleLumenTool, summaryTools->ToggleLumenAction);
  HIDECONTROL(summaryTools->ToggleWallTool, summaryTools->ToggleWallAction);
  HIDECONTROL(summaryTools->ShowAsMovieTool, summaryTools->ShowAsMovieAction);//PUT BACK IN WHEN IMPLEMENTED: if (owner != 0) ui->summaryAreaControls->insertWidget(0, summaryTools->ShowAsMovieTool);
  HIDECONTROL(summaryTools->AnnotateTool, summaryTools->AnnotateAction);//PUT BACK IN WHEN IMPLEMENTED: if (owner != 0) ui->summaryAreaControls->insertWidget(0, summaryTools->AnnotateTool);
  HIDECONTROL(summaryTools->MaxThrTool, summaryTools->MaxThrAction);
  HIDECONTROL(summaryTools->MaxUlcTool, summaryTools->MaxUlcAction);
  if (owner != 0) ui->summaryAreaControls->insertWidget(0, summaryTools->MaxMATXTool);
  if (owner != 0) ui->summaryAreaControls->insertWidget(0, summaryTools->MaxIPHTool);
  if (owner != 0) ui->summaryAreaControls->insertWidget(0, summaryTools->MaxLRNCTool);
  if (owner != 0) ui->summaryAreaControls->insertWidget(0, summaryTools->MaxCalcTool);
  if (owner != 0) ui->summaryAreaControls->insertWidget(0, summaryTools->MaxWTTool);
  if (owner != 0) ui->summaryAreaControls->insertWidget(0, summaryTools->MaxRRTool);
  if (owner != 0) ui->summaryAreaControls->insertWidget(0, summaryTools->MaxDilationTool);
  if (owner != 0) ui->summaryAreaControls->insertWidget(0, summaryTools->MaxStenosisTool);
  HIDECONTROL(summaryTools->ToggleCornerTool, summaryTools->ToggleCornerAction);
  HIDECONTROL(summaryTools->OptimizeSubvolumeTool, summaryTools->OptimizeSubvolumeAction);
  HIDECONTROL(summaryTools->ShapeIntensityTool, summaryTools->ShapeIntensityAction);
  HIDECONTROL(summaryTools->RestoreViewSettingsTool, summaryTools->RestoreViewSettingsAction);
  HIDECONTROL(summaryTools->SaveViewSettingsTool, summaryTools->SaveViewSettingsAction);
  HIDECONTROL(summaryTools->MeasureTool, summaryTools->MeasureAction);
  HIDECONTROL(summaryTools->TargetPathSwitchTool, summaryTools->TargetPathSwitchAction);
  HIDECONTROL(summaryTools->TargetCreateTool, summaryTools->TargetCreateAction);
  HIDECONTROL(summaryTools->TargetDeleteTool, summaryTools->TargetDeleteAction);
  HIDECONTROL(summaryTools->TargetModifyTool, summaryTools->TargetModifyAction);
  HIDECONTROL(summaryTools->ToggleObliqueTool, summaryTools->ToggleObliqueAction);
  HIDECONTROL(summaryTools->VolumeRenderTool, summaryTools->VolumeRenderAction);
  if (owner != 0) ui->summaryAreaControls->insertWidget(0, summaryTools->ToggleCoronalTool);
  if (owner != 0) ui->summaryAreaControls->insertWidget(0, summaryTools->ToggleSagittalTool);
  if (owner != 0) ui->summaryAreaControls->insertWidget(0, summaryTools->ToggleAxialTool);
  HIDECONTROL(summaryTools->ToggleColorBarTool, summaryTools->ToggleColorBarAction);
  HIDECONTROL(summaryTools->CenterAtCursorTool, summaryTools->CenterAtCursorAction);
  HIDECONTROL(summaryTools->MoveProximalTool, summaryTools->MoveProximalAction);
  HIDECONTROL(summaryTools->MoveDistalTool, summaryTools->MoveDistalAction);
  if (owner != 0) ui->summaryAreaControls->insertWidget(0, summaryTools->SaveToReportTool);
  
  sliceTools = new capTools(owner, product, index, viewers, ebvIdSliceAreaRender0, sliceAreaRender0Type, ui->sliceAreaControlsBox, patientAnalyzeMenu, masterWithRespectToMenu, "__SLICE__", menuActionEnabledMap);
  sliceTools->enableToolButtons(); // start the buttons enabled
  sliceTools->markLesionEnableState = true; // always have the needed context for this in patientAnalyze
  sliceTools->clearLesionsEnableState = true; // always have the needed context for this in patientAnalyze
  sliceTools->editSegmentationEnableState = false; // unlike in targetDefine where this state is always disabled because patientAnalyze lacks the needed context
  sliceTools->editSegmentationActionLabel = tr("(Edit Segmentation only accessible via Define)");
  HIDECONTROL(sliceTools->ShowAsMovieTool, sliceTools->ShowAsMovieAction);//PUT BACK IN WHEN IMPLEMENTED: if (owner != 0) ui->sliceAreaControls->insertWidget(0, sliceTools->ShowAsMovieTool);
  HIDECONTROL(sliceTools->AnnotateTool, sliceTools->AnnotateAction);//PUT BACK IN WHEN IMPLEMENTED: if (owner != 0) ui->sliceAreaControls->insertWidget(0, sliceTools->AnnotateTool);
  if (owner != 0) ui->sliceAreaControls->insertWidget(0, sliceTools->MeasureTool);
  if (owner != 0) ui->sliceAreaControls->insertWidget(0, sliceTools->TargetCreateTool);
  if (owner != 0) ui->sliceAreaControls->insertWidget(0, sliceTools->TargetModifyTool);
  if (owner != 0) ui->sliceAreaControls->insertWidget(0, sliceTools->TargetDeleteTool);
  if (owner != 0) ui->sliceAreaControls->insertWidget(0, sliceTools->TargetPathSwitchTool);
  if (owner != 0) ui->sliceAreaControls->insertWidget(0, sliceTools->MoveProximalTool);
  if (owner != 0) ui->sliceAreaControls->insertWidget(0, sliceTools->MoveDistalTool);
  HIDECONTROL(sliceTools->MaxCalcTool, sliceTools->MaxCalcAction);
  HIDECONTROL(sliceTools->MaxLRNCTool, sliceTools->MaxLRNCAction);
  HIDECONTROL(sliceTools->MaxIPHTool, sliceTools->MaxIPHAction);
  HIDECONTROL(sliceTools->MaxMATXTool, sliceTools->MaxMATXAction);
  HIDECONTROL(sliceTools->MaxUlcTool, sliceTools->MaxUlcAction);
  HIDECONTROL(sliceTools->MaxThrTool, sliceTools->MaxThrAction);
  HIDECONTROL(sliceTools->MaxStenosisTool, sliceTools->MaxStenosisAction);
  HIDECONTROL(sliceTools->MaxDilationTool, sliceTools->MaxDilationAction);
  HIDECONTROL(sliceTools->MaxRRTool, sliceTools->MaxRRAction);
  HIDECONTROL(sliceTools->MaxWTTool, sliceTools->MaxWTAction);
  HIDECONTROL(sliceTools->OptimizeSubvolumeTool, sliceTools->OptimizeSubvolumeAction);
  HIDECONTROL(sliceTools->ShapeIntensityTool, sliceTools->ShapeIntensityAction);
  HIDECONTROL(sliceTools->RestoreViewSettingsTool, sliceTools->RestoreViewSettingsAction);
  HIDECONTROL(sliceTools->SaveViewSettingsTool, sliceTools->SaveViewSettingsAction);
  HIDECONTROL(sliceTools->VolumeRenderTool, sliceTools->VolumeRenderAction);
  HIDECONTROL(sliceTools->ToggleCoronalTool, sliceTools->ToggleCoronalAction);
  HIDECONTROL(sliceTools->ToggleSagittalTool, sliceTools->ToggleSagittalAction);
  HIDECONTROL(sliceTools->ToggleAxialTool, sliceTools->ToggleAxialAction);
  HIDECONTROL(sliceTools->ToggleCornerTool, sliceTools->ToggleCornerAction);
  if (owner != 0) ui->sliceAreaControls->insertWidget(0, sliceTools->ToggleColorBarTool);
  if (owner != 0) ui->sliceAreaControls->insertWidget(0, sliceTools->ToggleLumenTool);
  if (owner != 0) ui->sliceAreaControls->insertWidget(0, sliceTools->ToggleWallTool);
  if (owner != 0) ui->sliceAreaControls->insertWidget(0, sliceTools->ToggleObliqueTool);
  if (owner != 0) ui->sliceAreaControls->insertWidget(0, sliceTools->CenterAtCursorTool);
  if (owner != 0) ui->sliceAreaControls->insertWidget(0, sliceTools->SaveToReportTool);

  if (masterWithRespectToMenu) {
    patientAnalyzeMenu->addSeparator();
    gotoPatientAnalyzeAction = new QAction(tr("Go to Patient Analyze"), this);
    gotoPatientAnalyzeAction->setObjectName("gotoPatientAnalyze");
    patientAnalyzeMenu->addAction(gotoPatientAnalyzeAction);

    gotoPatientAnalyzeAction->setEnabled(false);
    patientAnalyzeMenu->setEnabled(false);
  }
  else {
    foreach (QAction *a, patientAnalyzeMenu->actions()) {
      if (a->objectName() == "gotoPatientAnalyze")
        gotoPatientAnalyzeAction = a;
      (*menuActionEnabledMap)[a] = true;
    }
  }
}  

void patientAnalyze::disconnectMenuActions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  disconnect(patientAnalyzeMenu, SIGNAL(triggered(QAction *)), this, SLOT(ensureOnPatientAnalyzePage()));
  //disconnect(gotoPatientAnalyzeAction, SIGNAL(triggered()), this, SLOT(ensureOnPatientAnalyzePage()));
  summaryTools->disconnectMenuActions();
  sliceTools->disconnectMenuActions();
  disableMenuActions();
}

void patientAnalyze::connectMenuActions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  connect(patientAnalyzeMenu, SIGNAL(triggered(QAction *)), this, SLOT(ensureOnPatientAnalyzePage()));
  //connect(gotoPatientAnalyzeAction, SIGNAL(triggered()), this, SLOT(ensureOnPatientAnalyzePage()));
  summaryTools->connectMenuActions();
  sliceTools->connectMenuActions();
  enableMenuActions();
}

void patientAnalyze::disableMenuActions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  for (auto actionEnabled : *menuActionEnabledMap)
    actionEnabled.first->setEnabled(false);
  patientAnalyzeMenu->setEnabled(false);
  gotoPatientAnalyzeAction->setEnabled(false);
  owner->getPatientReport(sessionItemIndex)->disableMenuActions(); // propagate it downstream
}

void patientAnalyze::enableMenuActions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  for (auto actionEnabled : *menuActionEnabledMap)
    actionEnabled.first->setEnabled(actionEnabled.second);
  patientAnalyzeMenu->setEnabled(true);
  gotoPatientAnalyzeAction->setEnabled(true);
  owner->getPatientReport(sessionItemIndex)->enableMenuActions(); // propagate it downstream
}

void patientAnalyze::ensureOnPatientAnalyzePage() 
{ 
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  switch (owner->getWorkItemProcess(sessionItemIndex)->currentIndex()) {
    case PATIENTREPORT:
      owner->getPatientReport(sessionItemIndex)->on_backToAnalyzeButton_clicked();
      return; // don't fall through

    case SERIESSURVEY:
      owner->getSeriesSurvey(sessionItemIndex)->on_proceedToAnalysisButton_clicked();
      // fall through

    case TARGETDEFINE:
      owner->getTargetDefine(sessionItemIndex)->on_continueWithAnalysisButton_clicked();
  }
}

void patientAnalyze::preloadAnalyze(QString product, int index, ebiVesselPipeline::Pointer p, QList<imageSeries> *imageSeriesSet, QList<targetDef> *targetDefs)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  thisProduct = product;
  sessionItemIndex = index;
  pipeline = p;
  images = imageSeriesSet; 
  targets = targetDefs; 
  establishTools(thisProduct, sessionItemIndex, false);

  structureSel = new readingsSelector(this, tr("Structure"), viewers, ebvIdSummaryAreaGraph1);
  compositionSel = new readingsSelector(this, tr("Composition"), viewers, ebvIdSummaryAreaGraph2);

  // Now it is important to initialize the various controls effecting planes according to how the viewers were initialized.
  // we set them in both control sets in case controls move from one side to the other as the product evolves.
  summaryTools->VolumeRenderTool->setChecked(false);
  sliceTools->VolumeRenderTool->setChecked(false);
  summaryTools->ToggleObliqueTool->setChecked(true);
  sliceTools->ToggleObliqueTool->setChecked(true);
  summaryTools->ToggleCoronalTool->setChecked(false);
  summaryTools->ToggleSagittalTool->setChecked(false);
  summaryTools->ToggleAxialTool->setChecked(false);
  sliceTools->ToggleCoronalTool->setChecked(false);
  sliceTools->ToggleSagittalTool->setChecked(false);
  sliceTools->ToggleAxialTool->setChecked(false);

  // last but not least, start out by explicitly indicating there is no current target
  setCurrentTarget(NULL);
  summaryTools->disableToolButtons();
  sliceTools->disableToolButtons();
  for (auto actionEnabled : *menuActionEnabledMap)
    actionEnabled.first->setEnabled(false);
}

void patientAnalyze::resetWI(ebiVesselPipeline::Pointer p, QList<imageSeries> *imageSeriesSet, QList<targetDef> *targetDefs)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  pipeline = p;
  images = imageSeriesSet; 
  targets = targetDefs; 
}

bool patientAnalyze::switchTargetPath()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->setEnabled(false);
  viewers->GetScene()->IncrCurrentPathSetCursor(1);
  if (viewers->GetScene()->IsVesselTargetSelected()) {
    ebID id = viewers->GetScene()->GetSelectedVesselTargetID();
    if ((currentTarget == NULL) 
        || ((id != currentTarget->initializerIDdefine) || (id != currentTarget->pathIDdefine) || (id != currentTarget->readingsIDdefine))) {
      // we not only switched paths, but targets as well.  Need to reach back to Define to make his switch.
      float perspectiveDistance = viewers->GetViewer3D(id3d)->GetPerspectiveDistance();
      (void) owner->getTargetDefine(sessionItemIndex)->switchTargetPath();
      viewers->GetViewer3D(id3d)->SetPerspectiveDistance(perspectiveDistance);
    } // end-if there had been no prior selected target or if selected target has changed
    else { // there is no change to the target (possibly the path within the target changed, but no actual target switch)
      this->setEnabled(true);
      return true;
    }
  } // end-if there is a selected target
  else {
    // get here either if there is no selected target or if can't find the one that supposedly is
    setCurrentTarget(NULL);
    DISABLECONTROL(sliceTools->ToggleObliqueTool, sliceTools->ToggleObliqueAction, tr("ToggleOblique is disabled (select a target with a path to enable)"));
    DISABLECONTROL(sliceTools->MoveProximalTool, sliceTools->MoveProximalAction, tr("MoveProximal is disabled (select a target with a path to enable)"));
    DISABLECONTROL(sliceTools->MoveDistalTool, sliceTools->MoveDistalAction, tr("MoveDistal is disabled (select a target to enable)"));
    message->showMessage(tr("Logic error; viable target should be found. (contact Elucid if condition persists)"));
    this->setEnabled(true);
    return false;
  }
  owner->setEnabled(true);
  return true;
}

void patientAnalyze::displayTargets()
{
  ebLog eblog(Q_FUNC_INFO); eblog << std::to_string(targets->size()) << std::endl;
  this->setEnabled(false);
  
  // prior to looping over targets, assume there are no regions or probability maps
  DISABLECONTROL(sliceTools->ToggleLumenTool, sliceTools->ToggleLumenAction, tr("ToggleLumen is disabled"));
  DISABLECONTROL(sliceTools->ToggleWallTool, sliceTools->ToggleWallAction, tr("ToggleWall is disabled"));
  DISABLECONTROL(sliceTools->ToggleColorBarTool, sliceTools->ToggleColorBarAction, tr("ToggleColorBar is disabled"));

  // loop over the targets
  for (int i=0; i < targets->size(); i++) {
    qDebug() << "beginning target " << i;
    targetDef *def = &((*targets)[i]);
    if (def->isViable) {
      // start from path and work forward, as deep as can go based on what is in place (partially overlapping what define does, but going deeper)
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath()) {
        if (def->initializerIDanalyze != null_ebID) {
          viewers->RemoveVesselTarget(def->initializerIDanalyze); // if have a path, don't want to display the initializer too
        }
        def->initializerIDanalyze = null_ebID;
        if (def->pathIDanalyze != null_ebID) {
          viewers->RemoveVesselTarget(def->pathIDanalyze); 
        }
        def->pathIDanalyze = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath(), "path"); 
        /*if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation()) { 
        def->lumenSegIDanalyze = viewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation(), 0, "lumenSegmentation"); 
        viewers->GetScene()->GetSegmentation4(def->lumenSegIDanalyze)->SetSmoothMarchingCubes(true);*/
        if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenPartition()) {
          // first remove any old ones
          if (!def->lumenPartIDsAnalyze.empty()) {
            for (auto partitionID : def->lumenPartIDsAnalyze) {
              viewers->RemoveSegmentation4(partitionID);
            }
          }
          // now start fresh
          def->lumenPartIDsAnalyze.clear();
          int j = 0;
          for (auto partition : *pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenPartition()) {
            ebID partitionID = viewers->AddSegmentation4(partition.GetPointer(), 0, "lumenPartition"+std::to_string(j++));
            viewers->GetScene()->GetSegmentation4(partitionID)->SetSmoothMarchingCubes(true);
            def->lumenPartIDsAnalyze.insert(partitionID);
          }
          ENABLECONTROL(sliceTools->ToggleLumenTool, sliceTools->ToggleLumenAction, tr("Press to toggle lumen display"));
          sliceTools->ToggleLumenTool->setChecked(true); 
          /*if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallSegmentation()) {
          def->wallSegIDanalyze = viewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallSegmentation(), 0, "wallSegmentation"); 
          viewers->GetScene()->GetSegmentation4(def->wallSegIDanalyze)->SetSmoothMarchingCubes(true);*/
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallPartition()) {
            // first remove any old ones
            if (!def->wallPartIDsAnalyze.empty()) {
              for (auto partitionID : def->wallPartIDsAnalyze) {
                viewers->RemoveSegmentation4(partitionID);
              }
            }
            // now start fresh
            def->wallPartIDsAnalyze.clear();
            j = 0;
            for (auto partition : *pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallPartition()) {
              ebID partitionID = viewers->AddSegmentation4(partition.GetPointer(), 0, "wallPartition"+std::to_string(j++));
              viewers->GetScene()->GetSegmentation4(partitionID)->GetInteriorProperty()->SetOpacity(0);
              viewers->GetScene()->GetSegmentation4(partitionID)->GetContourProperty()->SetOpacity(.2);
              viewers->GetScene()->GetSegmentation4(partitionID)->GetSurfaceProperty()->SetOpacity(.2);
              viewers->GetScene()->GetSegmentation4(partitionID)->SetSmoothMarchingCubes(true);
              def->wallPartIDsAnalyze.insert(partitionID);
              //std::string partitionRegion = GetMetaData(partition.GetPointer(),"region");              
            }
            ENABLECONTROL(sliceTools->ToggleWallTool, sliceTools->ToggleWallAction, tr("Press to toggle outer wall display"));
            sliceTools->ToggleWallTool->setChecked(true); 
            if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetWallThickness()) {
              // display the wall thickness (first removing a prior one if present)
              if (def->wallThickIDanalyze != null_ebID) {
                viewers->GetScene()->RemoveImage4(def->wallThickIDanalyze);
              }
              def->wallThickIDanalyze = viewers->AddImage4(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetWallThickness(), "wallThickness");
              viewers->GetScene()->GetImage4(def->wallThickIDanalyze)->SetImageIntensityType(ebvLinkedViewersImage4::THICKNESS);
              viewers->GetScene()->GetImage4(def->wallThickIDanalyze)->SetIntensityUnitString("mm");
              // now remove any old composition...
              if (!def->compositionSegIDsAnalyze.empty()) {
                for (auto analyteSegID : def->compositionSegIDsAnalyze) {
                  viewers->RemoveSegmentation4(analyteSegID);
                }
              }
              def->compositionSegIDsAnalyze.clear();
              viewers->SyncViewersToScene();
              /*THIS IS WHERE PERIVASCULARREGION WOULD GO*/
                // ...and now check if there is new composition to display
                if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetComposition()) {
                  // display the composition
                  ebAssert(pipeline);
                  ebAssert(pipeline->GetVesselTargetPipeline(def->targetPipelineID));
                  ebAssert(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetComposition());
                  for (auto analyte : *pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetComposition()) {
                    std::string analyteName = GetMetaData(analyte,"region");
                    std::string pixelInterpretation = GetMetaData(analyte,"pixelInterpretation");
                    ebAssert(analyte);
                    ebID analyteSegID = (pixelInterpretation == "VolumeFraction") ?
                      viewers->AddSegmentation4(analyte.GetPointer(), 0.5, analyteName) :
                      viewers->AddSegmentation4(analyte.GetPointer(), 0, analyteName);
                    def->compositionSegIDsAnalyze.insert(analyteSegID);
                    std::string partitionRegion = GetMetaData(analyte.GetPointer(),"region");
                    if (partitionRegion == "CALC") {
                      viewers->GetScene()->GetSegmentation4(analyteSegID)->GetInteriorProperty()->SetOpacity(0.3);
                      viewers->GetScene()->GetSegmentation4(analyteSegID)->GetContourProperty()->SetOpacity(0.4);
                      viewers->GetScene()->GetSegmentation4(analyteSegID)->GetSurfaceProperty()->SetOpacity(0.4);
                    } else if (partitionRegion == "LRNC") {
                      viewers->GetScene()->GetSegmentation4(analyteSegID)->GetInteriorProperty()->SetOpacity(0.3);
                      viewers->GetScene()->GetSegmentation4(analyteSegID)->GetContourProperty()->SetOpacity(0.4);
                      viewers->GetScene()->GetSegmentation4(analyteSegID)->GetSurfaceProperty()->SetOpacity(0.4);
                    } else if (partitionRegion == "IPH") {
                      viewers->GetScene()->GetSegmentation4(analyteSegID)->GetInteriorProperty()->SetOpacity(0.3);
                      viewers->GetScene()->GetSegmentation4(analyteSegID)->GetContourProperty()->SetOpacity(0.4);
                      viewers->GetScene()->GetSegmentation4(analyteSegID)->GetSurfaceProperty()->SetOpacity(0.4);
                    } else if (partitionRegion == "MATX") {
                      viewers->GetScene()->GetSegmentation4(analyteSegID)->GetInteriorProperty()->SetOpacity(0);
                      viewers->GetScene()->GetSegmentation4(analyteSegID)->GetContourProperty()->SetOpacity(0);
                      viewers->GetScene()->GetSegmentation4(analyteSegID)->GetSurfaceProperty()->SetOpacity(0);
                    } else if (partitionRegion == "PVAT") {
                      viewers->GetScene()->GetSegmentation4(analyteSegID)->GetInteriorProperty()->SetOpacity(0.15); // USE 0 IF DON'T WANT PVAT DISPLAY
                      //viewers->GetScene()->GetSegmentation4(analyteSegID)->GetInteriorProperty()->SetEmissiveTexture(7, 0.9804, 0.5020, 0.4471, 1); //salmon
                      std::cerr << "pvat information is " << *viewers->GetScene()->GetSegmentation4(analyteSegID)->GetInteriorProperty();
                      //uc2d(0xBE),uc2d(0x96),uc2d(0x00)
                      //viewers->GetScene()->GetSegmentation4(analyteSegID)->GetInteriorProperty()->SetDiffuseColor(0xBE,0x96,0x00);
                      viewers->GetScene()->GetSegmentation4(analyteSegID)->GetContourProperty()->SetOpacity(0.0);
                      viewers->GetScene()->GetSegmentation4(analyteSegID)->GetSurfaceProperty()->SetOpacity(0.15); // USE 0 IF DON'T WANT PVAT DISPLAY
                    } 
                  }
                  ENABLECONTROL(sliceTools->ToggleColorBarTool, sliceTools->ToggleColorBarAction, tr("Press to toggle color presentation"));
                  sliceTools->ToggleColorBarTool->setChecked(true); 
                  if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetCapThickness()) {
                    // display the cap thickness (first removing a prior one if present)
                    if (def->capThickIDanalyze != null_ebID) {
                      viewers->GetScene()->RemoveImage4(def->capThickIDanalyze);
                    }
                    def->capThickIDanalyze = viewers->AddImage4(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetCapThickness(), "capThickness");
                    viewers->GetScene()->GetImage4(def->capThickIDanalyze)->SetImageIntensityType(ebvLinkedViewersImage4::THICKNESS);
                    viewers->GetScene()->GetImage4(def->capThickIDanalyze)->SetIntensityUnitString("mm");
                    viewers->SyncViewersToScene();
                    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetReadings()) { 
                      // remove the path display and display the readings
                      if (def->initializerIDanalyze != null_ebID) {
                        viewers->RemoveVesselTarget(def->initializerIDanalyze); // if have a path, don't want to display the initializer too
                      }
                      def->initializerIDanalyze = null_ebID;
                      if (def->pathIDanalyze != null_ebID) {
                        viewers->RemoveVesselTarget(def->pathIDanalyze); // if have readings, don't want to display the path too
                      }
                      def->pathIDanalyze = null_ebID;
                      if (def->readingsIDanalyze != null_ebID) {
                        viewers->RemoveVesselTarget(def->readingsIDanalyze);
                      }
                      viewers->SyncViewersToScene();
                      def->readingsIDanalyze = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetReadings(), "readings"); 
                      // now see if can proceed to lesions
                      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionLumenPartition()) {
                        // first remove any old ones
                        if (!def->lesionLumenPartIDsAnalyze.empty()) {
                          for (auto partitionID : def->lesionLumenPartIDsAnalyze) {
                            viewers->RemoveSegmentation4(partitionID);
                          }
                        }
                        def->lesionLumenPartIDsAnalyze.clear();
                        // now start fresh
                        j = 0;
                        for (auto partition : *pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionLumenPartition()) {
                          ebID partitionID = viewers->AddSegmentation4(partition.GetPointer(), 0, "lesionLumenPartition"+std::to_string(j++));
                          viewers->GetScene()->GetSegmentation4(partitionID)->SetSmoothMarchingCubes(true);
                          def->lesionLumenPartIDsAnalyze.insert(partitionID);
                        }
                        // and now lesionLumenAndWall
                        if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionLumenAndWallPartition()) {
                          // first remove any old ones
                          if (!def->lesionLumenAndWallPartIDsAnalyze.empty()) {
                            for (auto partitionID : def->lesionLumenAndWallPartIDsAnalyze) {
                              viewers->RemoveSegmentation4(partitionID);
                            }
                          }
                          def->lesionLumenAndWallPartIDsAnalyze.clear();
                          // now start fresh
                          j = 0;
                          for (auto partition : *pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionLumenAndWallPartition()) {
                            ebID partitionID = viewers->AddSegmentation4(partition.GetPointer(), 0, "lesionLumenAndWallPartition"+std::to_string(j++));
                            viewers->GetScene()->GetSegmentation4(partitionID)->SetSmoothMarchingCubes(true);
                            def->lesionLumenAndWallPartIDsAnalyze.insert(partitionID);
                          }
                          // last but not least, lesion readings
                          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionReadings()) { // not nested inside of composition because it is possible to just have structure readings
                            // remove the path display and display the readings
                            if (def->initializerIDanalyze != null_ebID) {
                              viewers->RemoveVesselTarget(def->initializerIDanalyze); // if have a path, don't want to display the initializer too
                            }
                            def->initializerIDanalyze = null_ebID;
                            if (def->pathIDanalyze != null_ebID) {
                              viewers->RemoveVesselTarget(def->pathIDanalyze); // if have readings, don't want to display the path too
                            }
                            def->pathIDanalyze = null_ebID;
                            if (def->readingsIDanalyze != null_ebID) {
                              viewers->RemoveVesselTarget(def->readingsIDanalyze);
                            }
                            def->readingsIDanalyze = null_ebID;
                            if (def->lesionReadingsIDanalyze != null_ebID) {
                              viewers->RemoveVesselTarget(def->lesionReadingsIDanalyze);
                            }
                            def->lesionReadingsIDanalyze = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionReadings(), "lesion readings");
                            viewers->SyncViewersToScene();
                          } // end-if there are lesion readings
                        } // end-if there are lesionLumenAndWall partitions
                      } // end-if there are lesionLumen partitions
                    } // end-if there are readings
                  } // end-if there is cap thickness
                } // end-if there is composition
              //} // end-if there is a perivascular region
            } // end-if there is wall thickness
          } // wnd-if there is a partitioned wall
        } // end-if there is a partitioned lumen
      } // end-if there is a path
    } // end-if the target has been deemed as viable 
    else 
      qWarning() << "target in list position" << i << "is not viable, skipping.";
  } // end-for each listed target

  // enable the right set of buttons, given that there is no current target focus yet.
  enableControlsAsAppropriateWhenNoTargetFocus();
  ui->proceedToReportButton->setEnabled(true); // ability to proceed to report does not depend on target focus
  this->setEnabled(true);
}

void patientAnalyze::processCompositionSettingsChange()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  this->setEnabled(false);
  /*The following lines are a fairly brute force way to fix VASCUCAP-605. See the original lines below, which have been retained but 
    commented out for now as it may be better to fix the underlying problem rather than resorting to what is here now, which serves
    as an effective fix without known side-effects but which is not elegant because it removes/adds whole viewers in order to guarrantee
    proper state. Root cause analysis of VASCUCAP-605 led to ebvLinkedViewersImage4::UpdateCPR, which was crashing at the 
    image3s.at(currentPhase).cprs[cprIndex].cprAlg->Update(); line when the composition control was used. This in turn may be the result
    of RemoveSegmentation not completely cleanign the pipline for CPR objects, or possibly because it thinks there is a CPR object where
    there is none, but in either case scrubbing the viewers as done here clears the problem even if inelegant.  This could be 
    reconsidered in future re-factoring if desired.*/
  // set the screen to define
  giveToDefineIfNotAlreadyThere();
  // get rid of old viewers that otherwise can contain bad state
  viewers->RemoveViewer(id3d);
  viewers->RemoveViewer(ebvIdSliceAreaRender0);
  viewers->RemoveViewer(ebvIdSliceAreaRender1);
  viewers->RemoveViewer(ebvIdSliceAreaRender2);
  // put the viewers back
  id3d = viewers->AddViewer(ebvViewer::THREED,ui->summaryAreaRender0->GetRenderWindow());
  viewers->GetViewer(id3d)->SetShowLogo(true);
  // planes set for initial condition of oblique OFF
  viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::AXIAL, true);
  viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::CORONAL, true);
  viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::SAGITTAL, true);
  viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::OBLIQUE, false);
  viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
  viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::CPR, false);
  viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::OBLIQUEPERP2, false);
    
  sliceAreaRender0Type = ebvViewer::AXIAL;
  ebvIdSliceAreaRender0 = viewers->AddViewer(sliceAreaRender0Type,ui->sliceAreaRender0->GetRenderWindow());
  viewers->GetViewer(ebvIdSliceAreaRender0)->SetShowLogo(true);
  sliceAreaRender1Type = ebvViewer::CORONAL;
  ebvIdSliceAreaRender1 = viewers->AddViewer(sliceAreaRender1Type,ui->sliceAreaRender1->GetRenderWindow());
  viewers->GetViewer(ebvIdSliceAreaRender1)->SetShowLogo(true);
  sliceAreaRender2Type = ebvViewer::SAGITTAL;
  ebvIdSliceAreaRender2 = viewers->AddViewer(sliceAreaRender2Type,ui->sliceAreaRender2->GetRenderWindow());
  viewers->GetViewer(ebvIdSliceAreaRender2)->SetShowLogo(true);

  viewers->GetViewer3D(id3d)->SetShowSceneCornerAnnotations(true);
  viewers->GetViewer2D(ebvIdSliceAreaRender0)->SetShowSceneCornerAnnotations(true);
  viewers->GetViewer2D(ebvIdSliceAreaRender1)->SetShowSceneCornerAnnotations(false);
  viewers->GetViewer2D(ebvIdSliceAreaRender2)->SetShowSceneCornerAnnotations(false);
  addViewersLegend();
  viewers->SyncViewersToScene();
  viewers->SetScreenScaleFitFactor2D(1.0);
  viewers->OptimizeScreenScale2D();
  // and finally get back to analyze being active
  owner->getTargetDefine(sessionItemIndex)->giveToAnalyzeIfNotAlreadyThere();
  /*END OF VASCUCAP-605 FIX*/
  /*THIS IS HOW IT WAS BEFORE VASCUCAP-605:
  viewers->GetViewer(id3d)->SetTemporaryText(tr("Re-computing Composition").toStdString());
  viewers->SyncViewersToScene();
  viewers->Render();
  for (int i=0; i < targets->size(); i++) {
    targetDef *def = &((*targets)[i]);
    if (def->isViable) {
      owner->getTargetDefine(sessionItemIndex)->computeRemainingStages(def);
    }
  }
  viewers->GetViewer(id3d)->SetTemporaryText("");
  viewers->SyncViewersToScene();
  viewers->Render();
  END OF HOW IT WAS BEFORE VASCUCAP-605*/
  enableControlsAsAppropriateGivenATargetFocus();
  displayTargetsAndAnticipateReportContent();
  this->setEnabled(true);
}

bool patientAnalyze::updateServer()
{
    if (!owner->userLoggedInToServer) 
        return true;

    // ship the updated information to the server so that it can start updating report content
    // based on the updated readings.
    workItemListEntry *entry = owner->getWorkItem(sessionItemIndex)->selectedWorkItemListEntryPtr;

    if (!entry->setWorkItemFolder()) {
        // this may be the first time an operation that requires the folder has been done
        // signal that this op failed
        return false;
    }
    QString wiFolder = entry->getWorkItemFolder();
    owner->getWorkItem(sessionItemIndex)->prepChangedDataForServer(wiFolder, entry);
    return true;
}

void patientAnalyze::displayTargetsAndAnticipateReportContent()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QString currentSave = QDir::currentPath();
  displayTargets();
  targetDef *currentTargetSave = currentTarget;

  bool wasCAPenabled = owner->CAPisEnabled();
  owner->enableCAP(false);
  if (newTargetReadingsFlag || newLesionReadingsFlag) {
    // take default key images
    float perspectiveDistance = viewers->GetViewer3D(id3d)->GetPerspectiveDistance();
    for (int i=0; i < targets->size(); i++) {
      targetDef *def = &((*targets)[i]);
      setCurrentTarget(def);
      viewers->GetViewer3D(id3d)->SetPerspectiveDistance(perspectiveDistance);
      viewers->GetScene()->SetCursorPositionToSceneCenter();
      viewers->UpdateCameras();
      if (currentTarget == NULL) // if the currentTarget hasn't been set yet, then force the selection
        sliceTools->TargetPathSwitch();
      def->keyImages.clear();

      // now we do a sequence of automatic key images.  First, a coronal for the target level scaled out a bit for context
      // start be saving what had been in the slice viewer so that it may be restored and then making it a coronal view...
      bool cornersDisplayState = viewers->GetViewer2D(ebvIdSliceAreaRender0)->GetShowSceneCornerAnnotations();
      viewers->RemoveViewer(ebvIdSliceAreaRender0);
      ebvViewer::ViewerType typeSave = sliceAreaRender0Type;
      ebvIdSliceAreaRender0 = viewers->AddViewer(ebvViewer::CORONAL,ui->sliceAreaRender0->GetRenderWindow());
      viewers->GetViewer(ebvIdSliceAreaRender0)->SetShowLogo(true);
      sliceTools->resetViewer(ebvIdSliceAreaRender0, ebvViewer::CORONAL);
      addViewersLegend();

      // ...now take the target-level image...
      jumpTo("VesselTargetMaxStenosisByArea");
      viewers->GetViewer2D(ebvIdSliceAreaRender0)->SetParallelScale(70);
      viewers->PanCamerasToCursor(true, true);
      viewers->SyncViewersToScene();
      viewers->GetViewer3D(id3d)->SetPerspectiveDistance(perspectiveDistance);
      viewers->Render();
      saveToReport(ebvViewer::THREED, /*forceSliceViewerForTarget*/true, /*includeReportUpdate*/false);

      // ...and restore the view type that had been there before.
      viewers->RemoveViewer(ebvIdSliceAreaRender0);
      ebvIdSliceAreaRender0 = viewers->AddViewer(typeSave, ui->sliceAreaRender0->GetRenderWindow());
      viewers->GetViewer(ebvIdSliceAreaRender0)->SetShowLogo(true);
      sliceTools->resetViewer(ebvIdSliceAreaRender0, typeSave);
      viewers->GetViewer2D(ebvIdSliceAreaRender0)->SetShowSceneCornerAnnotations(cornersDisplayState);
      addViewersLegend();

      // Max position for the cross sections
      jumpTo("VesselTargetMaxStenosisByArea");
      viewers->PanCamerasToCursor(true, true);
      viewers->SyncViewersToScene();
      viewers->GetViewer3D(id3d)->SetPerspectiveDistance(perspectiveDistance);
      viewers->Render();
      saveToReport(ebvViewer::OBLIQUE, /*forceSliceViewerForTarget*/false, /*includeReportUpdate*/false);

      if (jumpTo("VesselTargetMaxMaxWallThickness")) {
        viewers->PanCamerasToCursor(true, true);
        viewers->SyncViewersToScene();
        viewers->GetViewer3D(id3d)->SetPerspectiveDistance(perspectiveDistance);
        viewers->Render();
        saveToReport(ebvViewer::OBLIQUE, /*forceSliceViewerForTarget*/false, /*includeReportUpdate*/false);
      }

      if (jumpTo("VesselTargetMaxCalcArea")) {
        viewers->PanCamerasToCursor(true, true);
        viewers->SyncViewersToScene();
        viewers->GetViewer3D(id3d)->SetPerspectiveDistance(perspectiveDistance);
        viewers->Render();
        saveToReport(ebvViewer::OBLIQUE, /*forceSliceViewerForTarget*/false, /*includeReportUpdate*/false);
      }

      if (jumpTo("VesselTargetMaxLRNCArea")) {
        viewers->PanCamerasToCursor(true, true);
        viewers->SyncViewersToScene();
        viewers->GetViewer3D(id3d)->SetPerspectiveDistance(perspectiveDistance);
        viewers->Render();
        saveToReport(ebvViewer::OBLIQUE, /*forceSliceViewerForTarget*/false, /*includeReportUpdate*/false);
      }
    }
  
    if (!updateServer()) {
        message->showMessage(tr("Error: Cannot write work item folder."));
        QDir::setCurrent(currentSave);
        setCurrentTarget(currentTargetSave);
        owner->enableCAP(wasCAPenabled);
        return;
    }
    newTargetReadingsFlag = false;
    newLesionReadingsFlag = false;
  }
  setCurrentTarget(currentTargetSave);

  viewers->GetScene()->SetCursorPositionToSceneCenter();
  viewers->UpdateCameras();
  if (currentTarget == NULL) // if the currentTarget hasn't been set yet, then force the selection
    sliceTools->TargetPathSwitch();
  jumpTo("VesselTargetMaxStenosisByArea");
  viewers->PanCamerasToCursor(true, true);
  viewers->SyncViewersToScene();
  viewers->GetViewer3D(id3d)->SetPerspectiveDistance(.7*viewers->GetViewer3D(id3d)->GetPerspectiveDistance());
  viewers->Render();
  QDir::setCurrent(currentSave);
  owner->enableCAP(wasCAPenabled);
}

void patientAnalyze::acceptScreenControl(QStackedWidget *seriesSelectionArea, imageSeries *series)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  this->setEnabled(false);
  // NOTE: added temporary debugging lines (with eblog) to debug Eva's crashes  1/30/18 (marked tmp)  
  if (!ui)  eblog << "ui null" << std::endl;  // tmp
  if (!ui->seriesSelectionArea)  eblog << "ui->seriesSelectionArea null" << std::endl;  // tmp
  ui->seriesSelectionArea->addWidget(seriesSelectionArea->widget(0));
  ui->seriesSelectionArea->addWidget(seriesSelectionArea->widget(0));
  ui->seriesSelectionArea->setCurrentIndex(0);
  eblog << "1" << std::endl; // tmp
  if (series != currentBackingSeries)
    resetBackingSeries(series);
  else { // at least need to do these parts, even if backing hasn't changed
    // base window and level from what was used before getting here
    if (!currentBackingSeries)  eblog << "currentBackingSeries null" << std::endl;  // tmp
    if (viewers && viewers->GetScene() && viewers->GetScene()->GetSelectedImage4() && currentBackingSeries) {
      eblog << "2" << std::endl; // tmp
      eblog << currentBackingSeries->window << " " << currentBackingSeries->level << std::endl;
      std::cerr << *viewers->GetScene()->GetSelectedImage4() << std::endl;
      std::cerr << *viewers->GetScene() << std::endl;
      std::cerr << *viewers << std::endl;
      viewers->GetScene()->GetSelectedImage4()->SetWindowLevel(currentBackingSeries->window, currentBackingSeries->level);
      eblog << "a" << std::endl;
      viewers->SyncViewersToScene();
      eblog << "b" << std::endl;
      viewers->Render();
      eblog << "c" << std::endl;
    }
  }
  eblog << "3" << std::endl; // tmp
  displayTargetsAndAnticipateReportContent();
  owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(PATIENTANALYZE);
  enableMenuActions();
  this->setEnabled(true);
}

void patientAnalyze::removeAllTargetDisplays()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  this->setEnabled(false);

  for (int i=0; i < targets->size(); i++) {
    targetDef *def = &((*targets)[i]);
    if (def->initializerIDanalyze != null_ebID)
      viewers->RemoveVesselTarget(def->initializerIDanalyze); 
    def->initializerIDanalyze = null_ebID;
    
    if (def->lumenSegIDanalyze != null_ebID)
      viewers->RemoveSegmentation4(def->lumenSegIDanalyze); 
    def->lumenSegIDanalyze = null_ebID;

    if (def->pathIDanalyze != null_ebID)
      viewers->RemoveVesselTarget(def->pathIDanalyze); 
    def->pathIDanalyze = null_ebID;

    if (!def->lumenPartIDsAnalyze.empty()) {
      for (auto partitionID : def->lumenPartIDsAnalyze) {
        viewers->RemoveSegmentation4(partitionID);
      }
    }
    def->lumenPartIDsAnalyze.clear();

    if (def->wallSegIDanalyze != null_ebID)
      viewers->RemoveSegmentation4(def->wallSegIDanalyze); 
    def->wallSegIDanalyze = null_ebID;

    if (!def->wallPartIDsAnalyze.empty()) {
      for (auto partitionID : def->wallPartIDsAnalyze) {
        viewers->RemoveSegmentation4(partitionID);
      }
    }
    def->wallPartIDsAnalyze.clear();

    if (def->wallThickIDanalyze != null_ebID)
      viewers->RemoveImage4(def->wallThickIDanalyze); 
    def->wallThickIDanalyze = null_ebID;

    if (def->periRegIDanalyze != null_ebID)
      viewers->RemoveSegmentation4(def->periRegIDanalyze); 
    def->periRegIDanalyze = null_ebID;

    if (!def->compositionSegIDsAnalyze.empty()) {
      for (auto analyteSegID : def->compositionSegIDsAnalyze) {
        viewers->RemoveSegmentation4(analyteSegID);
      }
    }
    def->compositionSegIDsAnalyze.clear();

    if (def->readingsIDanalyze != null_ebID)
      viewers->RemoveVesselTarget(def->readingsIDanalyze); 
    def->readingsIDanalyze = null_ebID;

    if (!def->lesionLumenPartIDsAnalyze.empty()) {
      for (auto partitionID : def->lesionLumenPartIDsAnalyze) {
        viewers->RemoveSegmentation4(partitionID);
      }
    }
    def->lesionLumenPartIDsAnalyze.clear();

    if (!def->lesionLumenAndWallPartIDsAnalyze.empty()) {
      for (auto partitionID : def->lesionLumenAndWallPartIDsAnalyze) {
        viewers->RemoveSegmentation4(partitionID);
      }
    }
    def->lesionLumenAndWallPartIDsAnalyze.clear();

    if (def->lesionReadingsIDanalyze != null_ebID)
      viewers->RemoveVesselTarget(def->lesionReadingsIDanalyze); 
    def->lesionReadingsIDanalyze = null_ebID;
  }

  if (structureSel) structureSel->cancelReadingsSelection();
  if (compositionSel) compositionSel->cancelReadingsSelection();

  viewers->SyncViewersToScene();
  viewers->Render();
  this->setEnabled(true);
}

void patientAnalyze::setCurrentTarget(targetDef *newCurrentDef)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  this->setEnabled(false);
  std::cerr << viewers << std::endl;
  currentTarget = newCurrentDef;
  DISABLECONTROL(sliceTools->TargetPathSwitchTool, sliceTools->TargetPathSwitchAction, tr("Need a target focus to switch")); // default disabled
  if (currentTarget != NULL) {      
    QString label = tr("Current target focus: ");
    QString displayName = currentTarget->getID();
    // override the display name if it is a coronary target (because the greater branching complexity makes their screen labelling confusing if we don't do this)
    if (displayName.contains("oronary") && currentTarget->rootVesselName != "") {
      displayName = currentTarget->rootVesselName;
    }
    label.append(displayName);
    ui->summaryAreaLabel0->setText(label);
    if ((currentTarget->lesionReadingsIDanalyze != null_ebID) || (currentTarget->readingsIDanalyze != null_ebID) ||
              (currentTarget->pathIDanalyze != null_ebID) || (currentTarget->initializerIDanalyze != null_ebID)) {
      ebID id = null_ebID;
      if (currentTarget->lesionReadingsIDanalyze != null_ebID)
        id = currentTarget->lesionReadingsIDanalyze;
      else if (currentTarget->readingsIDanalyze != null_ebID)
        id = currentTarget->readingsIDanalyze;
      else if (currentTarget->pathIDanalyze != null_ebID)
        id = currentTarget->pathIDanalyze;
      else if (currentTarget->initializerIDanalyze != null_ebID)
        id = currentTarget->initializerIDanalyze;
      viewers->GetScene()->SetSelectedVesselTarget(id);
      ebAssert(viewers && viewers->GetScene() && viewers->GetScene()->GetSelectedVesselTarget());
      viewers->GetScene()->GetSelectedVesselTarget()->GetVesselTarget()->SetName(currentTarget->getID().toStdString());
    }
    if (targets->size() > 0) {
      ENABLECONTROL(sliceTools->TargetPathSwitchTool, sliceTools->TargetPathSwitchAction, tr("Press to switch current target/path")); // can enable if at least 1
    }
    bool atLeastOneAlreadyInTheParens = false;
    if (targets->size() > 1) {
      label = tr("(other targets available for detailed review: ");
      for (int k=0; k < targets->size(); k++) {
        targetDef *def = &((*targets)[k]);
        if (def->getBodySite() != currentTarget->getBodySite()) {
          QString otherTargetDisplayName = def->getID();
          // override the display name if it is a coronary target (because the greater branching complexity makes their screen labelling confusing if we don't do this)
          if (otherTargetDisplayName.contains("oronary") && def->rootVesselName != "") {
            otherTargetDisplayName = def->rootVesselName;
          }
          if (atLeastOneAlreadyInTheParens && (k < targets->size()))
            label.append(", ");
          label.append(otherTargetDisplayName);
          atLeastOneAlreadyInTheParens = true;
        } // end-if this isn't the current one  
      } // end-for each target
      label.append(")");
    } // end-if there are more targets defined
    else
      label = tr("(no other targets have as yet been defined)");
    ui->sliceAreaLabel0->setText(label);
    if (currentTarget->isViable) {
      if (structureSel) structureSel->setTargetId(currentTarget->readingsIDanalyze);
      if (compositionSel) compositionSel->setTargetId(currentTarget->readingsIDanalyze);
    }
    enableControlsAsAppropriateGivenATargetFocus();
    viewers->GetScene()->SetSelectedImage4(backingImageId);
    toggleOblique(sliceTools->ToggleObliqueTool->isChecked());
    viewers->SetScreenScaleFitFactor2D(1.5);
    viewers->OptimizeScreenScale2D();
    viewers->InitializeCameras();
    viewers->GetViewer3D(id3d)->SetPerspectiveDistance(.7*viewers->GetViewer3D(id3d)->GetPerspectiveDistance());
    viewers->PanCamerasToCursor(true, false);
    viewers->SyncViewersToScene();
    viewers->Render();
  }
  else { // the selected target is NULL, which indicates labels should be cleared since there is no current target focus
    QString label = tr("(no current target focus)");
    ui->summaryAreaLabel0->setText(label);
    bool atLeastOneAlreadyInTheParens = false;
    if ((targets != NULL) && (targets->size() != 0)) {
      label = tr("(targets available for detailed review: ");
      for (int k=0; k < targets->size(); k++) {
        targetDef *def = &((*targets)[k]);
        QString otherTargetDisplayName = def->getID();
        // override the display name if it is a coronary target (because the greater branching complexity makes their screen labelling confusing if we don't do this)
        if (otherTargetDisplayName.contains("oronary") && def->rootVesselName != "") {
          otherTargetDisplayName = def->rootVesselName;
        }
        if (atLeastOneAlreadyInTheParens && (k < targets->size()))
          label.append(", ");
        label.append(otherTargetDisplayName);
        atLeastOneAlreadyInTheParens = true;
      } // end-for each target
      label.append(")");
    } // end-if there are targets defined
    else
      label = tr("(no targets have as yet been defined)"); 
    ui->sliceAreaLabel0->setText(label);
    enableControlsAsAppropriateWhenNoTargetFocus();
    viewers->GetScene()->SetSelectedImage4(backingImageId);
    toggleOblique(sliceTools->ToggleObliqueTool->isChecked());
    viewers->SetScreenScaleFitFactor2D(1.5);
    viewers->OptimizeScreenScale2D();
    if (currentBackingSeries != NULL){
      viewers->GetScene()->SetCursorPositionToSceneCenter();
      viewers->UpdateCameras();
    }
    viewers->SyncViewersToScene();
    viewers->InitializeCameras();
    viewers->GetViewer3D(id3d)->SetPerspectiveDistance(.7*viewers->GetViewer3D(id3d)->GetPerspectiveDistance());
    viewers->SyncViewersToScene();
    viewers->Render();
  }
  this->setEnabled(true);
  this->repaint(); // ensure screen is updated
  qApp->processEvents();
}

void patientAnalyze::resetBackingSeries(imageSeries *series)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // set the previous image not visible
  if (currentBackingSeries != NULL)
    viewers->GetScene()->GetImage4(backingImageId)->SetVisibility(false);

  // establish the new backing as visbile
  currentBackingSeries = series;
  if (currentBackingSeries != NULL) {
    // now make sure the new backing has been added (if it hadn't already been)
    if (added_multireader_images.find(MultiReaderIDPair(pipeline->GetMultiImageReader(), series->imageID)) == added_multireader_images.end()) {
      series->image4IDanalyze = viewers->AddImage4(pipeline->GetMultiImageReader(), series->imageID, series->seriesType.toStdString());
      added_multireader_images.insert(MultiReaderIDPair(pipeline->GetMultiImageReader(), series->imageID));
    }
    backingImageId = series->image4IDanalyze;
    viewers->GetScene()->GetImage4(backingImageId)->SetVisibility(true);
    viewers->GetScene()->SetSelectedImage4(backingImageId);
    ebAssert(viewers && viewers->GetScene() && viewers->GetScene()->GetSelectedImage4());
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
    viewers->SetScreenScaleFitFactor2D(1.5);
    viewers->GetScene()->GetSelectedImage4()->SetWindowLevel(currentBackingSeries->window, currentBackingSeries->level);
    viewers->GetScene()->SetCursorPositionToSceneCenter();
    viewers->UpdateCameras();
    viewers->SyncViewersToScene();
    viewers->InitializeCameras();
    viewers->GetViewer3D(id3d)->SetPerspectiveDistance(.7*viewers->GetViewer3D(id3d)->GetPerspectiveDistance());
    viewers->Render();

    summaryTools->setBackingImageId(backingImageId);
    sliceTools->setBackingImageId(backingImageId);
  }
}

bool patientAnalyze::eventFilter(QObject *obj, QEvent *event) 
{ 
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // see if the event was one of the small widgets
  if ((obj == ui->summaryAreaGraph1) && (event->type() == QEvent::MouseButtonPress)) {
    if (((QMouseEvent*)event)->button() == Qt::LeftButton) {
      // bring up the configuration selector
      structureSel->presentDialog();
      return true;
    }
  }
  if ((obj == ui->summaryAreaGraph2) && (event->type() == QEvent::MouseButtonPress)) {
    if (((QMouseEvent*)event)->button() == Qt::LeftButton) {
      // bring up the configuration selector
      compositionSel->presentDialog();
      return true;
    }
  }
  if ((obj == ui->sliceAreaRender1) && (event->type() == QEvent::MouseButtonPress)) {
    bool cornersDisplayState = viewers->GetViewer2D(ebvIdSliceAreaRender0)->GetShowSceneCornerAnnotations();
    if (((QMouseEvent*)event)->button() == Qt::LeftButton) {
      double screenScale = viewers->GetViewer2D(ebvIdSliceAreaRender0)->GetScreenScale();
      // swap 1 and 0
      viewers->RemoveViewer(ebvIdSliceAreaRender0);
      viewers->RemoveViewer(ebvIdSliceAreaRender1);
        
      ebvViewer::ViewerType typeSave = sliceAreaRender0Type;
      ebvIdSliceAreaRender0 = viewers->AddViewer(sliceAreaRender1Type,ui->sliceAreaRender0->GetRenderWindow());
      viewers->GetViewer(ebvIdSliceAreaRender0)->SetShowLogo(true);
      sliceAreaRender0Type = sliceAreaRender1Type;
      ebvIdSliceAreaRender1 = viewers->AddViewer(typeSave,ui->sliceAreaRender1->GetRenderWindow());
      viewers->GetViewer(ebvIdSliceAreaRender1)->SetShowLogo(true);
      sliceAreaRender1Type = typeSave;
      sliceTools->resetViewer(ebvIdSliceAreaRender0, sliceAreaRender0Type);

      viewers->GetViewer2D(ebvIdSliceAreaRender0)->SetShowSceneCornerAnnotations(cornersDisplayState);
      viewers->GetViewer2D(ebvIdSliceAreaRender1)->SetShowSceneCornerAnnotations(false);
      viewers->GetViewer2D(ebvIdSliceAreaRender2)->SetShowSceneCornerAnnotations(false);
      addViewersLegend();
      viewers->SyncViewersToScene();
      viewers->SetScreenScale2D(screenScale);
      viewers->Render();
      return true;
    }
  }
  if ((obj == ui->sliceAreaRender2) && (event->type() == QEvent::MouseButtonPress)) {
    bool cornersDisplayState = viewers->GetViewer2D(ebvIdSliceAreaRender0)->GetShowSceneCornerAnnotations();
    if (((QMouseEvent*)event)->button() == Qt::LeftButton) {
      double screenScale = viewers->GetViewer2D(ebvIdSliceAreaRender0)->GetScreenScale();
      // swap 2 and 0
      viewers->RemoveViewer(ebvIdSliceAreaRender0);
      viewers->RemoveViewer(ebvIdSliceAreaRender2);
        
      ebvViewer::ViewerType typeSave = sliceAreaRender0Type;
      ebvIdSliceAreaRender0 = viewers->AddViewer(sliceAreaRender2Type,ui->sliceAreaRender0->GetRenderWindow());
      viewers->GetViewer(ebvIdSliceAreaRender0)->SetShowLogo(true);
      sliceAreaRender0Type = sliceAreaRender2Type;
      ebvIdSliceAreaRender2 = viewers->AddViewer(typeSave,ui->sliceAreaRender2->GetRenderWindow());
      viewers->GetViewer(ebvIdSliceAreaRender2)->SetShowLogo(true);
      sliceAreaRender2Type = typeSave;
      sliceTools->resetViewer(ebvIdSliceAreaRender0, sliceAreaRender0Type);

      viewers->GetViewer2D(ebvIdSliceAreaRender0)->SetShowSceneCornerAnnotations(cornersDisplayState);
      viewers->GetViewer2D(ebvIdSliceAreaRender1)->SetShowSceneCornerAnnotations(false);
      viewers->GetViewer2D(ebvIdSliceAreaRender2)->SetShowSceneCornerAnnotations(false);
      addViewersLegend();
      viewers->SyncViewersToScene();
      viewers->SetScreenScale2D(screenScale);
      viewers->Render();
      return true;
    }
  }

  // reach here because it wasn't one of the conditions we were looking for in this filter
  return false;
}

void patientAnalyze::saveToReport(ebvViewer::ViewerType viewerType, bool forceSliceViewerForTarget, bool includeReportUpdate)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  workItemListEntry *wi = owner->getWorkItem(sessionItemIndex)->selectedWorkItemListEntryPtr;
  if (!wi->setWorkItemFolder()) { // this may be the first time an operation that requires the folder has been done
    message->showMessage(tr("Error: Cannot write work item folder."));
    return;
  }
  if (!currentTarget->setTargetFolder(wi->getWorkItemFolder())) {
    message->showMessage(tr("Error: Cannot write target folder."));
    return;
  }

  // set a filename
  QString snapshotFilePath = currentTarget->getTargetFolder();
  snapshotFilePath.append("/");
  QUuid uid = QUuid::createUuid();
  QString snapshotFileName = uid.toString().mid(1,36).toUpper();

  snapshotFileName.append(".png");
  snapshotFilePath.append(snapshotFileName);

  // turn off corner annotations (they can't be read and aren't needed anyway since the report has more expansive tables), but save state to restore
  bool cornersDisplayState2D = viewers->GetViewer2D(ebvIdSliceAreaRender0)->GetShowSceneCornerAnnotations();
  bool cornersDisplayState3D = viewers->GetViewer3D(id3d)->GetShowSceneCornerAnnotations();
  viewers->GetViewer2D(ebvIdSliceAreaRender0)->SetShowSceneCornerAnnotations(false);
  viewers->GetViewer3D(id3d)->SetShowSceneCornerAnnotations(false);

  // now take the picture from the desired viewer
  if (forceSliceViewerForTarget)
    viewers->GetViewer(ebvIdSliceAreaRender0)->SaveRendering(snapshotFilePath.toStdString());
  else
    viewers->GetViewer((viewerType == ebvViewer::THREED) ? id3d : ebvIdSliceAreaRender0)->SaveRendering(snapshotFilePath.toStdString());
  currentTarget->keyImages.append(snapshotFilePath);

  // set tags in place to indicate that this should be included in report
  this->setEnabled(false);
  this->repaint(); // ensure that disabled status is shown
  qApp->processEvents();
  ebAssert(viewers && viewers->GetScene() && viewers->GetScene()->GetSelectedVesselTarget());
  viewers->GetScene()->GetSelectedVesselTarget()->GetVesselTarget()->AddTag("INCLUDEINREPORT");
  if (viewerType == ebvViewer::THREED) {
    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetReadings()) {
      viewers->GetScene()->GetSelectedVesselTarget()->GetVesselTarget()->AddKeyImage(snapshotFileName.toStdString());
      newTargetReadingsFlag = true;
    }
  }
  else {
    viewers->GetScene()->GetSelectedVesselTarget()->GetCurrentVessel()->AddVesselTag("INCLUDEINREPORT");
    viewers->GetScene()->GetSelectedVesselTarget()->GetCurrentSegment()->AddSegmentTag("INCLUDEINREPORT");
    viewers->GetScene()->GetSelectedVesselTarget()->GetCurrentCrossSection()->AddTag("INCLUDEINREPORT");
    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetReadings()) {
      viewers->GetScene()->GetSelectedVesselTarget()->GetCurrentCrossSection()->AddKeyImage(snapshotFileName.toStdString());
      newTargetReadingsFlag = true;
    }
  }

  // must make sure that the key image also makes it to the lesion readings
  if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLesionReadings()) {
    pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLesionReadings();
    owner->getTargetDefine(sessionItemIndex)->computeRemainingStages(currentTarget);
    newLesionReadingsFlag = true;
  }

  // restore the corner annotations, log the update, and return
  viewers->GetViewer2D(ebvIdSliceAreaRender0)->SetShowSceneCornerAnnotations(cornersDisplayState2D);
  viewers->GetViewer3D(id3d)->SetShowSceneCornerAnnotations(cornersDisplayState3D);
  QString step = "patientAnalyze::saveToReport"; // log the update
  emit logUpdate(step, wi->getWorkItemID(), NULL, sessionItemIndex);
  currentTarget->unsavedChanges = true;
  if (includeReportUpdate) {
    QString currentSave = QDir::currentPath();
    if (!updateServer()) {
        message->showMessage(tr("Error: Cannot write work item folder while trying to save key image."));
        QDir::setCurrent(currentSave);
        newTargetReadingsFlag = false;
        newLesionReadingsFlag = false;
        this->setEnabled(true);
        return;
    }
  }
  newTargetReadingsFlag = false;
  newLesionReadingsFlag = false;
  this->setEnabled(true);
  this->repaint(); // ensure that enabled status is shown
  qApp->processEvents();
}

void patientAnalyze::toggleCorner(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  viewers->GetViewer3D(id3d)->SetShowSceneCornerAnnotations(checked);
  viewers->GetViewer2D(ebvIdSliceAreaRender0)->SetShowSceneCornerAnnotations(checked);
  viewers->GetViewer2D(ebvIdSliceAreaRender1)->SetShowSceneCornerAnnotations(false);
  viewers->GetViewer2D(ebvIdSliceAreaRender2)->SetShowSceneCornerAnnotations(false);
  viewers->SyncViewersToScene();
  viewers->Render();
}

void patientAnalyze::toggleColorBar(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  // first the color control, if it is currently in use
  if (checked && currentTarget->parameters->compositionControlCurrentlyInUse)
    currentTarget->parameters->compositionControl->Dialog->show();
  else if (currentTarget->parameters->compositionControlCurrentlyInUse)
    currentTarget->parameters->compositionControl->Dialog->hide();

  // next the composition objects themselves
  for (int i=0; i < targets->size(); i++) {
    targetDef *def = &((*targets)[i]);
    for (auto analyteSegID : def->compositionSegIDsAnalyze)
      viewers->GetScene()->GetSegmentation4(analyteSegID)->SetVisibility(checked);
  }
  viewers->SyncViewersToScene();
  viewers->Render();
}

void patientAnalyze::toggleOblique(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << (checked ? "OBLIQUE/CPR" : "AXIS-ALIGNED") << std::endl;
  bool cornersDisplayState = viewers->GetViewer2D(ebvIdSliceAreaRender0)->GetShowSceneCornerAnnotations();
  double screenScale = viewers->GetViewer2D(ebvIdSliceAreaRender0)->GetScreenScale();
  viewers->RemoveViewer(ebvIdSliceAreaRender0);
  viewers->RemoveViewer(ebvIdSliceAreaRender1);
  viewers->RemoveViewer(ebvIdSliceAreaRender2);
  if (checked) {
    // make it oblique
    sliceAreaRender0Type = ebvViewer::OBLIQUE;
    ebvIdSliceAreaRender0 = viewers->AddViewer(sliceAreaRender0Type,ui->sliceAreaRender0->GetRenderWindow());
    viewers->GetViewer(ebvIdSliceAreaRender0)->SetShowLogo(true);
    sliceAreaRender1Type = ebvViewer::OBLIQUEPERP1;
    ebvIdSliceAreaRender1 = viewers->AddViewer(sliceAreaRender1Type,ui->sliceAreaRender1->GetRenderWindow());
    viewers->GetViewer(ebvIdSliceAreaRender1)->SetShowLogo(true);
    sliceAreaRender2Type = ebvViewer::CPR;
    ebvIdSliceAreaRender2 = viewers->AddViewer(sliceAreaRender2Type,ui->sliceAreaRender2->GetRenderWindow());
    viewers->GetViewer(ebvIdSliceAreaRender2)->SetShowLogo(true);
    viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::AXIAL, false);
    viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::CORONAL, false);
    viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::SAGITTAL, false);
    viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::OBLIQUEPERP2, false);
    if (summaryTools->ToggleAxialTool->isChecked())
      viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::OBLIQUE, summaryTools->ToggleAxialTool->isChecked());
    else
      viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::OBLIQUE, false);
    if (summaryTools->ToggleCoronalTool->isChecked())
      viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::OBLIQUEPERP1, summaryTools->ToggleCoronalTool->isChecked());
    else
      viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
    if (summaryTools->ToggleSagittalTool->isChecked()) 
      viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::CPR, summaryTools->ToggleSagittalTool->isChecked());
    else 
      viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::CPR, false);
  }
  else {
    // make it aligned
    sliceAreaRender0Type = ebvViewer::AXIAL;
    ebvIdSliceAreaRender0 = viewers->AddViewer(sliceAreaRender0Type,ui->sliceAreaRender0->GetRenderWindow());
    viewers->GetViewer(ebvIdSliceAreaRender0)->SetShowLogo(true);
    sliceAreaRender1Type = ebvViewer::CORONAL;
    ebvIdSliceAreaRender1 = viewers->AddViewer(sliceAreaRender1Type,ui->sliceAreaRender1->GetRenderWindow());
    viewers->GetViewer(ebvIdSliceAreaRender1)->SetShowLogo(true);
    sliceAreaRender2Type = ebvViewer::SAGITTAL;
    ebvIdSliceAreaRender2 = viewers->AddViewer(sliceAreaRender2Type,ui->sliceAreaRender2->GetRenderWindow());
    viewers->GetViewer(ebvIdSliceAreaRender2)->SetShowLogo(true);
    if (summaryTools->ToggleAxialTool->isChecked())
      viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::AXIAL, summaryTools->ToggleAxialTool->isChecked());
    else
      viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::AXIAL, false);
    if (summaryTools->ToggleCoronalTool->isChecked())
      viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::CORONAL, summaryTools->ToggleCoronalTool->isChecked());
    else
      viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::CORONAL, false);
    if (summaryTools->ToggleSagittalTool->isChecked())
      viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::SAGITTAL, summaryTools->ToggleSagittalTool->isChecked());
    else
      viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::SAGITTAL, false);
    viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::OBLIQUE, false);
    viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
    viewers->GetViewer3D(id3d)->SetShowSlice(ebvViewer::CPR, false);
    qDebug() << "turned CPR plane OFF, state is " << viewers->GetViewer3D(id3d)->GetShowSlice(ebvViewer::CPR);
  }
  sliceTools->resetViewer(ebvIdSliceAreaRender0, sliceAreaRender0Type);
  viewers->GetViewer3D(id3d)->SetShowSceneCornerAnnotations(cornersDisplayState);
  viewers->GetViewer2D(ebvIdSliceAreaRender0)->SetShowSceneCornerAnnotations(cornersDisplayState);
  viewers->GetViewer2D(ebvIdSliceAreaRender1)->SetShowSceneCornerAnnotations(false);
  viewers->GetViewer2D(ebvIdSliceAreaRender2)->SetShowSceneCornerAnnotations(false);
  summaryTools->ToggleObliqueTool->setChecked(checked);
  sliceTools->ToggleObliqueTool->setChecked(checked);
  addViewersLegend();
  viewers->SyncViewersToScene();
  viewers->SetScreenScale2D(screenScale);
  viewers->Render();
}

void patientAnalyze::giveToDefineIfNotAlreadyThere()
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (ui->seriesSelectionArea->count() != 0) { // if not 0, then the screen is currently on patientAnalyze, so must give it to targetDefine
    //ebAssert(viewers && viewers->GetScene() && viewers->GetScene()->GetSelectedImage4());
    //viewers->GetScene()->GetSelectedImage4()->GetWindowLevel(currentBackingSeries->window, currentBackingSeries->level);
    //qDebug() << "... window stored as" << currentBackingSeries->window << " and level stored as" << currentBackingSeries->level;
    dismissCompositionControls();
    emit giveScreenControlToDefineFromAnalyze(ui->seriesSelectionArea, currentBackingSeries, currentTarget);
  }
}

void patientAnalyze::toggleSegmentation(QString regName, bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // loop through targets, because the segmentation toggles effect all of them
  for (int i=0; i < targets->size(); i++) {
    // go through multiple levels of check to see if the specification is complete enough to add
    targetDef *def = &((*targets)[i]);
    // have to do this in layers, since partitions override segmentations
    if (regName.contains("lumen")) {
      if (def->lumenSegIDanalyze != null_ebID)
        viewers->GetScene()->GetSegmentation4(def->lumenSegIDanalyze)->SetVisibility(checked);
      if (!def->lumenPartIDsAnalyze.empty()) {
        for (auto partitionID : def->lumenPartIDsAnalyze) {
          viewers->GetScene()->GetSegmentation4(partitionID)->SetVisibility(checked);
        }
      }
    }
    if (regName.contains("wall")) {
      if (def->wallSegIDanalyze != null_ebID) 
        viewers->GetScene()->GetSegmentation4(def->wallSegIDanalyze)->SetVisibility(checked);
      if (!def->wallPartIDsAnalyze.empty()) {
        for (auto partitionID : def->wallPartIDsAnalyze) {
          viewers->GetScene()->GetSegmentation4(partitionID)->SetVisibility(checked);
        }
      }
    }
  } // end-for each listed target
  viewers->SyncViewersToScene();
  viewers->Render();
}

bool patientAnalyze::jumpTo(QString tag)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  bool foundtag = viewers->GetScene()->JumpOnPathToTag(tag.toStdString());
  // set 2D zoom
  viewers->GetViewer2D(ebvIdSliceAreaRender0)->SetParallelScale(30);
  viewers->MatchScreenScale2D(viewers->GetViewer2D(ebvIdSliceAreaRender0));
  viewers->SyncViewersToScene();
  viewers->Render();
  return foundtag;
}

void patientAnalyze::on_backToDefineButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  this->setEnabled(false);
  disableMenuActions();
  giveToDefineIfNotAlreadyThere();
  this->setEnabled(true);
}

void patientAnalyze::dismissCompositionControls()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  for (int i=0; i < targets->size(); i++) {
    // go through multiple levels of check to see if the specification is complete enough to add
    targetDef *def = &((*targets)[i]);
    if (def->parameters != NULL) {
      if (def->parameters->compositionControl != NULL) {
        def->parameters->compositionControl->cancelCompositionSetting();
      }
    }
  }
}

void patientAnalyze::markLesion()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->getTargetDefine(sessionItemIndex)->giveToAnalyzeIfNotAlreadyThere();
  owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(PATIENTANALYZE);

  // must revert to the readings (rather than the lesion readings) 
  if (currentTarget->lesionReadingsIDanalyze != null_ebID) {
    viewers->RemoveVesselTarget(currentTarget->lesionReadingsIDanalyze); 
    currentTarget->lesionReadingsIDanalyze = null_ebID;
    currentTarget->readingsIDanalyze = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetReadings(), "readings"); 
  }

  // enter marking mode to collect points
  ui->backToDefineButton->setEnabled(false); // can't go back when marking lesions
  ui->proceedToReportButton->setEnabled(false);
  viewers->StartLesionMarking();
  sliceTools->TargetCreateTool->setChecked(true);
  ENABLECONTROL(sliceTools->TargetCreateTool, sliceTools->TargetCreateAction, tr("Press to end point collection"));
}

void patientAnalyze::clearLesions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->getTargetDefine(sessionItemIndex)->giveToAnalyzeIfNotAlreadyThere();
  owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(PATIENTANALYZE);

  // call the ebi functions to clear out the lesion objects in reverse order. 
  this->setEnabled(false);
  if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLesionReadings()) {
    pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLesionReadings();
  }
  if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLesionLumenAndWallPartition()) {
    if (!currentTarget->lesionLumenAndWallPartIDsAnalyze.empty()) {
      for (auto partitionID : currentTarget->lesionLumenAndWallPartIDsAnalyze) {
        viewers->RemoveSegmentation4(partitionID);
      }
    }
    currentTarget->lesionLumenAndWallPartIDsAnalyze.clear();
    pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLesionLumenAndWallPartition();
  }
  if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLesionLumenPartition()) {
    if (!currentTarget->lesionLumenPartIDsAnalyze.empty()) {
      for (auto partitionID : currentTarget->lesionLumenPartIDsAnalyze) {
        viewers->RemoveSegmentation4(partitionID);
      }
    }
    currentTarget->lesionLumenPartIDsAnalyze.clear();
    pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLesionLumenPartition();
  }
                  
  // must revert to the readings (rather than the lesion readings) 
  if (currentTarget->lesionReadingsIDanalyze != null_ebID) {
    viewers->RemoveVesselTarget(currentTarget->lesionReadingsIDanalyze); 
    currentTarget->lesionReadingsIDanalyze = null_ebID;
    currentTarget->readingsIDanalyze = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetReadings(), "readings"); 
  }

  // now that lesion-derived objects are gone, clear the lesion markings themselves
  pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetReadings()->ClearLesions(); //the lesion markings themselves are actually stored in the pipeline readings object
  owner->getTargetDefine(sessionItemIndex)->computeRemainingStages(currentTarget); // needed to update the new readings file

  // log the update, which will force new saves as needed, update displays, and flush report data
  owner->getTargetDefine(sessionItemIndex)->logUpdateWithStageSettings("patientAnalyze::clearLesions", currentTarget, currentTarget->getID());
  currentTarget->unsavedChanges = true;
  enableControlsAsAppropriateGivenATargetFocus();
  newTargetReadingsFlag = true;
  QMessageBox msgBox(owner);
  msgBox.setText(tr("New Key Images will now be taken (to ensure that the cleared lesions do not appear)."));
  msgBox.setInformativeText(tr("You will see the screen flicker multiple times (this is normal)."));
  msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
  msgBox.exec();
  displayTargetsAndAnticipateReportContent();
  this->setEnabled(true);
}

void patientAnalyze::labelMarkedLesion()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QString lesionName;
  if (!viewers->LesionMarkingEmpty()) {
    bool ok;
    while (1) {
      lesionName=QInputDialog::getText(this, tr("Lesion Label"), tr("Lesion name:"), QLineEdit::Normal, "", &ok);
      if (ok && !lesionName.isEmpty())
              break;
      else {
              // need to see if they want to abandon or want to give a name
              QMessageBox msgBox(owner);
              msgBox.setText(tr("The name you gave was blank, or you hit cancel."));
              msgBox.setInformativeText(tr("Do you want to abandon the lesion marking? (Yes means the lesion will be abandoned, No means you wish to type a non-blank name.)"));
              msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
              msgBox.setDefaultButton(QMessageBox::Yes);
              msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
              int ret = msgBox.exec();
              if (ret == QMessageBox::Yes)  
                break; 
      }
    }
  }

  // turn off the marking mode which causes the lesion data structures to be established
  viewers->StopLesionMarking(lesionName.toStdString());
  ui->backToDefineButton->setEnabled(true); // now can go back

  // log the update and set the flag
  if (lesionName != "") {
    this->setEnabled(false);
    
    // close any prior computations
    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLesionLumenPartition() != NULL) {
      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLesionLumenPartition();
      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLesionLumenAndWallPartition();
      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLesionPerivascularRegionPartition();
      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLesionReadings();
    }
    
    // perform the computations
    owner->getTargetDefine(sessionItemIndex)->computeRemainingStages(currentTarget);
    
    // log the update, which will force new saves as needed, update displays, and flush report data
    owner->getTargetDefine(sessionItemIndex)->logUpdateWithStageSettings("patientAnalyze::markLesion", currentTarget, currentTarget->getID()+":"+lesionName);
    currentTarget->unsavedChanges = true;
    enableControlsAsAppropriateGivenATargetFocus();
    newLesionReadingsFlag = true;
    QMessageBox msgBox(owner);
    msgBox.setText(tr("New Key Images will now be taken (to ensure that the marked lesion appears)."));
    msgBox.setInformativeText(tr("You will see the screen flicker multiple times (this is normal)."));
    msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    msgBox.exec();
    displayTargetsAndAnticipateReportContent();
    this->setEnabled(true);
  }
  sliceTools->TargetCreateTool->setChecked(false);
  ENABLECONTROL(sliceTools->TargetCreateTool, sliceTools->TargetCreateAction, tr("Press to create a new target"));
  ui->proceedToReportButton->setEnabled(true);
}

/** @} */
