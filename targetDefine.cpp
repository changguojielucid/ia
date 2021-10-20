// Copyright (c) Elucid Bioimaging

#include "targetDefine.h"
#include "ui_targetDefine.h"
#include "workItemListFileToken.h"
#include "cap.h"
#include "QITKProgressDialog.h"

#include "ebiVesselTargetPipeline.h"
#include "ebiImageHelper.h"
#include "ebvOutputWindow.h"
#include "ebvViewer2D.h"
#include "ebAssert.h"

#include <QWidget>
#include <QString>
#include <QInputDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QList>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QMessageBox>

#include <iostream>
#include <string>
#include <vector>

/**
 * \ingroup targetDefine
 * @{
 *
 * See targetDefine.h for description of the package purpose and contents.  This file has the member functions for classes in the package.
 */

void targetDef::closeOutCompletedSession()
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // we keep all the file-oriented objects, but null out the pipeline IDs
  pipeline = NULL;
  isViable = false;
  unsavedChanges = false;
  targetPipelineID = null_ebID;
  initializerIDdefine = null_ebID; initializerIDanalyze = null_ebID;
  pathIDdefine = null_ebID; pathIDanalyze = null_ebID;
  readingsIDdefine = null_ebID; readingsIDanalyze = null_ebID;
  lesionReadingsIDdefine = null_ebID; lesionReadingsIDanalyze = null_ebID;
  lumenSegIDdefine = null_ebID; lumenSegIDanalyze = null_ebID;
  wallSegIDdefine = null_ebID; wallSegIDanalyze = null_ebID;
  periRegIDdefine = null_ebID; periRegIDanalyze = null_ebID;
  wallThickIDdefine = null_ebID; wallThickIDanalyze = null_ebID;
  capThickIDdefine = null_ebID; capThickIDanalyze = null_ebID;
  lumenPartIDsDefine.clear(); lumenPartIDsAnalyze.clear(); 
  wallPartIDsDefine.clear(); wallPartIDsAnalyze.clear(); 
  periRegPartIDsDefine.clear(); periRegPartIDsAnalyze.clear(); 
  lesionLumenPartIDsDefine.clear(); lesionLumenAndWallPartIDsDefine.clear(); lesionPeriRegPartIDsDefine.clear();
  lesionLumenPartIDsAnalyze.clear(); lesionLumenAndWallPartIDsAnalyze.clear(); lesionPeriRegPartIDsAnalyze.clear();
  /*compositionImageIDsAnalyze.clear();*/ compositionSegIDsAnalyze.clear();
  if (parameters) {
    if (parameters->compositionControl->Dialog)
      parameters->compositionControl->cancelCompositionSetting();
    delete parameters;
    parameters = NULL;
  }
}

targetDef::targetDef() :
  ID(QString ("")),
  bodySite(QString (""))
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  parameters = NULL;
  closeOutCompletedSession();
  probabilityMaps.clear();
  regions.clear();
  valueMaps.clear();
  keyImages.clear();
  targetFolder = "";
  readingsLocalFileName = ""; 
  lesionReadingsLocalFileName = ""; 
  targetPathFileName = "";
  registrationTransforms = "";
}

targetDef::targetDef(const QString &id, const QString &site) :
  ID(id),
  bodySite(site)
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  parameters = NULL;
  closeOutCompletedSession();
  probabilityMaps.clear();
  regions.clear();
  valueMaps.clear();
  keyImages.clear();
  targetFolder = "";
  readingsLocalFileName = ""; 
  lesionReadingsLocalFileName = ""; 
  targetPathFileName = "";
  registrationTransforms = "";
}

targetDef::~targetDef()
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  regions.clear();
  probabilityMaps.clear();
  valueMaps.clear();
}

void targetDef::saveTargetDerivedDataChanges(QString baseFolder)
{
  ebLog eblog(Q_FUNC_INFO); eblog << baseFolder.toStdString() << std::endl;
  if (unsavedChanges) {
    // first, scrub all target derived data objects as the correct state will be saved by virtue of what follows
    setTargetInitializerFileName("");
    setTargetPathFileName("");
    setTargetRegistrationTransforms("");
    setTargetReadingsFileName("");
    setLesionReadingsFileName("");
    regions.clear();
    valueMaps.clear();
    probabilityMaps.clear();

    // set up a file name prefix
    QString prefix = getTargetFolder();
    prefix.append("/");
    prefix.append( getLocalUserName() );
    prefix.append("_");
    QDateTime dt = QDateTime::currentDateTime();
    QString dtStr = dt.toString("yyyyMMdd-hhmm");
    QStringList dtSubStr = dtStr.split('-');
    prefix.append(dtSubStr.at(0));
    prefix.append("_");

    // now begin a nested sequence of save operations based on pipeline state
    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetInitialization()) {
      QString targetInitializer = prefix;
      targetInitializer.append("_initializer.json");
      setTargetInitializerFileName(targetInitializer);
      QString targetInitializerFull = baseFolder;
      targetInitializerFull.append("/");
      targetInitializerFull.append(getTargetInitializerFileName());
      pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveInitialization(targetInitializerFull.toStdString()); 
    
      if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenSegmentation()) {
        QString lumenSegFileName = prefix;
        lumenSegFileName.append("_lumenSegmentation.nrrd");
        region lumenSeg("lumenSegmentation", lumenSegFileName);
        QString lumenSegFileNameFull = baseFolder;
        lumenSegFileNameFull.append("/");
        lumenSegFileNameFull.append(lumenSeg.getRegFileName());
        pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLumenSegmentation(lumenSegFileNameFull.toStdString());
        regions.append(lumenSeg);

        if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetPath()) {
          QString targetPath = prefix;
          targetPath.append("_path.json");
          setTargetPathFileName(targetPath);
          QString targetPathFull = baseFolder;
          targetPathFull.append("/");
          targetPathFull.append(getTargetPathFileName());
          pipeline->GetVesselTargetPipeline(targetPipelineID)->SavePath(targetPathFull.toStdString()); 

          if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenPartition()) {
            QString lumenPartFileName = prefix;
            lumenPartFileName.append("_lumenPartition.multi.nrrd");
            region lumenPart("lumenPartition", lumenPartFileName);
            QString lumenPartFileNameFull = baseFolder;
            lumenPartFileNameFull.append("/");
            lumenPartFileNameFull.append(lumenPart.getRegFileName());
            pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLumenPartition(lumenPartFileNameFull.toStdString());
            regions.append(lumenPart);

            if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetResampledRegisteredImages()) {
              QString registrationTransforms = prefix;
              registrationTransforms.append("_transforms.json");
              setTargetRegistrationTransforms(registrationTransforms);
              QString registrationTransformsFull = baseFolder;
              registrationTransformsFull.append("/");
              registrationTransformsFull.append(getTargetRegistrationTransforms());
              pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveRegistration(registrationTransformsFull.toStdString()); 

              if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenAndWallSegmentation()) {
                QString wallSegFileName = prefix;
                wallSegFileName.append("_wallSegmentation.nrrd");
                region wallSeg("wallSegmentation", wallSegFileName);
                QString wallSegFileNameFull = baseFolder;
                wallSegFileNameFull.append("/");
                wallSegFileNameFull.append(wallSeg.getRegFileName());
                pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLumenAndWallSegmentation(wallSegFileNameFull.toStdString());
                regions.append(wallSeg);

                if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLumenAndWallPartition()) {
                  QString wallPartFileName = prefix;
                  wallPartFileName.append("_wallPartition.multi.nrrd");
                  region wallPart("wallPartition", wallPartFileName);
                  QString wallPartFileNameFull = baseFolder;
                  wallPartFileNameFull.append("/");
                  wallPartFileNameFull.append(wallPart.getRegFileName());
                  pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLumenAndWallPartition(wallPartFileNameFull.toStdString());
                  regions.append(wallPart);

                  if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetWallThickness()) {
                    QString wallThickFileName = prefix;
                    wallThickFileName.append("_wallThickness.nrrd");
                    valueMap wallThick("wallThickness", wallThickFileName);
                    QString wallThickFileNameFull = baseFolder;
                    wallThickFileNameFull.append("/");
                    wallThickFileNameFull.append(wallThick.getValueMapFileName());
                    pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveWallThickness(wallThickFileNameFull.toStdString());
                    valueMaps.append(wallThick);

                    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetPerivascularRegion()) {
                      QString periRegFileName = prefix;
                      periRegFileName.append("_perivascularRegion.nrrd");
                      region periReg("perivascularRegion", periRegFileName);
                      QString periRegFileNameFull = baseFolder;
                      periRegFileNameFull.append("/");
                      periRegFileNameFull.append(periReg.getRegFileName());
                      pipeline->GetVesselTargetPipeline(targetPipelineID)->SavePerivascularRegion(periRegFileNameFull.toStdString());
                      regions.append(periReg);

                      if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetPerivascularRegionPartition()) {
                        QString periRegPartFileName = prefix;
                        periRegPartFileName.append("_periRegPartition.multi.nrrd");
                        region periRegPart("periRegPartition", periRegPartFileName);
                        QString periRegPartFileNameFull = baseFolder;
                        periRegPartFileNameFull.append("/");
                        periRegPartFileNameFull.append(periRegPart.getRegFileName());
                        pipeline->GetVesselTargetPipeline(targetPipelineID)->SavePerivascularRegionPartition(periRegPartFileNameFull.toStdString());
                        regions.append(periRegPart);

                        if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetComposition()) {
                          QString compositionFileName = prefix;
                          compositionFileName.append("_composition.multi.nrrd");
                          probabilityMap composition("composition", compositionFileName);
                          QString compositionFileNameFull = baseFolder;
                          compositionFileNameFull.append("/");
                          compositionFileNameFull.append(composition.getProbabilityMapFileName());
                          pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveComposition(compositionFileNameFull.toStdString());
                          probabilityMaps.append(composition);

                          if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetCapThickness()) {
                            QString capThickFileName = prefix;
                            capThickFileName.append("_capThickness.nrrd");
                            valueMap capThick("capThickness", capThickFileName);
                            QString capThickFileNameFull = baseFolder;
                            capThickFileNameFull.append("/");
                            capThickFileNameFull.append(capThick.getValueMapFileName());
                            pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveCapThickness(capThickFileNameFull.toStdString());
                            valueMaps.append(capThick);

                            if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetReadings()) {
                              QString readingsLocalFileName = prefix;
                              readingsLocalFileName.append("_readings.json");
                              setTargetReadingsFileName(readingsLocalFileName);
                              QString readingsLocalFileNameFull = baseFolder;
                              readingsLocalFileNameFull.append("/");
                              readingsLocalFileNameFull.append(getTargetReadingsFileName());
                              pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveReadings(readingsLocalFileNameFull.toStdString());

                              if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionLumenPartition()) {
                                QString lesionLumenPartitionFileName = prefix;
                                lesionLumenPartitionFileName.append("_lesionLumenPartition.multi.nrrd");
                                region lesionLumenPart("lesionLumenPartition", lesionLumenPartitionFileName);
                                QString lesionLumenPartFileNameFull = baseFolder;
                                lesionLumenPartFileNameFull.append("/");
                                lesionLumenPartFileNameFull.append(lesionLumenPart.getRegFileName());
                                pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLesionLumenPartition(lesionLumenPartFileNameFull.toStdString());
                                regions.append(lesionLumenPart);

                                if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionLumenAndWallPartition()) {
                                  QString lesionLumenAndWallPartitionFileName = prefix;
                                  lesionLumenAndWallPartitionFileName.append("_lesionLumenAndWallPartition.multi.nrrd");
                                  region lesionLumenAndWallPart("lesionLumenAndWallPartition", lesionLumenAndWallPartitionFileName);
                                  QString lesionLumenAndWallPartFileNameFull = baseFolder;
                                  lesionLumenAndWallPartFileNameFull.append("/");
                                  lesionLumenAndWallPartFileNameFull.append(lesionLumenAndWallPart.getRegFileName());
                                  pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLesionLumenAndWallPartition(lesionLumenAndWallPartFileNameFull.toStdString());
                                  regions.append(lesionLumenAndWallPart);

                                  if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionPerivascularRegionPartition()) {
                                    QString lesionPerivascularRegionPartitionFileName = prefix;
                                    lesionPerivascularRegionPartitionFileName.append("_lesionPerivascularRegionPartition.multi.nrrd");
                                    region lesionPerivascularRegionPart("lesionPerivascularRegionPartition", lesionPerivascularRegionPartitionFileName);
                                    QString lesionPerivascularRegionPartFileNameFull = baseFolder;
                                    lesionPerivascularRegionPartFileNameFull.append("/");
                                    lesionPerivascularRegionPartFileNameFull.append(lesionPerivascularRegionPart.getRegFileName());
                                    pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLesionPerivascularRegionPartition(lesionPerivascularRegionPartFileNameFull.toStdString());
                                    regions.append(lesionPerivascularRegionPart);

                                    if (pipeline->GetVesselTargetPipeline(targetPipelineID)->GetLesionReadings()) {
                                      QString lesionReadingsLocalFileName = prefix;
                                      lesionReadingsLocalFileName.append("_lesionReadings.json");
                                      setLesionReadingsFileName(lesionReadingsLocalFileName);
                                      QString lesionReadingsLocalFileNameFull = baseFolder;
                                      lesionReadingsLocalFileNameFull.append("/");
                                      lesionReadingsLocalFileNameFull.append(getLesionReadingsFileName());
                                      pipeline->GetVesselTargetPipeline(targetPipelineID)->SaveLesionReadings(lesionReadingsLocalFileNameFull.toStdString());
                                    } // end-if there are lesionReadings
                                  } // end-if there are lesionPerivascularREgionPartitions
                                } // end-if there are lesionLumenAndWallPartitions
                              } // end-if there are lesionLumenPartitions
                            } // end-if there are readings
                          } // end-if cap thickness has been computed
                        } // end-if there is composition
                      } // end-if the perivascular region has been partitioned
                    } // end-if perivascular region has been established
                  } // end-if wall thickness has been computed
                } // end-if the wall has been partitioned
              } // end-if there is a wall segmentation
            } // end-if registration transforms have been computed
          } // end-if the lumen has been partitioned
        } // end-if the path has been determined
      } // end-if the lumen has been segmented
    } // end-if there is an initializer
    
    unsavedChanges = false;
  }
}

targetDefine::targetDefine(QWidget *p, QMenu *m, bool masterWithRespectToMenu) :
  QWidget(p),
  ui(new Ui::targetDefine)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner = dynamic_cast<cap *>(p);
  targetDefineMenu = m;
  currentBackingSeries = NULL;
  currentTarget = NULL;
  viewers = NULL;
  images = NULL;
  targets = NULL;
  viableTargetsCount = 0;
  definingRootFlag = false;
  editingSegmentation = false;

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
    ui->processingParametersButton->setEnabled(false); 
    ui->backToSurveyButton->setText(QChar(0x25C0));
          QString buttonText = tr("Proceed to Analyze ");
          buttonText.append(QChar(0x25B6));
    ui->continueWithAnalysisButton->setText(buttonText);
    ui->continueWithAnalysisButton->setEnabled(false); 

    // remove the placeholder series selection pages from the designer (as a reverse of the steps to add)
    ui->seriesSelectionArea->removeWidget(ui->seriesSelectionArea->widget(1));
    ui->seriesSelectionArea->removeWidget(ui->seriesSelectionArea->widget(0));
  
    // add the viewers
    viewers = vtkSmartPointer<ebvLinkedViewers2>::New();
    ebvIdVolume = viewers->AddViewer(ebvViewer::THREED,ui->volumeRender->GetRenderWindow());
    viewers->GetViewer(ebvIdVolume)->SetShowLogo(true);
    viewers->GetViewer3D(ebvIdVolume)->SetShowVolume(false);
    viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, true);
    viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, true);
    viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, true);
    viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, false);
    viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
    viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, false);
    viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP2, false);
    ebvIdCoronal = viewers->AddViewer(ebvViewer::CORONAL,ui->coronalRender->GetRenderWindow());
    coronalType = ebvViewer::CORONAL;
    viewers->GetViewer(ebvIdCoronal)->SetShowLogo(true);    
    //viewers->GetViewer2D(ebvIdCoronal)->SetSlabLineVisible(0,false);
    ebvIdSagittal = viewers->AddViewer(ebvViewer::SAGITTAL,ui->sagittalRender->GetRenderWindow());
    sagittalType = ebvViewer::SAGITTAL;
    viewers->GetViewer(ebvIdSagittal)->SetShowLogo(true);    
    //viewers->GetViewer2D(ebvIdSagittal)->SetSlabLineVisible(0,false);
    ebvIdAxial = viewers->AddViewer(ebvViewer::AXIAL,ui->axialRender->GetRenderWindow());
    axialType = ebvViewer::AXIAL;
    viewers->GetViewer(ebvIdAxial)->SetShowLogo(true);
    //viewers->GetViewer2D(ebvIdAxial)->SetSlabLineVisible(0,false);
    viewers->GetScene()->SetImageSlabType(VTK_IMAGE_SLAB_MAX);
    viewers->GetScene()->SetSlabThickness(0);
    viewers->SyncViewersToScene();
    viewers->UpdateCameras();    
    viewers->SetVesselTargetInitializationIntensityWindow(owner->systemPreferencesObject->getTargetDefineIntensityWindow());   
    ui->coronalLabel->setText(tr("MIP enabled (set slab thickness as desired)"));
    ui->sagittalLabel->setText(tr("MIP enabled (set slab thickness as desired)"));
    ui->axialLabel->setText(tr("MIP enabled (set slab thickness as desired)"));
    vtkOutputWindow::SetInstance(ebvOutputWindow::New());
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
  }
  else {
    // this is needed to make the dummy object to establish the menu have a place to add the tool buttons (which won't be used, but need for resusing fucntion)
    ui->targetDefineControlsBox = new QWidget(0);
  }

  // segmentation editor
  segmentationEditor = new SegmentationEditor(this);
}

targetDefine::~targetDefine()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  delete segmentationEditor;
  if (owner != 0) {
    viewers->RemoveViewer(ebvIdVolume);
    viewers->RemoveViewer(ebvIdCoronal);
    viewers->RemoveViewer(ebvIdSagittal);
    viewers->RemoveViewer(ebvIdAxial);
    /*viewers = NULL;
    images = NULL;
    targets = NULL;
    
    delete message;*/
    delete ui; 
  }
}

void targetDefine::on_processingParametersButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  bool compositionControlCurrentlyInUse = false;
  for (int i=0; i < targets->size(); i++) {
    targetDef *def = &((*targets)[i]);
    if (def->parameters != NULL)
      if (def->parameters->compositionControlCurrentlyInUse)
        compositionControlCurrentlyInUse = true;
  }
  if (!compositionControlCurrentlyInUse)
    currentTarget->parameters->presentDialog();
  else
    message->showMessage(tr("Can't have both the Composition Control and the Processing Parameters Panel open at same time."));
}

void targetDefine::processingParametersSettingsChanged()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  clearTargetBeyondCurrentStage(currentTarget);
  currentTarget->unsavedChanges = true;
  logUpdateWithStageSettings("targetDefine::processingParametersSettingsChanged", currentTarget, currentTarget->getID());
  viewers->SyncViewersToScene();
  viewers->Render();
}

void targetDefine::presentTarget(targetDef *def)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  this->setEnabled(false);
  // work forward, as deep as can go based on what is in place (except not as far as patientAnalyze goes)
  if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization()) {
    def->initializerIDdefine = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization(), "initializer");
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation()) { 
      def->lumenSegIDdefine = viewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation(), 0, "lumenSegmentation"); 
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath()) {
        viewers->RemoveVesselTarget(def->initializerIDdefine); // if have a path, don't want to display the initializer too
        def->initializerIDdefine = null_ebID;
        def->pathIDdefine = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath(), "path"); 
        if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenPartition()) {
          viewers->RemoveSegmentation4(def->lumenSegIDdefine); // if have partitions, don't want to display the segmentation too
          def->lumenSegIDdefine = null_ebID;
          def->lumenPartIDsDefine.clear();
          int i = 0;
          for (auto partition : *pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenPartition())
            def->lumenPartIDsDefine.insert(viewers->AddSegmentation4(partition.GetPointer(), 0, "lumenPartition"+std::to_string(i++)));
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallSegmentation()) {
            def->wallSegIDdefine = viewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallSegmentation(), 0, "wallSegmentation");
            viewers->GetScene()->GetSegmentation4(def->wallSegIDdefine)->GetInteriorProperty()->SetOpacity(0);
            if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallPartition()) {
              viewers->RemoveSegmentation4(def->wallSegIDdefine); // if have partitions, don't want to display the segmentation too
              def->wallSegIDdefine = null_ebID;
              def->wallPartIDsDefine.clear();
              int i = 0;
              for (auto partition : *pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallPartition()) {
                ebID partID = viewers->AddSegmentation4(partition.GetPointer(), 0, "wallPartition"+std::to_string(i++));
                def->wallPartIDsDefine.insert(partID);
                viewers->GetScene()->GetSegmentation4(partID)->GetInteriorProperty()->SetOpacity(0);   
              }
            }
          }
        }
      }
    }
  } 
  viewers->SyncViewersToScene();
  viewers->Render();
  this->setEnabled(true);
}

void targetDefine::logUpdateWithStageSettings(QString step, targetDef *def, QString ID)
{
  ebLog eblog(Q_FUNC_INFO); eblog << (def->getID()+":"+step).toStdString() << std::endl;
  // determine the stageParams based on the step
  stageParameters *stageParams = NULL;
  /*if (step.contains("nitializ"))
    stageParams = def->parameters->parametersByStages.at(ebiVesselTargetPipeline::INITIALIZATION_STAGE);
  else */if (step.contains("ComputeLumenSegmentation"))
    stageParams = def->parameters->parametersByStages.at(ebiVesselTargetPipeline::LUMENSEGMENTATION_STAGE);
  else if (step.contains("ComputeVesselTargetPath"))
    stageParams = def->parameters->parametersByStages.at(ebiVesselTargetPipeline::PATH_STAGE);
  else if (step.contains("ComputeLumenPartition")) 
    stageParams = def->parameters->parametersByStages.at(ebiVesselTargetPipeline::LUMENPARTITION_STAGE);
  else if (step.contains("ComputeRegistration")) 
    stageParams = def->parameters->parametersByStages.at(ebiVesselTargetPipeline::IMAGEREGISTRATION_STAGE);
  else if (step.contains("ComputeLumenAndWallSegmentation")) 
    stageParams = def->parameters->parametersByStages.at(ebiVesselTargetPipeline::LUMENANDWALLSEGMENTATION_STAGE);
  else if (step.contains("ComputeLumenAndWallPartition")) 
    stageParams = def->parameters->parametersByStages.at(ebiVesselTargetPipeline::LUMENANDWALLPARTITION_STAGE);
  else if (step.contains("ComputeWallThickness")) 
    stageParams = def->parameters->parametersByStages.at(ebiVesselTargetPipeline::WALLTHICKNESS_STAGE); 
  else if (step.contains("ComputePerivascularRegion")) 
    stageParams = def->parameters->parametersByStages.at(ebiVesselTargetPipeline::PERIVASCULARREGION_STAGE);  
  else if (step.contains("ComputePerivascularRegionPartition")) 
    stageParams = def->parameters->parametersByStages.at(ebiVesselTargetPipeline::PERIVASCULARREGIONPARTITION_STAGE);
  else if (step.contains("ComputeComposition")) 
    stageParams = def->parameters->parametersByStages.at(ebiVesselTargetPipeline::COMPOSITION_STAGE);  
  else if (step.contains("ComputeCapThickness")) 
    stageParams = def->parameters->parametersByStages.at(ebiVesselTargetPipeline::CAPTHICKNESS_STAGE); 
  else if (step.contains("ComputeVesselTargetReadings")) 
    stageParams = def->parameters->parametersByStages.at(ebiVesselTargetPipeline::READINGS_STAGE);  
  else if (step.contains("ComputeLesionLumenPartition")) 
    stageParams = def->parameters->parametersByStages.at(ebiVesselTargetPipeline::LESIONLUMENPARTITION_STAGE); 
  else if (step.contains("ComputeLesionLumenAndWallPartition")) 
    stageParams = def->parameters->parametersByStages.at(ebiVesselTargetPipeline::LESIONLUMENANDWALLPARTITION_STAGE);  
  else if (step.contains("ComputeLesionPerivascularRegionPartition")) 
    stageParams = def->parameters->parametersByStages.at(ebiVesselTargetPipeline::LESIONPERIVASCULARREGIONPARTITION_STAGE);  
  else if (step.contains("ComputeLesionReadings")) 
    stageParams = def->parameters->parametersByStages.at(ebiVesselTargetPipeline::LESIONREADINGS_STAGE);  

  // next perform a deep copy since the provenance is a persistent record
  stageParameters *deepCopyOfStageParams = NULL;
  if (stageParams != NULL) {
    deepCopyOfStageParams = new stageParameters();
    deepCopyOfStageParams->version = stageParams->version;
    for (int i=0; i < stageParams->parameterKeys.size(); i++) {
      deepCopyOfStageParams->parameterKeys << stageParams->parameterKeys.at(i);
      deepCopyOfStageParams->settings << stageParams->settings.at(i);
    }
  }
  emit logUpdate(step, ID, deepCopyOfStageParams, sessionItemIndex);
}

bool targetDefine::updateServer()
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

bool targetDefine::computeRemainingStages(targetDef *def)
{
  ebLog eblog(Q_FUNC_INFO); eblog << def->getID().toStdString() << std::endl;
  QITKProgressDialog progressIndicator(0,0);
  progressIndicator.setLabelText(tr("Completing calculations..."));
  progressIndicator.setWindowModality(Qt::NonModal);
  progressIndicator.setMinimumDuration(10);
  progressIndicator.setWindowFlags(progressIndicator.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::WindowStaysOnTopHint);
  progressIndicator.setCancelButton(nullptr);   // no cancel button on dialog
  progressIndicator.show();
  progressIndicator.AddFilter(ebiVesselTargetPipeline::LUMENSEGMENTATION_STAGE,pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentationFilter(),2);
  progressIndicator.AddFilter(ebiVesselTargetPipeline::PATH_STAGE,pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPathFilter(),1);
  progressIndicator.AddFilter(ebiVesselTargetPipeline::LUMENPARTITION_STAGE,pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenPartitionFilter(),0.5);
  progressIndicator.AddFilter(ebiVesselTargetPipeline::IMAGEREGISTRATION_STAGE,pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetRegistrationFilter(),0.01);
  progressIndicator.AddFilter(ebiVesselTargetPipeline::LUMENANDWALLSEGMENTATION_STAGE,pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallSegmentationFilter(),3);
  progressIndicator.AddFilter(ebiVesselTargetPipeline::LUMENANDWALLPARTITION_STAGE,pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallPartitionFilter(),0.5);
  progressIndicator.AddFilter(ebiVesselTargetPipeline::WALLTHICKNESS_STAGE,pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetWallThicknessFilter(),0.2);
  progressIndicator.AddFilter(ebiVesselTargetPipeline::PERIVASCULARREGION_STAGE,pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPerivascularRegionFilter(),10);
  progressIndicator.AddFilter(ebiVesselTargetPipeline::PERIVASCULARREGIONPARTITION_STAGE,pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPerivascularRegionPartitionFilter(),0.5);
  progressIndicator.AddFilter(ebiVesselTargetPipeline::COMPOSITION_STAGE,pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetCompositionFilter(),10);
  progressIndicator.AddFilter(ebiVesselTargetPipeline::CAPTHICKNESS_STAGE,pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetCapThicknessFilter(),0.2);
  progressIndicator.AddFilter(ebiVesselTargetPipeline::READINGS_STAGE,pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetReadingsFilter(),1);
  progressIndicator.AddFilter(ebiVesselTargetPipeline::LESIONLUMENPARTITION_STAGE,pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionLumenPartitionFilter(),0.5);
  progressIndicator.AddFilter(ebiVesselTargetPipeline::LESIONLUMENANDWALLPARTITION_STAGE,pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionLumenAndWallPartitionFilter(),0.5);
  progressIndicator.AddFilter(ebiVesselTargetPipeline::LESIONPERIVASCULARREGIONPARTITION_STAGE,pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionPerivascularRegionPartitionFilter(),0.5);
  progressIndicator.AddFilter(ebiVesselTargetPipeline::LESIONREADINGS_STAGE,pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionReadingsFilter(),1);
  this->repaint(); // ensure progress is shown
  qApp->processEvents();

  // verify the starting point and take it through to completion
  if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenSegmentationPreConditions()) {
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation() == NULL) {
      viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Delineating Overall Luminal Surfaces").toStdString());
      viewers->SyncViewersToScene();
      viewers->Render();
      try {
        pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenSegmentation();
      } catch (std::exception &e) {
              eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
        QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
        return false; 
      }
      logUpdateWithStageSettings("targetDefine::ComputeLumenSegmentation", def, def->getID());
      def->unsavedChanges = true;
      def->lumenSegIDdefine = viewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation(), 0, "lumenSegmentation"); 
      clearTargetBeyondCurrentStage(def);
    }
    this->repaint(); // ensure progress is shown
    qApp->processEvents();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PathPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath() == NULL) {
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Computing Detailed Cross-section Positions").toStdString());
        viewers->SyncViewersToScene();
        viewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputePath();
        } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
          return false;
        }
        logUpdateWithStageSettings("targetDefine::ComputeVesselTargetPath", def, def->getID());
        def->unsavedChanges = true;
        if (def->initializerIDdefine) {
          viewers->RemoveVesselTarget(def->initializerIDdefine); // if have a path, don't want to display the initializer too
          def->initializerIDdefine = null_ebID;
        }
        def->pathIDdefine = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath(), "path"); 
        clearTargetBeyondCurrentStage(def);
      }
    }
    this->repaint(); // ensure progress is shown
    qApp->processEvents();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenPartitionPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenPartition() == NULL) {
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Partitioning Lumen Into Vessels").toStdString());
        viewers->SyncViewersToScene();
        viewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenPartition();
              } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
          return false; 
        }
        logUpdateWithStageSettings("targetDefine::ComputeLumenPartition", def, def->getID());
        def->unsavedChanges = true;
        viewers->RemoveSegmentation4(def->lumenSegIDdefine); // if have partitions, don't want to display the segmentation too
        def->lumenSegIDdefine = null_ebID;
        def->lumenPartIDsDefine.clear();
        int j = 0;
        for (auto partition : *pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenPartition()) {
          def->lumenPartIDsDefine.insert(viewers->AddSegmentation4(partition.GetPointer(), 0, "lumenPartition"+std::to_string(j++)));
        }
        clearTargetBeyondCurrentStage(def);
      }
    }
    this->repaint(); // ensure progress is shown
    qApp->processEvents();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->RegistrationPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetResampledRegisteredImages() == NULL) {
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Registering Images").toStdString());
        viewers->SyncViewersToScene();
        viewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeRegistration();
              } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
          return false; 
        }
        logUpdateWithStageSettings("targetDefine::ComputeRegistration", def, def->getID());
        def->unsavedChanges = true;
        clearTargetBeyondCurrentStage(def);
      }
    }
    this->repaint(); // ensure progress is shown
    qApp->processEvents();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenAndWallSegmentationPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallSegmentation() == NULL) {
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Delineating Outer Wall Surface").toStdString());
        viewers->SyncViewersToScene();
        viewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenAndWallSegmentation();
              } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
          return false; 
        }
        logUpdateWithStageSettings("targetDefine::ComputeLumenAndWallSegmentation", def, def->getID());
        def->unsavedChanges = true;
        def->wallSegIDdefine = viewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallSegmentation(), 0, "wallSegmentation");
              viewers->GetScene()->GetSegmentation4(def->wallSegIDdefine)->GetInteriorProperty()->SetOpacity(0);
        clearTargetBeyondCurrentStage(def);
      }
    }
    this->repaint(); // ensure progress is shown
    qApp->processEvents();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenAndWallPartitionPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallPartition() == NULL) {
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Partitioning Wall Into Vessels").toStdString());
        viewers->SyncViewersToScene();
        viewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenAndWallPartition();
              } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
          return false; 
        }
        logUpdateWithStageSettings("targetDefine::ComputeLumenAndWallPartition", def, def->getID());
        def->unsavedChanges = true;
        viewers->RemoveSegmentation4(def->wallSegIDdefine); // if have partitions, don't want to display the segmentation too
        def->wallSegIDdefine = null_ebID;
        def->wallPartIDsDefine.clear();
        int j = 0;
        for (auto partition : *pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallPartition()) {
          ebID partID = viewers->AddSegmentation4(partition.GetPointer(), 0, "wallPartition"+std::to_string(j++));
          def->wallPartIDsDefine.insert(partID);
          viewers->GetScene()->GetSegmentation4(partID)->GetInteriorProperty()->SetOpacity(0);
        }
        clearTargetBeyondCurrentStage(def);
      }
    }
    this->repaint(); // ensure progress is shown
    qApp->processEvents();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->WallThicknessPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetWallThickness() == NULL) {
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Computing Wall Thickness Distribution").toStdString());
        viewers->SyncViewersToScene();
        viewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeWallThickness();
              } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
          return false; 
        }
        logUpdateWithStageSettings("targetDefine::ComputeWallThickness", def, def->getID());
        def->unsavedChanges = true;
        clearTargetBeyondCurrentStage(def);
      }
    }
    this->repaint(); // ensure progress is shown
    qApp->processEvents();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PerivascularRegionPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPerivascularRegion() == NULL) {
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Establishing Perivascular Region").toStdString());
        viewers->SyncViewersToScene();
        viewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputePerivascularRegion();
              } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
          return false; 
        }
        logUpdateWithStageSettings("targetDefine::ComputePerivascularRegion", def, def->getID());
        def->unsavedChanges = true;
        clearTargetBeyondCurrentStage(def);
      }
    }
    this->repaint(); // ensure progress is shown
    qApp->processEvents();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PerivascularRegionPartitionPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPerivascularRegionPartition() == NULL) {
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Partitioning Perivascular Region Into Vessels").toStdString());
        viewers->SyncViewersToScene();
        viewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputePerivascularRegionPartition();
              } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
          return false; 
        }
        logUpdateWithStageSettings("targetDefine::ComputePerivascularRegionPartition", def, def->getID());
        def->unsavedChanges = true;
        clearTargetBeyondCurrentStage(def);
      }
    }
    this->repaint(); // ensure progress is shown
    qApp->processEvents();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CompositionPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetComposition() == NULL) {
        QMessageBox msgBox(this);
        msgBox.setText(tr("The current target definitions now enable completion of the calculations."));
        msgBox.setInformativeText(tr("Would you like to compute locally, or on the CAPgraph server? Yes means compute locally, No means you prefer to compute on the server."));
        msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
        if (msgBox.exec() == QMessageBox::Yes) { 
          viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Determining Composition of Wall and Plaque Tissues").toStdString());
          viewers->SyncViewersToScene();
          viewers->Render();
          try {
          // NO LONGER NEED THIS BUT THE PATTERN MIGHT BE USEFUL FOR OTHER PURPOSES LATERif (owner->clinicalJurisdiction != "") // any of the clinical editiuons
          //   pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetCompositionAnalytes(true,true,false,true,false);
          // else
            pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetCompositionAnalytes(true,true,true,true,true);
            pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetCompositionFilter()->UpdateCalFactorBykVp(currentBackingSeries->kvp.toDouble());
            pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeComposition();
          } catch (std::exception &e) {
            eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
            QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
            return false; 
          }
          logUpdateWithStageSettings("targetDefine::ComputeComposition", def, def->getID());
          def->unsavedChanges = true;

          //FROM A.3 and later releases, it is no longer needed to display the composition control dialog for the following two reasons.
          //1) MultiKVP calibration (scheduled for A.3 release) will lead to improved composition analysis and hence may not be needed to provide range hint for parameters.
          //2) If still necessary, user can always set the parameter range from the Processing Parameter Dialog. 
          //def->parameters->presentCompositionControl();
          clearTargetBeyondCurrentStage(def);
        } else {
          // in this case we need to update the data to the server, so that computations are enabled
          if (!updateServer()) {
            message->showMessage(tr("Error: Cannot write work item folder."));
            return false;
          }
    
          // stop the transition, user will do the computation on the server
          viewers->GetViewer(ebvIdVolume)->SetTemporaryText("");
          viewers->SyncViewersToScene();
          viewers->Render();
          progressIndicator.setMaximum(100); // any value will take it down
          return false;
        }
      }
    }
    this->repaint(); // ensure progress is shown
    qApp->processEvents();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CapThicknessPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetCapThickness() == NULL) {
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Computing Cap Thickness Distribution").toStdString());
        viewers->SyncViewersToScene();
        viewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeCapThickness();
              } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
          return false; 
        }
        logUpdateWithStageSettings("targetDefine::ComputeCapThickness", def, def->getID());
        def->unsavedChanges = true;
        clearTargetBeyondCurrentStage(def);
      }
    }
    this->repaint(); // ensure progress is shown
    qApp->processEvents();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ReadingsPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetReadings() == NULL) {
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Performing Quantitative Calculations").toStdString());
        viewers->SyncViewersToScene();
        viewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeReadings();
              } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
          return false; 
        }
        logUpdateWithStageSettings("targetDefine::ComputeVesselTargetReadings", def, def->getID());
        def->unsavedChanges = true;
        def->keyImages.clear(); // anything that caused readings to be re-computed invalidates any saved key images
        clearTargetBeyondCurrentStage(def);
        owner->getPatientAnalyze(sessionItemIndex)->newTargetReadingsFlag = true; // this will cause report content to be anticipated
      }
      viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Target Calculations Completed").toStdString());
      viewers->SyncViewersToScene();
      viewers->Render();
    }
    this->repaint(); // ensure progress is shown
    qApp->processEvents();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionLumenPartitionPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionLumenPartition() == NULL) {
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Partitioning Lumen Into Lesions").toStdString());
        viewers->SyncViewersToScene();
        viewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionLumenPartition();
              } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
          return false; 
        }
        logUpdateWithStageSettings("targetDefine::ComputeLesionLumenPartition", def, def->getID());
        def->unsavedChanges = true;
        clearTargetBeyondCurrentStage(def);
      }
    }
    this->repaint(); // ensure progress is shown
    qApp->processEvents();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionLumenAndWallPartitionPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionLumenAndWallPartition() == NULL) {
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Partitioning Wall Into Lesions").toStdString());
        viewers->SyncViewersToScene();
        viewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionLumenAndWallPartition();
              } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
          return false; 
        }
        logUpdateWithStageSettings("targetDefine::ComputeLesionLumenAndWallPartition", def, def->getID());
        def->unsavedChanges = true;
        clearTargetBeyondCurrentStage(def);
      }
    }
    this->repaint(); // ensure progress is shown
    qApp->processEvents();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionPerivascularRegionPartitionPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionPerivascularRegionPartition() == NULL) {
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Partitioning PerivascularRegion Into Lesions").toStdString());
        viewers->SyncViewersToScene();
        viewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionPerivascularRegionPartition();
              } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
          return false; 
        }
        logUpdateWithStageSettings("targetDefine::ComputeLesionPerivascularRegionPartition", def, def->getID());
        def->unsavedChanges = true;
        clearTargetBeyondCurrentStage(def);
      }
    }
    this->repaint(); // ensure progress is shown
    qApp->processEvents();
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionReadingsPreConditions()) {
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionReadings() == NULL) {
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Performing Quantitative Calculations for Lesions").toStdString());
        viewers->SyncViewersToScene();
        viewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionReadings();
              } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
          return false; 
        }
        logUpdateWithStageSettings("targetDefine::ComputeLesionReadings", def, def->getID());
        def->unsavedChanges = true;
        clearTargetBeyondCurrentStage(def);
        owner->getPatientAnalyze(sessionItemIndex)->newLesionReadingsFlag = true; // this will cause report content to be anticipated
      }
      viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Lesion Calculations Completed").toStdString());
      viewers->SyncViewersToScene();
      viewers->Render();
      //progressIndicator.setMaximum(100); // any value will take it down
      return true;
    }
    /*
    else {
      qWarning() << "compute remaining stages was not able to make it all the way through to the lesion readings.";
      viewers->GetViewer(ebvIdVolume)->SetTemporaryText("");
      viewers->SyncViewersToScene();
      viewers->Render();
      //progressIndicator.setMaximum(100); // any value will take it down
      return false;
      }*/
    return true;
  }
  else {
    qWarning() << "compute remaining stages was invoked on a target that is not yet viable, skipping.";
    viewers->GetViewer(ebvIdVolume)->SetTemporaryText("");
    viewers->SyncViewersToScene();
    viewers->Render();
    //progressIndicator.setMaximum(100); // any value will take it down
    return false;
  }
}

void targetDefine::clearTargetBeyondCurrentStage(targetDef *def)
{
  ebLog eblog(Q_FUNC_INFO); eblog << def->getID().toStdString() << std::endl;
  int currentStage;
  if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization() == NULL)
    currentStage = ebiVesselTargetPipeline::INITIALIZATION_STAGE;
  else if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation() == NULL)
    currentStage = ebiVesselTargetPipeline::LUMENSEGMENTATION_STAGE;
  else if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath() == NULL)
    currentStage = ebiVesselTargetPipeline::PATH_STAGE;
  else if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenPartition() == NULL)
    currentStage = ebiVesselTargetPipeline::LUMENPARTITION_STAGE;
  else if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetResampledRegisteredImages() == NULL)
    currentStage = ebiVesselTargetPipeline::IMAGEREGISTRATION_STAGE;
  else if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallSegmentation() == NULL)
    currentStage = ebiVesselTargetPipeline::LUMENANDWALLSEGMENTATION_STAGE;
  else if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenAndWallPartition() == NULL)
    currentStage = ebiVesselTargetPipeline::LUMENANDWALLPARTITION_STAGE;
  else if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetWallThickness() == NULL)
    currentStage = ebiVesselTargetPipeline::WALLTHICKNESS_STAGE;
  else if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPerivascularRegion() == NULL)
    currentStage = ebiVesselTargetPipeline::PERIVASCULARREGION_STAGE;
  else if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPerivascularRegionPartition() == NULL)
    currentStage = ebiVesselTargetPipeline::PERIVASCULARREGIONPARTITION_STAGE;
  else if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetComposition() == NULL)
    currentStage = ebiVesselTargetPipeline::COMPOSITION_STAGE;
  else if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetCapThickness() == NULL)
    currentStage = ebiVesselTargetPipeline::CAPTHICKNESS_STAGE;
  else if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetReadings() == NULL)
    currentStage = ebiVesselTargetPipeline::READINGS_STAGE;
  else if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionLumenPartition() == NULL)
    currentStage = ebiVesselTargetPipeline::LESIONLUMENPARTITION_STAGE;
  else if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionLumenAndWallPartition() == NULL)
    currentStage = ebiVesselTargetPipeline::LESIONLUMENANDWALLPARTITION_STAGE;
  else if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionPerivascularRegionPartition() == NULL)
    currentStage = ebiVesselTargetPipeline::LESIONPERIVASCULARREGIONPARTITION_STAGE;
  else if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLesionReadings() == NULL)
    currentStage = ebiVesselTargetPipeline::LESIONREADINGS_STAGE;
  else {
    // nothing needs to be cleared
    return;
  }

  switch (currentStage) { // we use a switch statement because we want the fall-through behaviour
    case ebiVesselTargetPipeline::INITIALIZATION_STAGE:
      if (def->initializerIDdefine != null_ebID) {
        viewers->RemoveVesselTarget(def->initializerIDdefine); 
        def->initializerIDdefine = null_ebID;
        def->unsavedChanges = true;
      }
      def->setTargetInitializerFileName("");
    
    case ebiVesselTargetPipeline::LUMENSEGMENTATION_STAGE:
      if (def->lumenSegIDdefine != null_ebID) {
        viewers->RemoveSegmentation4(def->lumenSegIDdefine); 
        def->lumenSegIDdefine = null_ebID;
        def->unsavedChanges = true;
      }
      def->lumenSegIDdefine = null_ebID;
      for (int j=0; j < def->regions.size(); j++) {
        region *reg = &((def->regions)[j]);
        if (reg->regName.contains("lumenSegmentation")) {
          def->regions.removeAt(j);
          break;
        }
      }

    case ebiVesselTargetPipeline::PATH_STAGE:
      if (def->pathIDdefine != null_ebID) {
        viewers->RemoveVesselTarget(def->pathIDdefine); 
        def->pathIDdefine = null_ebID;
        // if there is an initializer, need to restore it
        if ((currentStage > 0) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization() != NULL)) {
          def->initializerIDdefine = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization(), "initializer"); 
        }
        def->unsavedChanges = true;
      }
      def->setTargetPathFileName("");
    
    case ebiVesselTargetPipeline::LUMENPARTITION_STAGE:
      if (!def->lumenPartIDsDefine.empty()) {
        for (auto partitionID : def->lumenPartIDsDefine) {
          viewers->RemoveSegmentation4(partitionID);
        }
        def->unsavedChanges = true;
      }
      def->lumenPartIDsDefine.clear();
      for (int j=0; j < def->regions.size(); j++) {
        region *reg = &((def->regions)[j]);
        if (reg->regName.contains("lumenPartition")) {
          def->regions.removeAt(j);
          break;
        }
      }
    
    case ebiVesselTargetPipeline::IMAGEREGISTRATION_STAGE:
      def->setTargetRegistrationTransforms("");
    
    case ebiVesselTargetPipeline::LUMENANDWALLSEGMENTATION_STAGE:
      if (def->wallSegIDdefine != null_ebID) {
        viewers->RemoveSegmentation4(def->wallSegIDdefine); 
        def->unsavedChanges = true;
      }
      def->wallSegIDdefine = null_ebID;
      for (int j=0; j < def->regions.size(); j++) {
        region *reg = &((def->regions)[j]);
        if (reg->regName.contains("wallSegmentation")) {
          def->regions.removeAt(j);
          break;
        }
      }
    
    case ebiVesselTargetPipeline::LUMENANDWALLPARTITION_STAGE:
      if (!def->wallPartIDsDefine.empty()) {
        for (auto partitionID : def->wallPartIDsDefine) {
          viewers->RemoveSegmentation4(partitionID);
        }
        def->unsavedChanges = true;
      }
      def->wallPartIDsDefine.clear();
      for (int j=0; j < def->regions.size(); j++) {
        region *reg = &((def->regions)[j]);
        if (reg->regName.contains("wallPartition")) {
          def->regions.removeAt(j);
          break;
        }
      }
      setToggleToolStates();

    case ebiVesselTargetPipeline::WALLTHICKNESS_STAGE:
      if (def->wallThickIDdefine != null_ebID) {
        viewers->RemoveImage4(def->wallThickIDdefine); 
        def->unsavedChanges = true;
      }
      def->wallThickIDdefine = null_ebID;
      for (int j=0; j < def->valueMaps.size(); j++) {
        valueMap *map = &((def->valueMaps)[j]);
        if (map->valueName.contains("wallThickness")) {
          def->valueMaps.removeAt(j);
          break;
        }
      }

    case ebiVesselTargetPipeline::PERIVASCULARREGION_STAGE:
      if (def->periRegIDdefine != null_ebID) {
        viewers->RemoveSegmentation4(def->periRegIDdefine); 
        def->unsavedChanges = true;
      }
      def->periRegIDdefine = null_ebID;
      for (int j=0; j < def->regions.size(); j++) {
        region *reg = &((def->regions)[j]);
        if (reg->regName.contains("perivascularRegion")) {
          def->regions.removeAt(j);
          break;
        }
      }
    
    case ebiVesselTargetPipeline::PERIVASCULARREGIONPARTITION_STAGE:
      if (!def->periRegPartIDsDefine.empty()) {
        for (auto partitionID : def->periRegPartIDsDefine) {
          viewers->RemoveSegmentation4(partitionID);
        }
        def->unsavedChanges = true;
      }
      def->periRegPartIDsDefine.clear();
      for (int j=0; j < def->regions.size(); j++) {
        region *reg = &((def->regions)[j]);
        if (reg->regName.contains("periRegPartition")) {
          def->regions.removeAt(j);
          break;
        }
      }

    case ebiVesselTargetPipeline::COMPOSITION_STAGE:
      for (int j=0; j < def->probabilityMaps.size(); j++) {
        probabilityMap *map = &((def->probabilityMaps)[j]);
        if (map->probabilityName.contains("composition")) {
          def->probabilityMaps.removeAt(j);
          break;
        }
      }

    case ebiVesselTargetPipeline::CAPTHICKNESS_STAGE:
      if (def->capThickIDdefine != null_ebID) {
        viewers->RemoveImage4(def->capThickIDdefine); 
        def->unsavedChanges = true;
      }
      def->capThickIDdefine = null_ebID;
      for (int j=0; j < def->valueMaps.size(); j++) {
        valueMap *map = &((def->valueMaps)[j]);
        if (map->valueName.contains("capThickness")) {
          def->valueMaps.removeAt(j);
          break;
        }
      }

    case ebiVesselTargetPipeline::READINGS_STAGE:
      if (def->readingsIDdefine != null_ebID) {
        viewers->RemoveVesselTarget(def->readingsIDdefine); 
        def->readingsIDdefine = null_ebID;
        // if there is either a path, or even an initializer, need to restore accordingly
        if ((currentStage > ebiVesselTargetPipeline::PATH_STAGE) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath() != NULL)) {
          def->pathIDdefine = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath(), "path"); 
        }
        else if ((currentStage > ebiVesselTargetPipeline::INITIALIZATION_STAGE) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization() != NULL)) {
          def->initializerIDdefine = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization(), "initializer"); 
        }
      }
      def->setTargetReadingsFileName("");

    case ebiVesselTargetPipeline::LESIONLUMENPARTITION_STAGE:
      if (!def->lesionLumenPartIDsDefine.empty()) {
        for (auto partitionID : def->lesionLumenPartIDsDefine) {
          viewers->RemoveSegmentation4(partitionID);
        }
        def->unsavedChanges = true;
      }
      def->lesionLumenPartIDsDefine.clear();
      for (int j=0; j < def->regions.size(); j++) {
        region *reg = &((def->regions)[j]);
        if (reg->regName.contains("lesionLumenPartition")) {
          def->regions.removeAt(j);
          break;
        }
      }
    
    case ebiVesselTargetPipeline::LESIONLUMENANDWALLPARTITION_STAGE:
      if (!def->lesionLumenAndWallPartIDsDefine.empty()) {
        for (auto partitionID : def->lesionLumenAndWallPartIDsDefine) {
          viewers->RemoveSegmentation4(partitionID);
        }
        def->unsavedChanges = true;
      }
      def->lesionLumenAndWallPartIDsDefine.clear();
      for (int j=0; j < def->regions.size(); j++) {
        region *reg = &((def->regions)[j]);
        if (reg->regName.contains("lesionLumenAndWallPartition")) {
          def->regions.removeAt(j);
          break;
        }
      }
    
    case ebiVesselTargetPipeline::LESIONPERIVASCULARREGIONPARTITION_STAGE:
      if (!def->lesionPeriRegPartIDsDefine.empty()) {
        for (auto partitionID : def->lesionPeriRegPartIDsDefine) {
          std::cerr << "line 1214"<< std::endl;
          viewers->RemoveSegmentation4(partitionID);
        }
        def->unsavedChanges = true;
      }
      def->lesionPeriRegPartIDsDefine.clear();
      for (int j=0; j < def->regions.size(); j++) {
        region *reg = &((def->regions)[j]);
        if (reg->regName.contains("lesionPeriRegPartition")) {
          def->regions.removeAt(j);
          break;
        }
      }
    
    case ebiVesselTargetPipeline::LESIONREADINGS_STAGE:
      if (def->lesionReadingsIDdefine != null_ebID) {
        viewers->RemoveVesselTarget(def->lesionReadingsIDdefine); 
        def->lesionReadingsIDdefine = null_ebID;
        // if there is either readings, a path, or even an initializer, need to restore accordingly
        if ((currentStage > ebiVesselTargetPipeline::READINGS_STAGE) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetReadings() != NULL)) {
          def->readingsIDdefine = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetReadings(), "readings"); 
        }
        else if ((currentStage > ebiVesselTargetPipeline::PATH_STAGE) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath() != NULL)) {
          def->pathIDdefine = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetPath(), "path"); 
        }
        else if ((currentStage > ebiVesselTargetPipeline::INITIALIZATION_STAGE) && (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization() != NULL)) {
          def->initializerIDdefine = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization(), "initializer"); 
        }
      }
      def->setLesionReadingsFileName("");
  }
}

void targetDefine::setToggleToolStates()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // prior to looping over targets, assume there are no lumen or wall regions
  DISABLECONTROL(tools->ToggleLumenTool, tools->ToggleLumenAction, tr("ToggleLumen is disabled"));  
  DISABLECONTROL(tools->ToggleWallTool, tools->ToggleWallAction, tr("ToggleWall is disabled"));
  tools->editSegmentationEnableState = false;

  for (int i=0; i < targets->size(); i++) {
    targetDef *def = &((*targets)[i]);

    if (!def->lumenPartIDsDefine.empty())
      for (auto partitionID : def->lumenPartIDsDefine)
        if (partitionID != null_ebID) {
          ENABLECONTROL(tools->ToggleLumenTool, tools->ToggleLumenAction, tr("Press to toggle lumen display"));
          tools->editSegmentationEnableState = true;
          tools->editSegmentationActionLabel = tr("Edit Lumen");
        }
    if (def->lumenSegIDdefine != null_ebID) {
      ENABLECONTROL(tools->ToggleLumenTool, tools->ToggleLumenAction, tr("Press to toggle lumen display"));
      tools->editSegmentationEnableState = true;
      tools->editSegmentationActionLabel = tr("Edit Lumen");
    }

    if (!def->wallPartIDsDefine.empty())
      for (auto partitionID : def->wallPartIDsDefine)
        if (partitionID != null_ebID) {
          ENABLECONTROL(tools->ToggleWallTool, tools->ToggleWallAction, tr("Press to toggle outer wall display"));
          tools->editSegmentationEnableState = true;
          tools->editSegmentationActionLabel = tr("Edit Wall");
        }
    if (def->wallSegIDdefine != null_ebID) {
      ENABLECONTROL(tools->ToggleWallTool, tools->ToggleWallAction, tr("Press to toggle outer wall display"));
      tools->editSegmentationEnableState = true;
      tools->editSegmentationActionLabel = tr("Edit Wall");
    }
  }
}

void targetDefine::establishTools(QString product, int index, bool masterWithRespectToMenu)
{
  // set up the tools, initally all disabled
  menuActionEnabledMap = new std::map<QAction *, bool>;
  tools = new capTools(owner, product, index, viewers, ebvIdVolume, ebvViewer::THREED, ui->targetDefineControlsBox, targetDefineMenu, masterWithRespectToMenu, "__PRIMARY__", menuActionEnabledMap);
  tools->enableToolButtons(); // start the buttons enabled
  tools->markLesionEnableState = false; // except mark lesion is always disabled because targetDefine lacks the needed context
  tools->markLesionActionLabel = tr("(Must Proceed to Analyze to Mark Lesion)");
  tools->clearLesionsEnableState = false; // except clear lesions is always disabled because targetDefine lacks the needed context
  tools->clearLesionsActionLabel = tr("(Must Proceed to Analyze to Clear Lesions)");
  HIDECONTROL(tools->ShowAsMovieTool, tools->ShowAsMovieAction);//PUT BACK IN WHEN IMPLEMENTED: if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->ShowAsMovieTool);
  HIDECONTROL(tools->AnnotateTool, tools->AnnotateAction);//PUT BACK IN WHEN IMPLEMENTED: if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->AnnotateTool);
  HIDECONTROL(tools->MeasureTool, tools->MeasureAction);
  if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->TargetCreateTool);
  if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->TargetModifyTool);
  if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->TargetDeleteTool);
  if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->TargetPathSwitchTool);
  if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->MoveProximalTool);
  if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->MoveDistalTool);
  if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->ToggleLumenTool);
  if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->ToggleWallTool);
  HIDECONTROL(tools->MaxStenosisTool, tools->MaxStenosisAction);
  HIDECONTROL(tools->MaxDilationTool, tools->MaxDilationAction);
  HIDECONTROL(tools->MaxRRTool, tools->MaxRRAction);
  HIDECONTROL(tools->MaxWTTool, tools->MaxWTAction);
  HIDECONTROL(tools->ToggleCornerTool, tools->ToggleCornerAction);
  HIDECONTROL(tools->MaxCalcTool, tools->MaxCalcAction);
  HIDECONTROL(tools->MaxLRNCTool, tools->MaxLRNCAction);
  HIDECONTROL(tools->MaxMATXTool, tools->MaxMATXAction);
  HIDECONTROL(tools->MaxIPHTool, tools->MaxIPHAction);
  HIDECONTROL(tools->MaxUlcTool, tools->MaxUlcAction);
  HIDECONTROL(tools->MaxThrTool, tools->MaxThrAction); 
  if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->ToggleObliqueTool);
  if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->ToggleCoronalTool);
  if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->ToggleSagittalTool);
  if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->ToggleAxialTool);
  HIDECONTROL(tools->ToggleColorBarTool, tools->ToggleColorBarAction);
  if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->OptimizeSubvolumeTool);
  if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->VolumeRenderTool);
  HIDECONTROL(tools->ShapeIntensityTool, tools->ShapeIntensityAction); //if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->ShapeIntensityTool);
  HIDECONTROL(tools->RestoreViewSettingsTool, tools->RestoreViewSettingsAction); //if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->RestoreViewSettingsTool);
  HIDECONTROL(tools->SaveViewSettingsTool, tools->SaveViewSettingsAction); //if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->SaveViewSettingsTool);
  if (owner != 0) ui->targetDefineControls->insertWidget(0, tools->CenterAtCursorTool);
  HIDECONTROL(tools->SaveToReportTool, tools->SaveToReportAction);  

  if (masterWithRespectToMenu) {
    targetDefineMenu->addSeparator();
    processingParametersAction = new QAction(tr("View and/or modify processing parameters"), this);
    processingParametersAction->setObjectName("processingParameters");
    targetDefineMenu->addAction(processingParametersAction);

    targetDefineMenu->addSeparator();
    gotoTargetDefineAction = new QAction(tr("Go to Target Define"), this);
    gotoTargetDefineAction->setObjectName("gotoTargetDefine");
    targetDefineMenu->addAction(gotoTargetDefineAction);

    gotoTargetDefineAction->setEnabled(false);
    targetDefineMenu->setEnabled(false);
  }   
  else {
    foreach (QAction *a, targetDefineMenu->actions()) {
      if (a->objectName() == "processingParameters") {
        processingParametersAction = a;
      }
      else if (a->objectName() == "gotoTargetDefine") {
        gotoTargetDefineAction = a;
      }
      (*menuActionEnabledMap)[a] = true;
    }
  }
  (*menuActionEnabledMap)[processingParametersAction] = false;
  processingParametersAction->setEnabled(false);

  // whereas other controls start out enabled in targetDefine, not so for ToggleOblique
  tools->ToggleObliqueTool->setChecked(false);
  DISABLECONTROL(tools->ToggleObliqueTool, tools->ToggleObliqueAction, tr("ToggleOblique is disabled (select a target with a path to enable)"));
  DISABLECONTROL(tools->MoveProximalTool, tools->MoveProximalAction, tr("MoveProximal is disabled (select a target with a path to enable)"));
  DISABLECONTROL(tools->MoveDistalTool, tools->MoveDistalAction, tr("MoveDistal is disabled (select a target to enable)"));
}

void targetDefine::disconnectMenuActions()
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (editingSegmentation) {
    segmentationEditor->on_doneButton_clicked();
  }
  disconnect(targetDefineMenu, SIGNAL(triggered(QAction *)), this, SLOT(ensureOnTargetDefinePage()));
  //disconnect(gotoTargetDefineAction, SIGNAL(triggered()), this, SLOT(ensureOnTargetDefinePage()));
  disconnect(processingParametersAction, SIGNAL(triggered()), this, SLOT(on_processingParametersButton_clicked()));
  tools->disconnectMenuActions();
  disableMenuActions();
}

void targetDefine::connectMenuActions()
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  connect(targetDefineMenu, SIGNAL(triggered(QAction *)), this, SLOT(ensureOnTargetDefinePage()));
  //connect(gotoTargetDefineAction, SIGNAL(triggered()), this, SLOT(ensureOnTargetDefinePage()));
  connect(processingParametersAction, SIGNAL(triggered()), this, SLOT(on_processingParametersButton_clicked()));
  tools->connectMenuActions();
  enableMenuActions();
}

void targetDefine::disableMenuActions()
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  for (auto actionEnabled : *menuActionEnabledMap) {
    actionEnabled.first->setEnabled(false);
  }
  targetDefineMenu->setEnabled(false);
  owner->getPatientAnalyze(sessionItemIndex)->disableMenuActions(); // propagate it downstream
}

void targetDefine::enableMenuActions()
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  for (auto actionEnabled : *menuActionEnabledMap)
    actionEnabled.first->setEnabled(actionEnabled.second);
  targetDefineMenu->setEnabled(true);
  if (viableTargetsCount > 0)
    owner->getPatientAnalyze(sessionItemIndex)->enableMenuActions(); // propagate it downstream
  else
    owner->getPatientAnalyze(sessionItemIndex)->disableMenuActions(); // propagate it downstream
}

void targetDefine::ensureOnTargetDefinePage() 
{ 
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  switch (owner->getWorkItemProcess(sessionItemIndex)->currentIndex()) {
    case SERIESSURVEY:
      owner->getSeriesSurvey(sessionItemIndex)->on_proceedToAnalysisButton_clicked();
      return; // don't fall through

    case PATIENTREPORT:
      owner->getPatientReport(sessionItemIndex)->on_backToAnalyzeButton_clicked();
      // fall through

    case PATIENTANALYZE:
      owner->getPatientAnalyze(sessionItemIndex)->on_backToDefineButton_clicked();
  }
}

void targetDefine::preloadDefinePre(QString product, int index, ebiVesselPipeline::Pointer p, QList<imageSeries> *imageSeriesSet, QList<targetDef> *targetDefs)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  thisProduct = product;
  sessionItemIndex = index;
  pipeline = p;
  images = imageSeriesSet; 
  targets = targetDefs; 
  establishTools(thisProduct, sessionItemIndex, false);
  setDefiningRootFlag(false);

  // Now it is important to initialize the various controls effecting planes according to how the viewers were initialized.
  tools->VolumeRenderTool->setChecked(false);
  tools->ToggleObliqueTool->setChecked(false);
  tools->ToggleCoronalTool->setChecked(true);
  tools->ToggleSagittalTool->setChecked(true);
  tools->ToggleAxialTool->setChecked(true);  
}

void targetDefine::resetWI(ebiVesselPipeline::Pointer p, QList<imageSeries> *imageSeriesSet, QList<targetDef> *targetDefs)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  pipeline = p;
  images = imageSeriesSet; 
  targets = targetDefs; 
}

void targetDefine::preloadDefinePost()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  selectCurrentTarget(NULL); // start out with no target selected as current focus

  // loop through initial description to setup predefined targets each according to how well specified they are
  for (int i=0; i < targets->size(); i++) {
    targetDef *def = &((*targets)[i]);
    if ((def->getID() != "") && (def->getBodySite() != "") && (def->getTargetFolder() != "")) {
      qInfo() << "targetDefine::preloadDefinePost: preloading a target of bodySite: " << def->getBodySite();
      def->unsavedChanges = false;
      def->setParentPipeline(pipeline);
      def->targetPipelineID = pipeline->AddVesselTargetPipeline();
      pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetBodySite(def->getBodySite().toStdString());
      pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetTargetFolder(def->getTargetFolder().toStdString());
      def->parameters = new processingParameters(this, owner->systemPreferencesObject, owner->clinicalJurisdiction, pipeline, def->getBodySite(), def->targetPipelineID);
      connect(def->parameters, SIGNAL(processingParametersSettingsChanged()), this, SLOT(processingParametersSettingsChanged()));
      def->pushTargetParametersToPipeline(pipeline->GetVesselTargetPipeline(def->targetPipelineID));
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->InitializationPreConditions()) {
        if (def->getTargetInitializerFileName() != "") {
          QFileInfo checkFile(def->getTargetInitializerFileName());
          if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
            try {
              pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenInitialization(def->getTargetInitializerFileName().toStdString());
            } catch (std::exception &e) {
              eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
              QMessageBox::warning(this, tr("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
              break; 
            }
          }
          else {
            QMessageBox::warning(this, tr("List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
            break; 
          }
        }
        else {
          pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CreateInitialization();
        }
        
        // now we continue as deep as the target definition allows
        // start with lumenSegmentation
        int j;
        region *reg;
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("lumenSegmentation")) {
            break;
          }
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenSegmentationPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLumenSegmentation(reg->regFileName.toStdString());
                  qInfo() << "...OpenLumenSegmentation done";
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  QMessageBox::warning(this, tr("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
                  break; 
                }
              }
              else {
                QMessageBox::warning(this, tr("List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
                break; 
              }
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a lumen segmentation region, but pipeline state doesn't allow it.";
            return;
          }
        }
        // proceed to path
        if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PathPreConditions()) {
          if (def->getTargetPathFileName() != "") {
            QFileInfo checkFile(def->getTargetPathFileName());
            if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
              if (false/* replace with test for whether the object shoudl be recomputed because runnign a later version from that which created the object */) { 
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputePath();
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
                  break; 
                }
                logUpdateWithStageSettings("targetDefine::ComputeVesselTargetPath", def, def->getID());
                def->unsavedChanges = true;
              }
              else {
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenPath(def->getTargetPathFileName().toStdString());
                  qInfo() << "...OpenPath done";
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  QMessageBox::warning(this, tr("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
                  break; 
                }
              }
            else {
              QMessageBox::warning(this, tr("List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
              break; 
            }
          }
        }
        else if (def->getTargetPathFileName() != "") {
          qWarning() << "target in list position" << i << "has a path, but pipeline state doesn't allow it.";
          return;
        }
        // lumenPartition
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("lumenPartition"))
            break;
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenPartitionPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenPartition();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
                    break; 
                  }
                  logUpdateWithStageSettings("targetDefine::ComputeLumenPartition", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLumenPartition(reg->regFileName.toStdString());
                    qInfo() << "...OpenLumenPartition done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
                    break; 
                  }
                }
                // given that we have a partitioned lumen, the target is considered viable.
                def->isViable = true;
                viableTargetsCount++;
              }
              else {
                QMessageBox::warning(this, tr("List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
                break; 
              }              
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a partitioned lumen, but pipeline state doesn't allow it.";
            return;
          }
        }
        // registration
        if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->RegistrationPreConditions()) {
          if (def->getTargetRegistrationTransforms() != "") {
            QFileInfo checkFile(def->getTargetRegistrationTransforms());
            if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
              if (false/* replace with test for whether the object shoudl be recomputed because runnign a later version from that which created the object */) { 
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeRegistration();
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
                  break; 
                }
                logUpdateWithStageSettings("targetDefine::ComputeRegistration", def, def->getID());
                def->unsavedChanges = true;
              }
              else {
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenRegistration(def->getTargetRegistrationTransforms().toStdString());
                  qInfo() << "...OpenRegistration done";
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  QMessageBox::warning(this, tr("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
                  break; 
                }
              }
            else {
              QMessageBox::warning(this, tr("List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
              break; 
            }
          }
        }
        else if (def->getTargetRegistrationTransforms() != "") {
          qWarning() << "target in list position" << i << "has registration transforms, but pipeline state doesn't allow it.";
          return;
        }
        // wallSegmentation
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("wallSegmentation"))
            break;
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenAndWallSegmentationPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLumenAndWallSegmentation(reg->regFileName.toStdString());
                  qInfo() << "...OpenLumenAndWallSegmentation done";
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  QMessageBox::warning(this, tr("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
                  break; 
                }
              }
              else {
                QMessageBox::warning(this, tr("List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
                break; 
              }
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a wall segmentation region, but pipeline state doesn't allow it.";
            return;
          }
        }
        // wallPartition
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("wallPartition"))
            break;
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LumenAndWallPartitionPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLumenAndWallPartition();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
                    break; 
                  }
                  logUpdateWithStageSettings("targetDefine::ComputeLumenAndWallPartition", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLumenAndWallPartition(reg->regFileName.toStdString());
                    qInfo() << "...OpenLumenAndWallPartition done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
                    break; 
                  }
                }
              }
              else {
                QMessageBox::warning(this, tr("List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
                break; 
              }              
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a partitioned wall, but pipeline state doesn't allow it.";
            return;
          }
        }
        // wallThickness
        valueMap *WTmap;
        for (j=0; j < def->valueMaps.size(); j++) {
          WTmap = &((def->valueMaps)[j]);
          if (WTmap->valueName.contains("wallThickness"))
            break;
        }
        if (j < def->valueMaps.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->WallThicknessPreConditions()) {
            if (WTmap->valueMapFileName != "") {
              QFileInfo checkFile(WTmap->valueMapFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeWallThickness();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
                    break; 
                  }
                  logUpdateWithStageSettings("targetDefine::ComputeWallThickness", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenWallThickness(WTmap->valueMapFileName.toStdString());
                    qInfo() << "...OpenWallThickness done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
                    break; 
                  }
                }
              else {
                QMessageBox::warning(this, tr("List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
                break; 
              }              
            }
          }
          else if (WTmap->valueMapFileName != "") {
            qWarning() << "target in list position" << i << "has a wall thickness value map, but pipeline state doesn't allow it.";
            return;
          }
        }
        // perivascularRegion
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("perivascularRegion"))
            break;
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PerivascularRegionPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenPerivascularRegion(reg->regFileName.toStdString());
                  qInfo() << "...OpenPerivascularRegion done";
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  QMessageBox::warning(this, tr("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
                  break; 
                }
              }
              else {
                QMessageBox::warning(this, tr("List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
                break; 
              }
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a perivascular region, but pipeline state doesn't allow it.";
            return;
          }
        }
        // periRegPartition
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("periRegPartition"))
            break;
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->PerivascularRegionPartitionPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputePerivascularRegionPartition();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
                    break; 
                  }
                  logUpdateWithStageSettings("targetDefine::ComputePerivascularRegionPartition", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenPerivascularRegionPartition(reg->regFileName.toStdString());
                    qInfo() << "...OpenPerivascularRegionPartition done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
                    break; 
                  }
                }
              }
              else {
                QMessageBox::warning(this, tr("List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
                break; 
              }              
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a partitioned perivascular region, but pipeline state doesn't allow it.";
            return;
          }
        }
        // composition
        probabilityMap *pmap;
        for (j=0; j < def->probabilityMaps.size(); j++) {
          pmap = &((def->probabilityMaps)[j]);
          if (pmap->probabilityName.contains("composition"))
            break;
        }
        if (j < def->probabilityMaps.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CompositionPreConditions()) {
            if (pmap->probabilityMapFileName != "") {
              QFileInfo checkFile(pmap->probabilityMapFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    /*NOT NEEDED NOW BUT LEFT FOR THE PATTERNif (owner->clinicalJurisdiction != "") // any of the clinical editions
                      pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetCompositionAnalytes(true,true,false,true,false);
                    else*/
                      pipeline->GetVesselTargetPipeline(def->targetPipelineID)->SetCompositionAnalytes(true,true,true,true,true);
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeComposition();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
                    break; 
                  }
                  logUpdateWithStageSettings("targetDefine::ComputeComposition", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenComposition(pmap->probabilityMapFileName.toStdString());
                    qInfo() << "...OpenComposition done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
                    break; 
                  }
                }
              else {
                QMessageBox::warning(this, tr("List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
                break; 
              } 
            }
          }
          else if (pmap->probabilityMapFileName != "") {
            qWarning() << "target in list position" << i << "has a composition probability map, but pipeline state doesn't allow it.";
            //return;
          }
        }
        // capThickness
        valueMap *CTmap;
        for (j=0; j < def->valueMaps.size(); j++) {
          CTmap = &((def->valueMaps)[j]);
          if (CTmap->valueName.contains("capThickness"))
            break;
        }
        if (j < def->valueMaps.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CapThicknessPreConditions()) {
            if (CTmap->valueMapFileName != "") {
              QFileInfo checkFile(CTmap->valueMapFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeCapThickness();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
                    break; 
                  }
                  logUpdateWithStageSettings("targetDefine::ComputeCapThickness", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenCapThickness(CTmap->valueMapFileName.toStdString());
                    qInfo() << "...OpenCapThickness done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
                    break; 
                  }
                }
              else {
                QMessageBox::warning(this, tr("List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
                break; 
              }              
            }
          }
          else if (CTmap->valueMapFileName != "") {
            qWarning() << "target in list position" << i << "has a cap thickness value map, but pipeline state doesn't allow it.";
            //return;
          }
        }
        // readings
        if (def->getTargetReadingsFileName() != "") {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ReadingsPreConditions()) {
            QFileInfo checkFile(def->getTargetReadingsFileName());
            if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
              if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeReadings();
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
                  break; 
                }
                logUpdateWithStageSettings("targetDefine::ComputeVesselTargetReadings", def, def->getID());
                def->unsavedChanges = true;
              }
              else {
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenReadings(def->getTargetReadingsFileName().toStdString());
                  qInfo() << "...OpenReadings done";
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  QMessageBox::warning(this, tr("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
                  break; 
                }
              }
            else {
              QMessageBox::warning(this, tr("List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
              break; 
            }
          }
          else {
            qWarning() << "target in list position" << i << "has readings, but pipeline state doesn't allow it.";
            //return;
          }
        } // end-if there are readings
        // lesionLumenPartition
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("lesionLumenPartition"))
            break;
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionLumenPartitionPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionLumenPartition();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
                    break; 
                  }
                  logUpdateWithStageSettings("targetDefine::ComputeLesionLumenPartition", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLesionLumenPartition(reg->regFileName.toStdString());
                    qInfo() << "...OpenLesionLumenPartition done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
                    break; 
                  }
                }
              }
              else {
                QMessageBox::warning(this, tr("List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
                break; 
              }              
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a partitioned lumen for lesions, but pipeline state doesn't allow it.";
            //return;
          }
        }
        // lesionLumenAndWallPartition
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("lesionLumenAndWallPartition"))
            break;
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionLumenAndWallPartitionPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionLumenAndWallPartition();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
                    break; 
                  }
                  logUpdateWithStageSettings("targetDefine::ComputeLesionLumenAndWallPartition", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLesionLumenAndWallPartition(reg->regFileName.toStdString());
                    qInfo() << "...OpenLesionLumenAndWallPartition done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
                    break; 
                  }
                }
              }
              else {
                QMessageBox::warning(this, tr("List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
                break; 
              }              
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a partitioned lumenAndWall for lesions, but pipeline state doesn't allow it.";
            //return;
          }
        }
        // lesionPeriRegPartition
        for (j=0; j < def->regions.size(); j++) {
          reg = &((def->regions)[j]);
          if (reg->regName.contains("lesionPeriRegPartition"))
            break;
        }
        if (j < def->regions.size()) {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionPerivascularRegionPartitionPreConditions()) {
            if (reg->regFileName != "") {
              QFileInfo checkFile(reg->regFileName);
              if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable()) {
                if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionPerivascularRegionPartition();
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
                    break; 
                  }
                  logUpdateWithStageSettings("targetDefine::ComputeLesionPerivascularRegionPartition", def, def->getID());
                  def->unsavedChanges = true;
                }
                else {
                  try {
                    pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLesionPerivascularRegionPartition(reg->regFileName.toStdString());
                    qInfo() << "...OpenLesionPerivascularRegionPartition done";
                  } catch (std::exception &e) {
                    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                    QMessageBox::warning(this, tr("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
                    break; 
                  }
                }
              }
              else {
                QMessageBox::warning(this, tr("List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
                break; 
              }              
            }
          }
          else if (reg->regFileName != "") {
            qWarning() << "target in list position" << i << "has a partitioned perivascular region for lesions, but pipeline state doesn't allow it.";
            //return;
          }
        }
        // lesion readings
        if (def->getLesionReadingsFileName() != "") {
          if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->LesionReadingsPreConditions()) {
            QFileInfo checkFile(def->getLesionReadingsFileName());
            if (checkFile.exists() && checkFile.isFile() && checkFile.isReadable())
              if (false/* replace with test for whether the object should be recomputed because running a later version from that which created the object */) { 
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->ComputeLesionReadings();
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
                  break; 
                }
                logUpdateWithStageSettings("targetDefine::ComputeLesionReadings", def, def->getID());
                def->unsavedChanges = true;
              }
              else {
                try {
                  pipeline->GetVesselTargetPipeline(def->targetPipelineID)->OpenLesionReadings(def->getLesionReadingsFileName().toStdString());
                  qInfo() << "...OpenLesionReadings done";
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                  QMessageBox::warning(this, tr("Corrupted object"), QString(tr("ending preload early exception while opening object, suggest resetting target")));
                  break; 
                }
              }
            else {
              QMessageBox::warning(this, tr("List error"), QString(tr("ending preload early as file does not exist: %1")).arg(checkFile.filePath()));
              break; 
            }
          }
          else {
            qWarning() << "target in list position" << i << "has lesion readings, but pipeline state doesn't allow it.";
            //return;
          }
        } // end-if there are readings
      } // end-if the target can be initialized
      else
        qWarning() << "target in list position" << i << "cannot be initialized, skipping.";
      presentTarget(def);
    } // end-if the minimum spec is in place (ID, bodySite, and folder all being non-blank) 
    else 
      qWarning() << "target in list position" << i << "does not have the minimum specification, skipping.";
  } // end-for each listed target
  
  // disable target controls if there are no targets
  if (targets->size() < 1) {
    DISABLECONTROL(tools->TargetDeleteTool, tools->TargetDeleteAction, tr("TargetDelete is disabled"));
    DISABLECONTROL(tools->TargetModifyTool, tools->TargetModifyAction, tr("TargetModify is disabled"));
  }
  // disable target controls if there aren't any targets
  if (targets->size() == 0) {
    DISABLECONTROL(tools->TargetPathSwitchTool, tools->TargetPathSwitchAction, tr("TargetPathSwitch is disabled"));
  }
  setToggleToolStates();
  if (viableTargetsCount) {
    ui->continueWithAnalysisButton->setEnabled(true); 
    owner->getPatientAnalyze(sessionItemIndex)->enableMenuActions(); // propagate it downstream
  }
  else {
    ui->continueWithAnalysisButton->setEnabled(false); 
    owner->getPatientAnalyze(sessionItemIndex)->disableMenuActions(); // propagate it downstream
  }
}

void targetDefine::selectCurrentTarget(targetDef *newCurrentDef)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  std::cerr << viewers << std::endl;
  owner->setEnabled(false);

  // first take down the parameter setting dialog if it may be up
  if (currentTarget != NULL)
    if (currentTarget->parameters != NULL)
      currentTarget->parameters->cancelParameterSetting();

  // now proceed
  currentTarget = newCurrentDef;
  QString label;
  if (newCurrentDef != NULL) {
    if ((currentTarget->lesionReadingsIDdefine != null_ebID) || (currentTarget->readingsIDdefine != null_ebID) ||
              (currentTarget->pathIDdefine != null_ebID) || (currentTarget->initializerIDdefine != null_ebID)) {
      ebID id = null_ebID;
      if (currentTarget->lesionReadingsIDdefine != null_ebID)
        id = currentTarget->lesionReadingsIDdefine;
      else if (currentTarget->readingsIDdefine != null_ebID)
        id = currentTarget->readingsIDdefine;
      else if (currentTarget->pathIDdefine != null_ebID)
        id = currentTarget->pathIDdefine;
      else if (currentTarget->initializerIDdefine != null_ebID)
        id = currentTarget->initializerIDdefine;
      viewers->GetScene()->SetSelectedVesselTarget(id);
      ebAssert(viewers && viewers->GetScene() && viewers->GetScene()->GetSelectedVesselTarget());
      viewers->GetScene()->GetSelectedVesselTarget()->GetVesselTarget()->SetName(currentTarget->getID().toStdString());
    }
    label = "Current target being defined: ";
    ENABLECONTROL(ui->processingParametersButton, processingParametersAction, tr("Press to view and/or modify processing paramters to be used on subsequent computations"));
    QString displayName = currentTarget->getID();
    // override the display name if it is a coronary target (because the greater branching complexity makes their screen labelling confusing if we don't do this)
    if (displayName.contains("oronary") && currentTarget->rootVesselName != "") {
      displayName = currentTarget->rootVesselName;
    }
    label.append(displayName);
    bool atLeastOneAlreadyInTheParens = false;
    if (targets->size() > 1) {
      label.append(" (other available targets include: ");
      for (int k=0; k < targets->size(); k++) {
        targetDef *def = &((*targets)[k]);
        if (def->getID() != currentTarget->getID()) {
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
    else {
      label.append(tr(" (no other targets have as yet been defined)"));
    }
  }
  else { // the selected target is NULL, compose label accordingly
    label = tr("No current target focus, press ");
    if (targets->size() > 0) {
      label.append("Switch Target button to select from among ");
      bool atLeastOneAlreadyInTheParens = false;
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
      label.append(tr(" (or press Create Target to add an additional one)"));
      ENABLECONTROL(tools->TargetPathSwitchTool, tools->TargetPathSwitchAction, tr("Press to switch target"));
    }
    else {
      label.append(tr("Create Target button to make one"));
      DISABLECONTROL(tools->TargetPathSwitchTool, tools->TargetPathSwitchAction, tr("TargetPathSwitch is disabled (create a target to enable)"));
    }
    DISABLECONTROL(tools->TargetDeleteTool, tools->TargetDeleteAction, tr("TargetDelete is disabled (select or create a target to enable it)"));
    DISABLECONTROL(tools->TargetModifyTool, tools->TargetModifyAction, tr("TargetModify is disabled (select or create a target to enable it)"));
    DISABLECONTROL(ui->processingParametersButton, processingParametersAction, tr("Processing Parameters is disabled (select or create a target to enable it)"));
    DISABLECONTROL(tools->ToggleObliqueTool, tools->ToggleObliqueAction, tr("ToggleOblique is disabled (select a target with a path to enable)"));
    DISABLECONTROL(tools->MoveProximalTool, tools->MoveProximalAction, tr("MoveProximal is disabled (select a target with a path to enable)"));
    DISABLECONTROL(tools->MoveDistalTool, tools->MoveDistalAction, tr("MoveDistal is disabled (select a target with a path to enable)"));
  }
  ui->volumeLabel->setText(label);
  this->repaint(); // ensure screen is updated
  qApp->processEvents();
  viewers->Render();
  emit setCurrentTarget(currentTarget); // flush information forward
  owner->setEnabled(true);
  this->repaint(); // ensure screen is updated
  qApp->processEvents();
  viewers->Render();
}

void targetDefine::acceptScreenControlFromAnalyze(QStackedWidget *seriesSelectionArea, imageSeries *series, targetDef *def)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  selectCurrentTarget(def);
  acceptScreenControlCommon(seriesSelectionArea, series);
}

void targetDefine::acceptScreenControlFromSurvey(QStackedWidget *seriesSelectionArea, imageSeries *series)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  acceptScreenControlCommon(seriesSelectionArea, series);
}

void targetDefine::acceptScreenControlCommon(QStackedWidget *seriesSelectionArea, imageSeries *series)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  this->setEnabled(false);
  ui->seriesSelectionArea->addWidget(seriesSelectionArea->widget(0));
  ui->seriesSelectionArea->addWidget(seriesSelectionArea->widget(0));
  ui->seriesSelectionArea->setCurrentIndex(0);
  if (series != currentBackingSeries)
    resetBackingSeries(series);
  else { // at least need to do these parts, even if backing hasn't changed
    // base window and level from what was used in slice viewer based values, but scaled by a factor to account for fact that they are MIPs here
    if (viewers && viewers->GetScene() && viewers->GetScene()->GetSelectedImage4() && currentBackingSeries) {
      viewers->GetScene()->GetSelectedImage4()->SetWindowLevel(currentBackingSeries->window, currentBackingSeries->level);
      viewers->SyncViewersToScene();
      viewers->Render();
    }
  }
  owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
  enableMenuActions();
  this->setEnabled(true);
}

void targetDefine::resetBackingSeries(imageSeries *series)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // set the previous image not visible
  if (currentBackingSeries != NULL)
    viewers->GetScene()->GetImage4(backingImageId)->SetVisibility(false);

  // establish the new backing as visible
  currentBackingSeries = series;
  if (currentBackingSeries != NULL) {
    // now make sure the new backing has been added (if it hadn't already been)
    if (added_multireader_images.find(MultiReaderIDPair(pipeline->GetMultiImageReader(), series->imageID)) == added_multireader_images.end()) {
      series->image4IDdefine = viewers->AddImage4(pipeline->GetMultiImageReader(), series->imageID, series->seriesType.toStdString());
      added_multireader_images.insert(MultiReaderIDPair(pipeline->GetMultiImageReader(), series->imageID));
    }
    backingImageId = series->image4IDdefine;
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
    viewers->SetScreenScaleFitFactor2D(.9);
    viewers->OptimizeScreenScale2D();
    // base window and level from what was used in slice viewer based values, but scaled by a factor to account for fact that they are MIPs here
    viewers->GetScene()->GetSelectedImage4()->SetWindowLevel(currentBackingSeries->window, currentBackingSeries->level);
    viewers->GetScene()->SetCursorPositionToSceneCenter();
    viewers->InitializeCameras();
    tools->setBackingImageId(backingImageId);
    viewers->GetViewer3D(ebvIdVolume)->SetPerspectiveDistance(.5*viewers->GetViewer3D(ebvIdVolume)->GetPerspectiveDistance());
    viewers->SyncViewersToScene();
    viewers->UpdateCameras();
    viewers->Render();

    // given that we have a backing series, we can enable the menus
    enableMenuActions();
  }
}

void targetDefine::on_continueWithAnalysisButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  // this is a complicated process, protect against further gui input during it
  this->setEnabled(false);

  if (viableTargetsCount) {
    // loop through initial description to setup predefined targets each according to how well specified they are
    bool atLeastOneViable = false;
    bool atLeastOneNonViable = false;
    targetDef *aViableDef, *aNonViableDef;
    for (int i=0; i < targets->size(); i++) {
      targetDef *def = &((*targets)[i]);
      if ((def->isViable) && computeRemainingStages(def)) {
        atLeastOneViable = true;
        aViableDef = def;
      }
      else {
        atLeastOneNonViable = true;
        aNonViableDef = def;
      }
    }

    if (!atLeastOneViable && atLeastOneNonViable) {
      this->setEnabled(true);
      return;
    }
    if (atLeastOneViable && atLeastOneNonViable) {
      QMessageBox msgBox(owner);
      msgBox.setText(tr("At least one target is not viable (no initializer and lumen computation)."));
      msgBox.setInformativeText(tr("Would you like to initialize it now? (Yes means initialize, No means proceed to analyze the target(s) that is (are) viable.")); 
      msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      msgBox.setDefaultButton(QMessageBox::Yes);
      msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
      int ret = msgBox.exec();
      if (ret == QMessageBox::Yes) {
        if (atLeastOneNonViable) {
          if ((currentTarget == NULL) || (currentTarget->isViable == true)) {
            // need to select a non-viable target, otherwise a viable target might be seclected 
            // causing a potentially confusing screen sequence
            selectCurrentTarget(aNonViableDef);
            // further: if there is no initializer, egin the initialization process
            if ((currentTarget->getTargetInitializerFileName() == "") || (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()->InitializationPointsEmpty())) {
              setDefiningRootFlag(true);
              createTargetPre(currentTarget->getBodySite());
            }
          }
        }
        else {
          message->showMessage(tr("Logic error; non-viable target should be found. (contact Elucid if condition persists)"));
        }
        this->setEnabled(true);
        return;
      }
    }
    if ((currentTarget != NULL) && (currentTarget->parameters != NULL)) {
      currentTarget->parameters->cancelParameterSetting();
    }
    if (atLeastOneViable) {
      if ((currentTarget == NULL) || (currentTarget->isViable != true)) {
        // need to ensure that only a viable target is selected before going to analyze... otherwise a cycle occurs, where even though we are supposed to be on analyze,
        // we can't be, because only viable targest can be selected when we are in analyze. 
        selectCurrentTarget(aViableDef);
      }
    }
    else
      message->showMessage(tr("Logic error; viable target should be found. (contact Elucid if condition persists)"));
    emit giveScreenControlToAnalyze(ui->seriesSelectionArea, currentBackingSeries);
    this->setEnabled(true);
    return;
  } 
  owner->getPatientAnalyze(sessionItemIndex)->disableMenuActions(); // propagate it downstream
  if (currentBackingSeries != NULL) {
    message->showMessage(tr("Warning: Can't proceed without at least one viable target on which computations may be completed to proceed with analysis."));
  }
  this->setEnabled(true);
}

void targetDefine::createTargetPre(QString bodySite)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // compose a menu to get the root vessel name
  QMenu menu(this);
  QList<QString> allowableVesselsGivenBodySite;

  if (bodySite == "Aorta") {
    allowableVesselsGivenBodySite = QList<QString>()
       << tr("Aorta")
       << tr("ThoracicAorta")
       << tr("AbdominalAorta")
       << tr("LeftCommonIliacArtery")
       << tr("RightCommonIliacArtery")
       ;
  } 
  else if (bodySite == "LeftCoronary") {
    allowableVesselsGivenBodySite = QList<QString>()
       << tr("MainStem")
       << tr("LeftAnteriorDescending")
       << tr("Circumflex")
       << tr("LeftCoronaryArtery")
       ;
  } 
  else if (bodySite == "RightCoronary") {
    allowableVesselsGivenBodySite = QList<QString>()
       << tr("RightCoronaryArtery")
       ;
  } 
  else if (bodySite == "LeftCarotid") {
    allowableVesselsGivenBodySite = QList<QString>()
       << tr("CommonCarotidArtery")
       << tr("InternalCarotidArtery")
       << tr("ExternalCarotidArtery")
       << tr("CarotidArtery")
       ;
  } 
  else if (bodySite == "RightCarotid") {
    allowableVesselsGivenBodySite = QList<QString>()
       << tr("CommonCarotidArtery")
       << tr("InternalCarotidArtery")
       << tr("ExternalCarotidArtery")
       << tr("CarotidArtery")
       ;
  } 
  else if ((bodySite == "LeftVertebral") || (bodySite == "RightVertebral")) {
    addVessel(tr("VertebralArtery"));
    return;
  } 
  else if ((bodySite == "LeftFemoral") || (bodySite == "RightFemoral")) {
    addVessel(tr("FemoralArtery"));
    return;
  } 
  else {
    addVessel(tr("NotSpecified"));
    return;
  }

  foreach (QString vessel, allowableVesselsGivenBodySite) {
    QVariant vesselParam;
    QString actionString;
    vesselParam.setValue(vessel);
    actionString = tr("Set root vessel as ");
    actionString.append(vessel);
    QAction *vesselAction = new QAction(actionString, this);
    vesselAction->setData(vesselParam);
    menu.addAction(vesselAction);
    connect(vesselAction, SIGNAL(triggered()), this, SLOT(addRootVesselAction()));
  }

  ui->volume->setEnabled(false);
  while (!owner->quitting) { // this causes input to be mandatory, because not getting the input would leave a partial data structure in an undefined state
    if (menu.exec(QCursor::pos()) != 0) {
      break;
    }
  }
  ui->volume->setEnabled(true);
}

void targetDefine::addRootVesselAction()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QAction *act = qobject_cast<QAction *>(sender());
  if (act != 0) {
    QString rootVesselName = act->data().toString();
    currentTarget->rootVesselName = rootVesselName;
    addVessel(rootVesselName);
  }
  else 
    qWarning() << "targetDefine::addRootVesselAction invoked by act=0";
}

void targetDefine::establishNewTarget(QString bodySite)
{
  ebLog eblog(Q_FUNC_INFO); eblog << bodySite.toStdString() << std::endl;

  if (coronalType == ebvViewer::CPR) {
    // can't define a target when in oblique, need to get back to axial
    toggleOblique(false);
  }

  // allocate the new target def
  targetDef *newTarget = new targetDef();
  newTarget->setBodySite(bodySite); // bodySite stored by createVesselPre to pass to createVesselAction
  newTarget->rootVesselName = ""; // will be set later but need empty string now for display logic

  // we establish the ID of the target to be the body site, or if this same body site has already been seen we also append an index count.  
  QString newID(bodySite);
  int countOfTargetsWithThisBodySite = 1;
  for (int i=0; i < targets->size(); i++) {
    targetDef *def = &((*targets)[i]);
    if (def->getBodySite() == bodySite)
      countOfTargetsWithThisBodySite++;
  }
  if (countOfTargetsWithThisBodySite > 1)
    newID.append(QString::number(countOfTargetsWithThisBodySite)); // this will make the IDs be assigned for example as LeftCarotid, LeftCarotid2, LeftCarotid3, etc., which should work for the typical cases where a given body site isn't usually re-sued (as in LeftCarotid) but also works when it is (e.g., PulmonaryLesion)
  newTarget->setID(newID);

  // now proceed to the rest of the fields
  newTarget->setTargetReadingsFileName(""); 
  newTarget->setLesionReadingsFileName(""); 
  newTarget->setTargetPathFileName("");
  newTarget->setTargetRegistrationTransforms("");
  newTarget->setParentPipeline(pipeline);
  newTarget->targetPipelineID = pipeline->AddVesselTargetPipeline();
  pipeline->GetVesselTargetPipeline(newTarget->targetPipelineID)->SetBodySite(bodySite.toStdString());
  newTarget->parameters = new processingParameters(this, owner->systemPreferencesObject, owner->clinicalJurisdiction, pipeline, newTarget->getBodySite(), newTarget->targetPipelineID);
  connect(newTarget->parameters, SIGNAL(processingParametersSettingsChanged()), this, SLOT(processingParametersSettingsChanged()));
  newTarget->pushTargetParametersToPipeline(pipeline->GetVesselTargetPipeline(newTarget->targetPipelineID));
  if (pipeline->GetVesselTargetPipeline(newTarget->targetPipelineID)->InitializationPreConditions()) {
    newTarget->unsavedChanges = true;

    // setting the targetFolder is more involved due to the need to carefully construct it
    workItemListEntry *wi = owner->getWorkItem(sessionItemIndex)->selectedWorkItemListEntryPtr;
    if (!wi->setWorkItemFolder()) { // this may be the first time an operation that requires the folder has been done
      message->showMessage(tr("Error: Cannot write work item folder."));
      return;
    }
    if (!newTarget->setTargetFolder(wi->getWorkItemFolder())) {
      message->showMessage(tr("Error: Cannot write target folder."));
      return;
    }

    targets->append(*newTarget);

    // here we make sure we have the screen, as the create may have originated from analyze.  That would be simple enough but for a catch: a similar 
    // sequence is used when the request is to modify (rather than create) the target; and in that case, we want analyze to set the current target as 
    // what is to be modified. But this doesn't work for create, since analyze doesn't have the newly created one at this stage of the sequence yet. So
    // need to select the current target as the new one after the transition.
    owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
    selectCurrentTarget(&targets->last()); // the one we just appended
    owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
    qApp->processEvents();

    // now set up to collect points
    pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CreateInitialization();
    currentTarget->initializerIDdefine = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization(), "initializer"); 
  }
  else {
    message->showMessage(tr("Error: New target may not be initialized, skipping."));
  }
  pipeline->GetVesselTargetPipeline(newTarget->targetPipelineID)->SetTargetFolder(currentTarget->getTargetFolder().toStdString());
}

void targetDefine::addVessel(QString startFromName)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // enter gesture mode to collect points for root vessel
  viewers->GetViewer3D(ebvIdVolume)->SetShowVolume(true);
  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, false);
  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, false);
  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, false);
  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, false);
  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, false);
  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP2, false);

  tools->disableToolButtons(); // almost everything needs to be disabled during this operation
  tools->VolumeRenderTool->setChecked(true);

  ENABLECONTROL(tools->OptimizeSubvolumeTool, tools->OptimizeSubvolumeAction, tr("Press to toggle widgets allowing optimization of subvolume"));

  ENABLECONTROL(tools->TargetCreateTool, tools->TargetCreateAction, tr("Press to stop collecting points and process them"));
  tools->TargetCreateTool->setChecked(true);

  DISABLECONTROL(tools->TargetModifyTool, tools->TargetModifyAction, tr("TargetModify is disabled"));

  ENABLECONTROL(tools->ToggleCoronalTool, tools->ToggleCoronalAction, tr("Press to toggle coronal plane"));
  tools->ToggleCoronalTool->setChecked(false);
    
  ENABLECONTROL(tools->ToggleSagittalTool, tools->ToggleSagittalAction, tr("Press to toggle sagittal plane"));
  tools->ToggleSagittalTool->setChecked(false);
    
  ENABLECONTROL(tools->ToggleAxialTool, tools->ToggleAxialAction, tr("Press to toggle axila plane"));
  tools->ToggleAxialTool->setChecked(false);

  ui->continueWithAnalysisButton->setEnabled(false); 
  owner->getPatientAnalyze(sessionItemIndex)->disableMenuActions(); // propagate it downstream

  viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Sub-volume ready").toStdString());
  viewers->SyncViewersToScene();
  viewers->Render();
  // when adding rather than creating, proximal vessel is determined by distance rather than being explicitly given
  viewers->StartVesselTargetInitialization(startFromName.toStdString());
  ui->backToSurveyButton->setEnabled(false); // can't go back when in middle of operation
}

void targetDefine::addDistalVessel()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
  owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
  // clear out any existing computations and displays
  // Keep only the initializer. This is accomplished by closing the lumen (if it exists) and then flushing.
  if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation())
    pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLumenSegmentation(); 
  clearTargetBeyondCurrentStage(currentTarget);
  addVessel("");
}

void targetDefine::completeDistalVessel()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // prompt user for distal vessel name
  // compose a menu to get the root vessel name
  QMenu menu(this);
  QList<QString> allowableDistalVesselsGivenRoot;

  if (currentTarget->rootVesselName == "Aorta") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("LeftCoronaryCusp")
       << tr("LeftCoronaryArtery")
       << tr("MainStem")
       << tr("RightCoronaryCusp")
       << tr("RightCoronaryArtery")
       << tr("NonCoronaryCusp")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "LeftCoronaryArtery") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("LeftAnteriorDescending")
       << tr("Circumflex")
       << tr("Ramus")
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "MainStem") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("LeftAnteriorDescending")
       << tr("Circumflex")
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "LeftAnteriorDescending") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("Diagonal")
       << tr("Diagonal1")
       << tr("Diagonal2")
       << tr("SeptalPerforator")
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "Circumflex") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("LeftMarginal")
       << tr("LeftMarginal1")
       << tr("LeftMarginal2")
       << tr("PosterioLateral")
       << tr("SinoAtrialNode")
       << tr("PosteriorDescending")
       << tr("AtrioventricularNodal")
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "RightCoronaryArtery") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("AtrioventricularNodal")
       << tr("SinoAtrialNode")
       << tr("RightMarginal")
       << tr("PosteriorDescending")
       << tr("PosterioLateral")
       << tr("SeptalPerforator")
       << tr("ConusBranch")
       << tr("Ventricular")
       << tr("AcuteMarginal")
       << tr("RightVentricular")
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "CoronaryArtery") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("CoronaryArtery")
       << tr("NotSpecified")
       ;
  } 
  else if (currentTarget->rootVesselName == "CommonCarotidArtery") {
    allowableDistalVesselsGivenRoot = QList<QString>()
       << tr("InternalCarotidArtery")
       << tr("ExternalCarotidArtery")
       << tr("NotSpecified")
       ;
  }
  else {
    labelDistalVessel(tr("NotSpecified"));
    return;
  }

  foreach (QString vessel, allowableDistalVesselsGivenRoot) {
    QVariant vesselParam;
    QString actionString;
    vesselParam.setValue(vessel);
    actionString = tr("Assign last defined point to ");
    actionString.append(vessel);
    QAction *vesselAction = new QAction(actionString, this);
    vesselAction->setData(vesselParam);
    menu.addAction(vesselAction);
    connect(vesselAction, SIGNAL(triggered()), this, SLOT(labelDistalVesselAction()));
  }

  while (!owner->quitting) { // this causes input to be mandatory, because not getting the input would leave a partial data structure in an undefined state
    if (menu.exec(QCursor::pos()) != 0) {
      break;
    }
  }
}

void targetDefine::labelDistalVesselAction()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QAction *act = qobject_cast<QAction *>(sender());
  if (act != 0) {
    QString distalVesselName = act->data().toString();
    labelDistalVessel(distalVesselName);
  }
  else 
    qWarning() << "targetDefine::labelDistalVesselAction invoked by act=0";
}

void targetDefine::labelDistalVessel(QString distalVesselName)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // turn off the gesture mode (with distal vessel parameter) which causes the target data structures to be established
  viewers->StopVesselTargetInitialization(distalVesselName.toStdString());
  ui->backToSurveyButton->setEnabled(true); // can go back now that operation is done

  //viewers->SetSubVolumeWidgetsEnabled(false);

  // set tool and button settings as needed   
  ENABLECONTROL(tools->ShapeIntensityTool, tools->ShapeIntensityAction, tr("Press to set auto window/level"));
  ENABLECONTROL(tools->SaveViewSettingsTool, tools->SaveViewSettingsAction, tr("Press to save view settings")); 
  ENABLECONTROL(tools->AnnotateTool, tools->AnnotateAction, tr("Press to annotate"));
  ENABLECONTROL(tools->MeasureTool, tools->MeasureAction, tr("Press to make caliper measurements"));
  if (targets->size() >= 1) {
    ENABLECONTROL(tools->TargetPathSwitchTool, tools->TargetPathSwitchAction, tr("Press to switch target path"));
  }
  else {
    DISABLECONTROL(tools->TargetPathSwitchTool, tools->TargetPathSwitchAction, tr("Switch target path is disabled"));
  }
  ENABLECONTROL(tools->TargetCreateTool, tools->TargetCreateAction, tr("Press to create a new target"));
  ENABLECONTROL(tools->TargetDeleteTool, tools->TargetDeleteAction, tr("Press to delete current target"));
  ENABLECONTROL(tools->TargetModifyTool, tools->TargetModifyAction, tr("Press to modify current target"));
  ENABLECONTROL(tools->VolumeRenderTool, tools->VolumeRenderAction, tr("Press to toggle volume rendering"));
  tools->VolumeRenderTool->setChecked(false);
  ENABLECONTROL(tools->ShowAsMovieTool, tools->ShowAsMovieAction, tr("Press to show as movie"));
  ENABLECONTROL(tools->ToggleCoronalTool, tools->ToggleCoronalAction, tr("Press to toggle coronal plane"));
  ENABLECONTROL(tools->ToggleSagittalTool, tools->ToggleSagittalAction, tr("Press to toggle sagittal plane"));
  ENABLECONTROL(tools->ToggleAxialTool, tools->ToggleAxialAction, tr("Press to toggle axila plane"));
  ENABLECONTROL(tools->CenterAtCursorTool, tools->CenterAtCursorAction, tr("Press to center at cursor"));

  // log the update and set the flag
  logUpdateWithStageSettings("targetDefine::labelDistalVessel (initializer)", currentTarget, currentTarget->getID());
  currentTarget->unsavedChanges = true;
}

bool targetDefine::switchTargetPath()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  viewers->GetScene()->IncrCurrentPathSetCursor(1);
  if (viewers->GetScene()->IsVesselTargetSelected()) {
    ENABLECONTROL(tools->ToggleObliqueTool, tools->ToggleObliqueAction, tr("Press to toggle slice viewer alignment"));
    ENABLECONTROL(tools->MoveProximalTool, tools->MoveProximalAction, tr("Press to move proximally"));
    ENABLECONTROL(tools->MoveDistalTool, tools->MoveDistalAction, tr("Press to move distally"));
    ebID id = viewers->GetScene()->GetSelectedVesselTargetID();

    if ((currentTarget == NULL) 
        || ((id != currentTarget->initializerIDdefine) || (id != currentTarget->pathIDdefine) || (id != currentTarget->readingsIDdefine))) {
      // we not only switched paths, but targets as well.  Find the new one and switch to it.
      int j;
      for (j=0; j < targetDefs()->size(); j++) {
        if ((id == targetDefs()->at(j).initializerIDdefine) || (id == targetDefs()->at(j).pathIDdefine) || (id == targetDefs()->at(j).readingsIDdefine))
          break;
      }
      if (j < targetDefs()->size()) {
        selectCurrentTarget(&((*targetDefs())[j]));
              ebAssert(viewers && viewers->GetScene() && viewers->GetScene()->GetSelectedVesselTarget());
        viewers->GetScene()->GetSelectedVesselTarget()->GetVesselTarget()->SetName(currentTarget->getID().toStdString());

        // if the initializer has no points, then get them
        if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()->InitializationPointsEmpty()) {
          owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
          owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
          createTargetPre(currentTarget->getBodySite());
          return false; // signifies that there had to be an initializer created
        }
        else if (!currentTarget->isViable) {
          // similar to case above but without going into the initializer point collecting sequence
          owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
          owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
        }

        // update buttons buttons now that there is a specific target in focus
        ENABLECONTROL(tools->TargetDeleteTool, tools->TargetDeleteAction, tr("Press to delete current target"));
        ENABLECONTROL(tools->TargetModifyTool, tools->TargetModifyAction, tr("Press to modify current target"));
        tools->TargetModifyTool->setChecked(false);
        return true;
      }
      else {
        qWarning() << "can't find target id" << id << ", skipping.";
      }
    } // end-if there had been no prior selected target or if selected target has changed
    else { // there is no change to the target (possibly the path within the target changed, but no actual target switch)
      return true;
    }
  } // end-if there is a selected target
  // get here either if there is no selected target or if can't find the one that supposedly is
  selectCurrentTarget(NULL);
  DISABLECONTROL(tools->ToggleObliqueTool, tools->ToggleObliqueAction, tr("ToggleOblique is disabled (select a target with a path to enable)"));
  DISABLECONTROL(tools->MoveProximalTool, tools->MoveProximalAction, tr("MoveProximal is disabled (select a target with a path to enable)"));
  DISABLECONTROL(tools->MoveDistalTool, tools->MoveDistalAction, tr("MoveDistal is disabled (select a target to enable)"));
  return false;
}

void targetDefine::resetAllTargetsDueToChangesInImages()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  this->setEnabled(false);
  for (int i=0; i < targets->size(); i++) {
    targetDef *def = &((*targets)[i]);
    // Keep only the initializer. This is accomplished by closing the lumen (if it exists) and then flushing.
    if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetLumenSegmentation())
      pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CloseLumenSegmentation(); 
    clearTargetBeyondCurrentStage(def);
    def->unsavedChanges = true;
    logUpdateWithStageSettings("targetDefine::resetAllTargetsDueToChangesInImages", def, def->getID());
  }
  viewers->SyncViewersToScene();
  viewers->Render();
  this->setEnabled(true);
}

void targetDefine::resetAllTargetsDueToDifferentImages()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  this->setEnabled(false);
  if (targets->size() > 0) {
    // start by clearing everything but the initializers
    resetAllTargetsDueToChangesInImages();

    // now prompt to see if want to save the initializers before scrubbing
    QMessageBox msgBox(this);
    msgBox.setText(tr("Resetting targets due to difference in images."));
    msgBox.setInformativeText(tr("Do you want to save the target initializers, so they are available after the reset?"));
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    int ret = msgBox.exec();
    for (int i=0; i < targets->size(); i++) {
      targetDef *def = &((*targets)[i]);
      if (ret == QMessageBox::Yes) {
        owner->save();
      }
      // now close the initializers and delete the pipeline
      if (pipeline->GetVesselTargetPipeline(def->targetPipelineID)->GetInitialization())
        pipeline->GetVesselTargetPipeline(def->targetPipelineID)->CloseInitialization();  
      clearTargetBeyondCurrentStage(def);
      pipeline->RemoveVesselTargetPipeline(def->targetPipelineID);
      def->targetPipelineID = null_ebID;
    }

    // finally, select images again and re-load
    pipeline->SelectImages();
    preloadDefinePost();
  }
  this->setEnabled(true);
}

void targetDefine::deleteTarget()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  this->setEnabled(false);
  if (currentTarget != NULL) {
    // determine whether there are unsaved changes and seek user confirmation if so
    if (currentTarget->unsavedChanges) {
      QMessageBox msgBox(this);
      msgBox.setText(tr("There are unsaved changes which will be lost if the target is deleted."));
      msgBox.setInformativeText(tr("Do you want to save these changes before deleting?"));
      msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      msgBox.setDefaultButton(QMessageBox::No);
      msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
      int ret = msgBox.exec();
      if (ret == QMessageBox::Yes) {
        owner->save();
      }
    }

    // start the deletion by removing all displays for the target. This is accomplished by closing the initializer and then flushing.
    owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
    owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
    owner->getPatientAnalyze(sessionItemIndex)->removeAllTargetDisplays();
    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization())
      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseInitialization(); 
    clearTargetBeyondCurrentStage(currentTarget);
    pipeline->RemoveVesselTargetPipeline(currentTarget->targetPipelineID);
    currentTarget->targetPipelineID = null_ebID;
    logUpdateWithStageSettings("targetDefine::deleteTarget", currentTarget, currentTarget->getID());
    viewers->SyncViewersToScene();
    if (currentTarget->isViable)
      --viableTargetsCount;
    // now that it is out of the displays, remove it from the targets list
    for (int i=0; i < targets->size(); i++) {
      if (&(targets->at(i)) == currentTarget) {
        targetDefs()->removeAt(i);
      }
    }
    currentTarget = NULL; // since we just deleted it!
    selectCurrentTarget(NULL); // to catch up all the labeling etc.

    // finally, perform a switch target path
    switchTargetPath();
    viewers->SyncViewersToScene();
    viewers->Render();
  }
  else
    message->showMessage(tr("Program logic error; can't remove a target unless it is current, yet current is NULL"));
  this->setEnabled(true);
}

/*void targetDefine::renameTarget()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  bool ok;
  QString name = QInputDialog::getText(this, tr("Rename Current Target"), tr("ID:"), QLineEdit::Normal, currentTarget->getID(), &ok);
  if (ok) {
    currentTarget->setID(name);
    pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetPath()->SetName(name.toStdString()); 
    currentTarget->unsavedChanges = true;
    QString step = "targetDefine::renameTarget"; // log the update
    emit logUpdate(step, sessionItemIndex);
  }
}*/

void targetDefine::evaluateLumen()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;

  // first check to make sure at least one image is of type CT, exiting with apology if not
  int i;
  for (i=0; i < images->size(); i++) {
    // go through multiple levels of check to see if the specification is complete enough to add
    if (images->at(i).seriesType == "CT")
      break;
  } // end-for each listed series
  if (i == images->size()) {
    QMessageBox msg;
    msg.setWindowFlags(msg.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint);
    msg.setText(tr("Series types other than CT have been loaded but are not presently used for lumen evaluation."));
    msg.exec();
    return;
  } 

  // proceed if there is a current target
  if ((currentTarget != NULL) 
      && (!pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()->InitializationPointsEmpty())) {
    this->setEnabled(false);
    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenSegmentationPreConditions()) {
      // first reset the pipeline if necessary, i.e., if a prior lumen had existed
      if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation())
        pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLumenSegmentation();
      clearTargetBeyondCurrentStage(currentTarget);

      QITKProgressDialog progressIndicator(0,0);
      progressIndicator.setWindowModality(Qt::NonModal);
      progressIndicator.setMinimumDuration(10);
      progressIndicator.setWindowFlags(progressIndicator.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::WindowStaysOnTopHint);
      progressIndicator.setCancelButton(nullptr);   // no cancel button on dialog
      progressIndicator.show();
      progressIndicator.AddFilter(2,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentationFilter(),2);
      progressIndicator.AddFilter(3,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetPathFilter(),1);
      progressIndicator.AddFilter(4,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenPartitionFilter(),0.5);
      this->repaint(); // ensure progress is shown
      qApp->processEvents();
      
      // and proceed step-wise with the task
      viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Delineating Luminal Surface").toStdString());
      viewers->SyncViewersToScene();
      viewers->Render();
      try {
              pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenSegmentation();
      } catch (std::exception &e) {
              eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
        QMessageBox::warning(this, tr("Error computing"), QString("exception while computing object, suggest resetting target"));
        this->setEnabled(true);
        return; 
      }
      logUpdateWithStageSettings("targetDefine::ComputeLumenSegmentation", currentTarget, currentTarget->getID());
      currentTarget->unsavedChanges = true;
      currentTarget->lumenSegIDdefine = viewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation(), 0, "lumenSegmentation");
      clearTargetBeyondCurrentStage(currentTarget);
      viewers->SyncViewersToScene();
      viewers->Render();

      if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->PathPreConditions()) {
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Computing Detailed Cross Section Positions").toStdString());
        viewers->SyncViewersToScene();
        viewers->Render();
        try {
          pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputePath();
              } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
          QMessageBox::warning(this, tr("Error computing"), QString("exception while computing object, suggest resetting target"));
          this->setEnabled(true);
          return; 
        }
        ENABLECONTROL(tools->ToggleObliqueTool, tools->ToggleObliqueAction, tr("Press to toggle slice viewer alignment"));
        ENABLECONTROL(tools->MoveProximalTool, tools->MoveProximalAction, tr("Press to move proximally"));
        ENABLECONTROL(tools->MoveDistalTool, tools->MoveDistalAction, tr("Press to move distally"));
        logUpdateWithStageSettings("targetDefine::ComputeVesselTargetPath", currentTarget, currentTarget->getID());
        currentTarget->unsavedChanges = true;
        viewers->RemoveVesselTarget(currentTarget->initializerIDdefine); // now that we have a path, don't want to display the initializaer too
        currentTarget->initializerIDdefine = null_ebID;
        currentTarget->pathIDdefine = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetPath(), "path"); 
        clearTargetBeyondCurrentStage(currentTarget);
        viewers->SyncViewersToScene();
        viewers->Render();
        if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenPartitionPreConditions()) {
          viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Partitioning Lumen into Vessels").toStdString());
          viewers->SyncViewersToScene();
          viewers->Render();
          try {
            pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenPartition();
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
            QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
            this->setEnabled(true);
            return; 
          }
          logUpdateWithStageSettings("targetDefine::ComputeLumenPartition", currentTarget, currentTarget->getID());
          currentTarget->unsavedChanges = true;
          viewers->RemoveSegmentation4(currentTarget->lumenSegIDdefine); // if have partitions, don't want to display the segmentation too
          currentTarget->lumenSegIDdefine = null_ebID;
          int i = 0;
          for (auto partition : *pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenPartition()) {
            currentTarget->lumenPartIDsDefine.insert(viewers->AddSegmentation4(partition.GetPointer(), 0, "lumenPartition"+std::to_string(i++)));
          }
          clearTargetBeyondCurrentStage(currentTarget);
          viewers->SyncViewersToScene();
          viewers->Render();
        } // end-if can can compute global lumen
        else
          qWarning() << "evaluate lumen was not able to make it all the way through to the partitioned lumen.";
      } // end-if can compute vessel target path 
      
      viewers->GetViewer(ebvIdVolume)->SetTemporaryText("");
      viewers->SyncViewersToScene();
      viewers->Render();
      currentTarget->isViable = true; // now that there is a lumen the target may be considered viable :-)
      viableTargetsCount++;

      currentTarget->unsavedChanges = true;

      ENABLECONTROL(tools->ToggleLumenTool, tools->ToggleLumenAction, tr("Press to toggle lumen display"));
      tools->ToggleLumenTool->setChecked(true);
      tools->editSegmentationEnableState = true;
      tools->editSegmentationActionLabel = tr("Edit Lumen");

      ui->continueWithAnalysisButton->setEnabled(true); 
      owner->getPatientAnalyze(sessionItemIndex)->enableMenuActions(); // propagate it downstream

      //progressIndicator.setMaximum(100); // any value will take it down
    } // end-if can compute local lumen
    else
      message->showMessage(tr("Pipeline state does not allow to compute lumen, skipping."));
    this->setEnabled(true);
  }
  else
    message->showMessage(tr("Warning: Need a current target focus with completed initializer, skipping."));
}

void targetDefine::evaluateWall()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if ((currentTarget != NULL) 
      && (!pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()->InitializationPointsEmpty())) {
    this->setEnabled(false);
    // start by seeing if have to do lumen first
    if ((pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenSegmentationPreConditions())
        && (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation() == NULL)) {
      QMessageBox msgBox(this);
      msgBox.setText(tr("A lumen is needed before computing the wall, and preconditions to compute it are met."));
      msgBox.setInformativeText(tr("Do you want to compute the lumen now and go straight through to the wall?"));
      msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      msgBox.setDefaultButton(QMessageBox::Yes);
      msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
      int ret = msgBox.exec();
      if (ret == QMessageBox::Yes) 
        evaluateLumen();
      else {
        this->setEnabled(true);
        return;
      }
    }

    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->RegistrationPreConditions()) {
      // first reset the pipeline if necessary, i.e., if a prior lumen had existed
      if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetResampledRegisteredImages())
        pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseRegistration();
      clearTargetBeyondCurrentStage(currentTarget);

      // display a dialog for progress
      QITKProgressDialog progressIndicator(0,0);
      progressIndicator.setWindowModality(Qt::NonModal);
      progressIndicator.setMinimumDuration(10);
      progressIndicator.setWindowFlags(progressIndicator.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::WindowStaysOnTopHint);
      progressIndicator.setCancelButton(nullptr);   // no cancel button on dialog
      progressIndicator.show();
      progressIndicator.AddFilter(5,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetRegistrationFilter(),0.01);
      progressIndicator.AddFilter(6,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentationFilter(),3);
      progressIndicator.AddFilter(7,pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallPartitionFilter(),0.5);
      this->repaint(); // ensure progress is shown
      qApp->processEvents();

      // and proceed step-wise with the task
      viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Registering Images").toStdString());
      viewers->SyncViewersToScene();
      viewers->Render();
      try {
        pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeRegistration();
      } catch (std::exception &e) {
              eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
        QMessageBox::warning(this, tr("ebException computing"), QString(tr("object, suggest resetting target")));
        this->setEnabled(true);
        return;
      }
      logUpdateWithStageSettings("targetDefine::ComputeRegistration", currentTarget, currentTarget->getID());
      currentTarget->unsavedChanges = true;

      if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenAndWallSegmentationPreConditions()) {
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Delineating Outer Wall Surface").toStdString());
        viewers->SyncViewersToScene();
        viewers->Render();
        try {
                pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenAndWallSegmentation();
              } catch (std::exception &e) {
                eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
                QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
          this->setEnabled(true);
          return; 
        }
        logUpdateWithStageSettings("targetDefine::ComputeLumenAndWallSegmentation", currentTarget, currentTarget->getID());
        currentTarget->unsavedChanges = true;
        currentTarget->wallSegIDdefine = viewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation(), 0, "wallSegmentation");
              viewers->GetScene()->GetSegmentation4(currentTarget->wallSegIDdefine)->GetInteriorProperty()->SetOpacity(0);   
        clearTargetBeyondCurrentStage(currentTarget);
        viewers->SyncViewersToScene();
        viewers->Render();

        if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenAndWallPartitionPreConditions()) {
          viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Partitioning Wall For Specific Vessels").toStdString());
          viewers->SyncViewersToScene();
          viewers->Render();
          try {
            pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenAndWallPartition();
                } catch (std::exception &e) {
                  eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
            QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
            this->setEnabled(true);
            return; 
          }
          logUpdateWithStageSettings("targetDefine::ComputeLumenAndWallPartition", currentTarget, currentTarget->getID());
          currentTarget->unsavedChanges = true;
          viewers->RemoveSegmentation4(currentTarget->wallSegIDdefine); // if have partitions, don't want to display the segmentation too
          currentTarget->wallSegIDdefine = null_ebID;
          currentTarget->wallPartIDsDefine.clear();
          int i = 0;
          for (auto partition : *pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallPartition()) {
                  ebID partID = viewers->AddSegmentation4(partition.GetPointer(),0,"wallPartition"+std::to_string(i++));
                  currentTarget->wallPartIDsDefine.insert(partID);
                  viewers->GetScene()->GetSegmentation4(partID)->GetInteriorProperty()->SetOpacity(0);   
                }
          clearTargetBeyondCurrentStage(currentTarget);
          viewers->SyncViewersToScene();
          viewers->Render();
        } // end-if can can compute partitions
        else
          qWarning() << "evaluate wall was not able to make it all the way through to the local wall.";
        
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText("");
        viewers->SyncViewersToScene();
        viewers->Render();
 
        currentTarget->unsavedChanges = true;

        ENABLECONTROL(tools->ToggleWallTool, tools->ToggleWallAction, tr("Press to toggle wall display"));
        tools->ToggleWallTool->setChecked(true);
        tools->editSegmentationEnableState = true;
        tools->editSegmentationActionLabel = tr("Edit Wall");

        ui->continueWithAnalysisButton->setEnabled(true); 
        owner->getPatientAnalyze(sessionItemIndex)->enableMenuActions(); // propagate it downstream
        
        //progressIndicator.setMaximum(100); // any value will take it down
      } // end-if can compute wall  
      else
        message->showMessage(tr("Pipeline state does not allow to compute wall, skipping."));
    } // end-if can do registration
    else
      message->showMessage(tr("Pipeline state does not allow registration, skipping."));
    this->setEnabled(true);
  }
  else
    message->showMessage(tr("Warning: Need a current target focus with completed initializer, skipping."));
}

void targetDefine::editSegmentation(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if ((currentTarget != NULL) 
    && (!pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()->InitializationPointsEmpty())) {
    ui->continueWithAnalysisButton->setEnabled(false);
    ui->backToSurveyButton->setEnabled(false);
    this->repaint(); // to make sure that the button disables take effect
    qApp->processEvents();
    if (checked) {
      // start by swapping the volume and axial viewers, making the large view a slice view
      savedParallelScale = viewers->GetViewer2D(ebvIdAxial)->GetParallelScale();
      savedPerspectiveDistance = viewers->GetViewer3D(ebvIdVolume)->GetPerspectiveDistance();
      viewers->RemoveViewer(ebvIdAxial);
      viewers->RemoveViewer(ebvIdVolume);
      ebvIdVolume = viewers->AddViewer((tools->ToggleObliqueTool->isChecked() ? ebvViewer::OBLIQUE : ebvViewer::AXIAL),ui->volumeRender->GetRenderWindow());
      viewers->GetViewer(ebvIdVolume)->SetShowLogo(true);
      tools->resetViewer(ebvIdVolume, ebvViewer::AXIAL);
      ebvIdAxial = viewers->AddViewer(ebvViewer::THREED,ui->axialRender->GetRenderWindow());
      viewers->GetViewer(ebvIdAxial)->SetShowLogo(true);
      viewers->GetViewer3D(ebvIdAxial)->SetShowVolume(false);
      viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::AXIAL, false);
      viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::CORONAL, false);
      viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::SAGITTAL, false);
      viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::OBLIQUE, false);
      viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
      viewers->GetViewer3D(ebvIdAxial)->SetShowSlice(ebvViewer::CPR, tools->ToggleObliqueTool->isChecked());
      viewers->PanCamerasToCursor(true, true);
      savedSlabThickness = viewers->GetScene()->GetSlabThickness();
      viewers->GetScene()->SetSlabThickness(0);
      viewers->UpdateCameras();

      double window, level;
      ebAssert(viewers && viewers->GetScene() && viewers->GetScene()->GetSelectedImage4());
      viewers->GetScene()->GetSelectedImage4()->GetWindowLevel(window, level);
      viewers->GetViewer2D(ebvIdVolume)->SetParallelScale(.5*savedParallelScale);
      currentBackingSeries->window = window;
      currentBackingSeries->level = level;
      viewers->GetScene()->GetSelectedImage4()->SetWindowLevel(currentBackingSeries->window, currentBackingSeries->level);

      viewers->GetViewer3D(ebvIdAxial)->SetPerspectiveDistance(.3*savedPerspectiveDistance);
      ui->axialLabel->setText("Surface Rendering");
      // start by seeing if it is wall or lumen which will be edited
      if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation() != NULL) {
        currentTarget->wallSegIDdefine = viewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation(), 0, "wallSegmentation");
              viewers->GetScene()->GetSegmentation4(currentTarget->wallSegIDdefine)->GetInteriorProperty()->SetOpacity(0);  
        pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseCapThickness(); 
        pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseComposition(); 
        pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseWallThickness(); 
        pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLumenAndWallPartition();
        clearTargetBeyondCurrentStage(currentTarget); // this will take down the partitions and anything else downstream
        logUpdateWithStageSettings("targetDefine::EditLumenAndWallSegmentationStart", currentTarget, currentTarget->getID());
        viewers->SyncViewersToScene();
        viewers->Render();
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Editing mode on").toStdString());
              viewers->StartSegmentationEditor();
        editingSegmentation = true;
              segmentationEditor->show();
        tools->editSegmentationActionLabel = tr("Exit editing mode");
        editingWall = true;
        ui->backToSurveyButton->setEnabled(false); // can't go back when in middle of operation
      }
      else if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation() != NULL) {
        currentTarget->lumenSegIDdefine = viewers->AddSegmentation4(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation(), 0, "lumenSegmentation");
        pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseLumenPartition();
        clearTargetBeyondCurrentStage(currentTarget); // this will take down the partitions and anything else downstream
        //pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()->SetVisibility(false); // since took off the path, the initializer would display but don't want it to obscure the edit
        if (currentTarget->initializerIDdefine) {
          viewers->RemoveVesselTarget(currentTarget->initializerIDdefine); // if have a path, don't want to display the initializer too
          currentTarget->initializerIDdefine = null_ebID;
        }
        logUpdateWithStageSettings("targetDefine::EditLumenSegmentationStart", currentTarget, currentTarget->getID());
        viewers->SyncViewersToScene();
        viewers->Render();
        viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Editing mode on").toStdString());
              viewers->StartSegmentationEditor();
              segmentationEditor->show();
        tools->editSegmentationActionLabel = tr("Exit editing mode");
        editingWall = false;
        ui->backToSurveyButton->setEnabled(false); // can't go back when in middle of operation
      }
      else {
        qWarning() << "no lumen or wall to edit.";
      }
      DISABLECONTROL(tools->ToggleCoronalTool, tools->ToggleCoronalAction, tr("ToggleCoronal is disabled during editing"));
      DISABLECONTROL(tools->ToggleSagittalTool, tools->ToggleSagittalAction, tr("ToggleSagittal is disabled during editing"));
      DISABLECONTROL(tools->ToggleAxialTool, tools->ToggleAxialAction, tr("ToggleAxial is disabled during editing"));
      DISABLECONTROL(tools->ToggleLumenTool, tools->ToggleLumenAction, tr("Toggle lumen display is disabled during editing"));
      DISABLECONTROL(tools->OptimizeSubvolumeTool, tools->OptimizeSubvolumeAction, tr("Optimization of subvolume is disabled during editing"));
      DISABLECONTROL(tools->AnnotateTool, tools->AnnotateAction, tr("Annotation is disabled during editing"));
      DISABLECONTROL(tools->TargetPathSwitchTool, tools->TargetPathSwitchAction, tr("Switch target path is disabled during editing"));
      DISABLECONTROL(tools->TargetDeleteTool, tools->TargetDeleteAction, tr("Delete target is disabled during editing"));
      DISABLECONTROL(tools->TargetCreateTool, tools->TargetCreateAction, tr("Create target is disabled during editing"));
      DISABLECONTROL(tools->ToggleObliqueTool, tools->ToggleObliqueAction, tr("Toggle slice viewer alignment is disabled during editing"));
      DISABLECONTROL(tools->VolumeRenderTool, tools->VolumeRenderAction, tr("Volume rendering is disabled during editing"));
      DISABLECONTROL(tools->ShowAsMovieTool, tools->ShowAsMovieAction, tr("Show as movie is disabled during editing"));
      DISABLECONTROL(tools->MoveProximalTool, tools->MoveProximalAction, tr("MoveProximal is disabled during editing"));
      DISABLECONTROL(tools->MoveDistalTool, tools->MoveDistalAction, tr("MoveDistal is disabled during editing"));
      ui->processingParametersButton->setEnabled(false); 
      // take down the parameter setting dialog if it may be up, because can't change paramters during editing
      if (currentTarget->parameters != NULL)
        currentTarget->parameters->cancelParameterSetting();
    }
    else { // stop editing and re-compute partitions
      logUpdateWithStageSettings("targetDefine::EditSegmentationStop", currentTarget, currentTarget->getID());
      viewers->StopSegmentationEditor();
      editingSegmentation = false;
      segmentationEditor->hide();
      currentTarget->unsavedChanges = true;
      this->setEnabled(false);
      ui->backToSurveyButton->setEnabled(true); // can go back now that operation is done
      // put viewers back to the default state
      viewers->RemoveViewer(ebvIdAxial);
      viewers->RemoveViewer(ebvIdVolume);
      ebvIdVolume = viewers->AddViewer(ebvViewer::THREED,ui->volumeRender->GetRenderWindow());
      viewers->GetViewer(ebvIdVolume)->SetShowLogo(true);
      tools->resetViewer(ebvIdVolume, ebvViewer::THREED);
      viewers->GetViewer3D(ebvIdVolume)->SetShowVolume(false);
      if (tools->ToggleObliqueTool->isChecked()) {
        viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, false);
        viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, false);
        viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, false);
        viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, tools->ToggleAxialTool->isChecked());
        viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, tools->ToggleCoronalTool->isChecked());
        viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, tools->ToggleSagittalTool->isChecked());
      }
      else {
        viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, tools->ToggleAxialTool->isChecked());
        viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, tools->ToggleCoronalTool->isChecked());
        viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, tools->ToggleSagittalTool->isChecked());
        viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, false);
        viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
        viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, false);
      }
      ENABLECONTROL(tools->ToggleCoronalTool, tools->ToggleCoronalAction, tr("Press to toggle coronal plane"));
      ENABLECONTROL(tools->ToggleSagittalTool, tools->ToggleSagittalAction, tr("Press to toggle sagittal plane"));
      ENABLECONTROL(tools->ToggleAxialTool, tools->ToggleAxialAction, tr("Press to toggle axial plane"));
      ENABLECONTROL(tools->ToggleLumenTool, tools->ToggleLumenAction, tr("Press to toggle lumen display"));
      ENABLECONTROL(tools->OptimizeSubvolumeTool, tools->OptimizeSubvolumeAction, tr("Press to toggle widgets allowing optimization of subvolume"));
      ENABLECONTROL(tools->AnnotateTool, tools->AnnotateAction, tr("Press to annotate"));
      if (targets->size() >= 1) {
        ENABLECONTROL(tools->TargetPathSwitchTool, tools->TargetPathSwitchAction, tr("Press to switch target path"));
      }
      else {
        DISABLECONTROL(tools->TargetPathSwitchTool, tools->TargetPathSwitchAction, tr("Switch target path is disabled"));
      }
      ENABLECONTROL(tools->TargetCreateTool, tools->TargetCreateAction, tr("Press to create a new target"));
      ENABLECONTROL(tools->TargetDeleteTool, tools->TargetDeleteAction, tr("Press to delete current target"));
      ENABLECONTROL(tools->TargetModifyTool, tools->TargetModifyAction, tr("Press to modify current target"));
      ENABLECONTROL(tools->ToggleObliqueTool, tools->ToggleObliqueAction, tr("Press to toggle slice viewer alignment"));
      ENABLECONTROL(tools->VolumeRenderTool, tools->VolumeRenderAction, tr("Press to toggle volume rendering"));
      ENABLECONTROL(tools->ShowAsMovieTool, tools->ShowAsMovieAction, tr("Press to show as movie"));
      ENABLECONTROL(tools->MoveProximalTool, tools->MoveProximalAction, tr("Press to move proximally"));
      ENABLECONTROL(tools->MoveDistalTool, tools->MoveDistalAction, tr("Press to move distally"));
      ui->processingParametersButton->setEnabled(true); 
      ebvIdAxial = viewers->AddViewer((tools->ToggleObliqueTool->isChecked() ? ebvViewer::OBLIQUE : ebvViewer::AXIAL),ui->axialRender->GetRenderWindow());
      viewers->GetViewer(ebvIdAxial)->SetShowLogo(true);
      //viewers->GetViewer2D(ebvIdAxial)->SetSlabLineVisible(0,false);
      viewers->GetScene()->SetImageSlabType(VTK_IMAGE_SLAB_MAX);
      viewers->GetScene()->SetSlabThickness(savedSlabThickness);
      viewers->UpdateCameras();
      ebAssert(viewers && viewers->GetScene() && viewers->GetScene()->GetSelectedImage4());
      viewers->GetScene()->GetSelectedImage4()->GetWindowLevel(currentBackingSeries->window, currentBackingSeries->level);
      viewers->GetScene()->GetSelectedImage4()->SetWindowLevel(currentBackingSeries->window, currentBackingSeries->level);
      viewers->GetViewer2D(ebvIdAxial)->SetParallelScale(savedParallelScale);
      viewers->GetViewer3D(ebvIdVolume)->SetPerspectiveDistance(savedPerspectiveDistance);
      ui->axialLabel->setText(tr("Maximum-Intensity Projection enabled (set slab thickness as desired)"));
      viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Editing mode off").toStdString());
      if (editingWall) {
        auto newLumenAndWall = LevelSetOr(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenSegmentation(), pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation());
        ImageCopyPixels(newLumenAndWall.GetPointer(), pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation());
        LevelSetReinit(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallSegmentation(),20);
        if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->LumenAndWallPartitionPreConditions()) {
          viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Paritioning Wall For Specific Vessels").toStdString());
          viewers->SyncViewersToScene();
          viewers->Render();
          try {
            pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenAndWallPartition();
          } catch (std::exception &e) {
            eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
            QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
            ui->continueWithAnalysisButton->setEnabled(true);
            ui->backToSurveyButton->setEnabled(true);
            this->setEnabled(true);
            return; 
          }
          logUpdateWithStageSettings("targetDefine::ComputeLumenAndWallPartition", currentTarget, currentTarget->getID());
          currentTarget->unsavedChanges = true;
          if (currentTarget->wallSegIDdefine != null_ebID)
            viewers->RemoveSegmentation4(currentTarget->wallSegIDdefine); // if have partitions, don't want to display the segmentation too
          currentTarget->wallSegIDdefine = null_ebID;
          currentTarget->wallPartIDsDefine.clear();
          int i = 0;
          for (auto partition : *pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenAndWallPartition()) {
            ebID partID = viewers->AddSegmentation4(partition.GetPointer(),0,"wallPartition"+std::to_string(i++));
            currentTarget->wallPartIDsDefine.insert(partID);
            viewers->GetScene()->GetSegmentation4(partID)->GetInteriorProperty()->SetOpacity(0);   
          }
          clearTargetBeyondCurrentStage(currentTarget);
          tools->editSegmentationActionLabel = tr("Edit Wall");
          viewers->SyncViewersToScene();
          viewers->Render();
        } // end-if can can compute partitions
        else
          qWarning() << "evaluate wall was not able to re-partition.";
      }
      else {
        pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ClosePath();
        if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->PathPreConditions()) {
          // re-compute new path
          viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Computing Detailed Cross-section Positions").toStdString());
          viewers->SyncViewersToScene();
          viewers->Render();
          try {
            pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputePath();
          } catch (std::exception &e) {
            eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
            QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
            ui->continueWithAnalysisButton->setEnabled(true);
            ui->backToSurveyButton->setEnabled(true);
            this->setEnabled(true);
            return; 
          }
          logUpdateWithStageSettings("targetDefine::ComputeVesselTargetPath", currentTarget, currentTarget->getID());
          currentTarget->unsavedChanges = true;
          if (currentTarget->initializerIDdefine) {
            viewers->RemoveVesselTarget(currentTarget->initializerIDdefine); // if have a path, don't want to display the initializer too
            currentTarget->initializerIDdefine = null_ebID;
          }
          currentTarget->pathIDdefine = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetPath(), "path"); 
          clearTargetBeyondCurrentStage(currentTarget);

          // re-compute new partition
          viewers->GetViewer(ebvIdVolume)->SetTemporaryText(tr("Partitioning Lumen into Vessels").toStdString());
          viewers->SyncViewersToScene();
          viewers->Render();
          try {
            pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->ComputeLumenPartition();
          } catch (std::exception &e) {
            eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
            QMessageBox::warning(this, tr("Error computing"), QString(tr("exception while computing object, suggest resetting target")));
            ui->continueWithAnalysisButton->setEnabled(true);
            ui->backToSurveyButton->setEnabled(true);
            this->setEnabled(true);
            return; 
          }
          logUpdateWithStageSettings("targetDefine::ComputeLumenPartition", currentTarget, currentTarget->getID());
          currentTarget->unsavedChanges = true;
          if (currentTarget->lumenSegIDdefine != null_ebID)
            viewers->RemoveSegmentation4(currentTarget->lumenSegIDdefine); // if have partitions, don't want to display the segmentation too
          currentTarget->lumenSegIDdefine = null_ebID;
          int i = 0;
          for (auto partition : *pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetLumenPartition()) {
            currentTarget->lumenPartIDsDefine.insert(viewers->AddSegmentation4(partition.GetPointer(), 0, "lumenPartition"+std::to_string(i++)));
          }
          clearTargetBeyondCurrentStage(currentTarget);

          // sync up the display
          tools->editSegmentationActionLabel = tr("Edit Lumen");
          viewers->SyncViewersToScene();
          viewers->Render();
        } // end-if can can compute global lumen
        else {
          qWarning() << "evaluate lumen was not able to re-partition.";
        }
      }
      ui->continueWithAnalysisButton->setEnabled(true);
      ui->backToSurveyButton->setEnabled(true);
    }

    viewers->SyncViewersToScene();
    viewers->Render(); 
    this->setEnabled(true);
  }
}

void targetDefine::resetWall()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->RegistrationPreConditions()) {
    // first reset the pipeline if necessary, i.e., if a prior lumen had existed
    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetResampledRegisteredImages()) {
      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseRegistration();
    }
    clearTargetBeyondCurrentStage(currentTarget);
  }
  viewers->SyncViewersToScene();
  viewers->Render();
}

void targetDefine::resetTarget()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (currentTarget != NULL) {
    this->setEnabled(false);
    // determine whether there are unsaved changes and ask whether user wants to save them if so if so
    if (currentTarget->unsavedChanges) {
      QMessageBox msgBox(this);
      msgBox.setText(tr("There are unsaved changes which will be lost if the target is reset."));
      msgBox.setInformativeText(tr("Do you want to save these changes before deleting?"));
      msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
      msgBox.setDefaultButton(QMessageBox::No);
      msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
      int ret = msgBox.exec();
      if (ret == QMessageBox::Yes) {
        owner->save();
      }
    }

    // close the old initializer
    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization()) {
      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CloseInitialization();  
    }
    clearTargetBeyondCurrentStage(currentTarget);
    viewers->SyncViewersToScene();
    viewers->Render();

    // now start the sequence for the new initializer
    if (pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->InitializationPreConditions()) {
      pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->CreateInitialization();
      currentTarget->initializerIDdefine = viewers->AddVesselTarget(pipeline->GetVesselTargetPipeline(currentTarget->targetPipelineID)->GetInitialization(), "initializer"); 
      createTargetPre(currentTarget->getBodySite());
    }
    else
      message->showMessage(tr("Pipeline state out of sync, skipping."));

    logUpdateWithStageSettings("targetDefine::resetTarget", currentTarget, currentTarget->getID());
    this->setEnabled(true);
  }
  else
    message->showMessage(tr("Program logic error; can't remove a target unless it is current, yet current is NULL"));
}

void targetDefine::toggleOblique(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << (checked ? "OBLIQUE/CPR" : "AXIS-ALIGNED") << std::endl;
  if (owner->getWorkItemProcess(sessionItemIndex)->currentIndex() != TARGETDEFINE) {
    return;
  }
  viewers->RemoveViewer(ebvIdAxial);
  viewers->RemoveViewer(ebvIdCoronal);
  if (checked) {
    // make it oblique
    axialType = ebvViewer::OBLIQUE;
    ebvIdAxial = viewers->AddViewer(axialType,ui->axialRender->GetRenderWindow());
    viewers->GetViewer(ebvIdAxial)->SetShowLogo(true);
    coronalType = ebvViewer::CPR;
    ebvIdCoronal = viewers->AddViewer(coronalType,ui->coronalRender->GetRenderWindow());
    ebAssert(viewers->GetViewerCPR(ebvIdCoronal));
    viewers->GetViewer(ebvIdCoronal)->SetShowLogo(true);
    viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, false);
    viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, false);
    viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, false);
    if (tools->ToggleAxialTool->isChecked())
      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, tools->ToggleAxialTool->isChecked());
    else
      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, false);
    if (tools->ToggleCoronalTool->isChecked())
      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, tools->ToggleCoronalTool->isChecked());
    else
      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
    if (tools->ToggleSagittalTool->isChecked())
      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, tools->ToggleSagittalTool->isChecked());
    else
      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, false);
  }
  else {
    // make it aligned
    axialType = ebvViewer::AXIAL;
    ebvIdAxial = viewers->AddViewer(axialType,ui->axialRender->GetRenderWindow());
    viewers->GetViewer(ebvIdAxial)->SetShowLogo(true);
    coronalType = ebvViewer::CORONAL;
    ebvIdCoronal = viewers->AddViewer(coronalType,ui->coronalRender->GetRenderWindow());
    viewers->GetViewer(ebvIdCoronal)->SetShowLogo(true);
    if (tools->ToggleAxialTool->isChecked())
      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, tools->ToggleAxialTool->isChecked());
    else
      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, false);
    if (tools->ToggleCoronalTool->isChecked())
      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, tools->ToggleCoronalTool->isChecked());
    else
      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, false);
    if (tools->ToggleSagittalTool->isChecked())
      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, tools->ToggleSagittalTool->isChecked());
    else
      viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, false);
    viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, false);
    viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
    viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, false);
  }
  tools->ToggleObliqueTool->setChecked(checked);
  viewers->SyncViewersToScene();
  viewers->Render();
}

void targetDefine::giveToAnalyzeIfNotAlreadyThere()
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (ui->seriesSelectionArea->count() != 0) // if not 0, then the screen is currently on targetDefine, so must give it to patientAnalyze
    on_continueWithAnalysisButton_clicked();
}

void targetDefine::toggleSegmentation(QString regName, bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // loop through targets, because the segmentation toggles effect all of them
  for (int i=0; i < targets->size(); i++) {
    // go through multiple levels of check to see if the specification is complete enough to add
    targetDef *def = &((*targets)[i]);
    // have to do this in layers, since partitions override segmentations
    if (regName.contains("lumen")) {
      if (def->lumenSegIDdefine != null_ebID)
        viewers->GetScene()->GetSegmentation4(def->lumenSegIDdefine)->SetVisibility(checked);
      if (!def->lumenPartIDsDefine.empty()) {
        for (auto partitionID : def->lumenPartIDsDefine) {
          viewers->GetScene()->GetSegmentation4(partitionID)->SetVisibility(checked);
        }
      }
    }
    if (regName.contains("wall")) {
      if (def->wallSegIDdefine != null_ebID) 
        viewers->GetScene()->GetSegmentation4(def->wallSegIDdefine)->SetVisibility(checked);
      if (!def->wallPartIDsDefine.empty()) {
        for (auto partitionID : def->wallPartIDsDefine) {
          viewers->GetScene()->GetSegmentation4(partitionID)->SetVisibility(checked);
        }
      }
    }
  } // end-for each listed target
  viewers->SyncViewersToScene();
  viewers->Render();
}

void targetDefine::on_backToSurveyButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  this->setEnabled(false);
  disableMenuActions();
  owner->getSeriesSurvey(sessionItemIndex)->ensureOnSeriesSurveyPage();
  this->setEnabled(true);
}

/** @} */

