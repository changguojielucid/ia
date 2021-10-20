// Copyright (c) Elucid Bioimaging

#ifndef SERIESSURVEY_H
#define SERIESSURVEY_H

#include "workItemListFileToken.h"
#include "capTools.h"
#include "processingParameters.h"
#include "ebiHelper.h"
#include "ebiImageReader.h"
#include "ebiThumbnailImageFilter.h"
#include "ebvLinkedViewers2.h"
#include "ebiVesselPipeline.h"
#include <QWidget>
#include <QString>
#include <QComboBox>
#include <QPushButton>
#include <QJsonObject>
#include <QList>
#include <QFileDialog>
#include <QBoxLayout>
#include <QDebug>
#include <QUuid>
#include <QThread>
#include <QLabel>
#include <QMouseEvent>
#include <QAction>
#include <QMenu>
#include <QErrorMessage>
#include <QStackedWidget>
#include <QToolButton>
#include <QProgressDialog>
#include <iostream>
#include <string>
#include <vector>

// forward declarations
class cap;

/**
 * @{ 
 ** 
 * @brief Series Survey Package
 *
 * The series package comprises the following related classes:
 *    
 * 1.         imageSeries
 *                 |
 * 2.         seriesSurvey
 *
 *  \copyright Elucid Bioimaging
 *  \ingroup seriesSurvey
 */

/**
 * @{ 
 ** 
 * @brief class workItemListProvenance to provide methods for the provenance of the workItem list as a whole.
 */

typedef itk::Image<float,3> ImageType;

/**
 * @brief class imageSeries represents the various aspects of each individual image series
 */
class imageSeries
{
 public:
  QVBoxLayout *seriesWidget;
  QWidget *seriesThumb;
  QLabel *seriesLabel;
  QFrame *line_2;
  QString token;      // unique to current session, not stored in work item list files
  QString seriesUID;  // dicom series uid
  QString seriesType, // the list of items that are stored in work item list files
          dicomUID,
          seriesAttributes,
          anatomy,
          make,
          model,
          sliceThickness,
          acquisitionContrast,
          contrastAgent,
          modality,
          magneticFieldStrength,
          convolutionKernel,
          kvp,
          mas,
          acquisitionDate,
          acquisitionTime;
  QString seriesFolder, seriesThumbFile;
  ebID imageID, image4IDsurvey, image4IDdefine, image4IDanalyze;
  bool isViable, makeBackingIfViable;
  QProgressDialog *progressIndicator;
  double window, level;

  imageSeries() {
    seriesWidget = NULL;
    seriesThumb = NULL;
    seriesLabel = NULL;
    line_2 = NULL;
    isViable = false;
  };

  ~imageSeries() {
    ;//for (auto &reader:reader3) reader = NULL; 
  };

  void readImageSeries(const QJsonObject &json) { 
    //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
    seriesType = json[seriesType_token].toString();
    dicomUID = json[dicomUID_token].toString();
    seriesAttributes = json[seriesAttributes_token].toString();
    anatomy = json[anatomy_token].toString();
    make = json[make_token].toString();
    model = json[model_token].toString();
    sliceThickness = json[sliceThickness_token].toString();
    acquisitionContrast = json[acquisitionContrast_token].toString();
    contrastAgent = json[contrastAgent_token].toString();
    modality = json[modality_token].toString();
    magneticFieldStrength = json[magneticFieldStrength_token].toString();
    convolutionKernel = json[convolutionKernel_token].toString();
    kvp = json[kvp_token].toString();
    mas = json[mas_token].toString();
    acquisitionDate = json[acquisitionDate_token].toString();
    acquisitionTime = json[acquisitionTime_token].toString();
    seriesFolder = json[seriesLocalFolderName_token].toString();
    seriesThumbFile = json[seriesThumbLocalFileName_token].toString();
    if (json.contains(seriesUID_token)) {
        seriesUID = json[seriesUID_token].toString();
    }
  }

  void writeImageSeries(QJsonObject &json) const { 
    //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
    json[seriesType_token] = seriesType; 
    json[dicomUID_token] = dicomUID; 
    json[seriesAttributes_token] = seriesAttributes;
    json[anatomy_token] = anatomy;
    json[make_token] = make;
    json[model_token] = model;
    json[sliceThickness_token] = sliceThickness;
    json[acquisitionContrast_token] = acquisitionContrast;
    json[contrastAgent_token] = contrastAgent;
    json[modality_token] = modality;
    json[magneticFieldStrength_token] = magneticFieldStrength;
    json[convolutionKernel_token] = convolutionKernel;
    json[kvp_token] = kvp;
    json[mas_token] = mas;
    json[acquisitionDate_token] = acquisitionDate;
    json[acquisitionTime_token] = acquisitionTime;
    json[seriesLocalFolderName_token] = seriesFolder;
    json[seriesThumbLocalFileName_token] = seriesThumbFile;
    json[seriesUID_token] = seriesUID;
  }

  bool updateSeries(imageSeries *oseries) {
    if (seriesUID == oseries->seriesUID || oseries->seriesUID.isEmpty() || seriesUID.isEmpty())
        return false;
    oseries->seriesType = seriesType;
    oseries->dicomUID = dicomUID;
    oseries->seriesAttributes = seriesAttributes;
    oseries->anatomy = anatomy;
    oseries->make = make;
    oseries->model = model;
    oseries->sliceThickness = sliceThickness;
    oseries->acquisitionContrast = acquisitionContrast;
    oseries->contrastAgent = contrastAgent;
    oseries->modality = modality;
    oseries->magneticFieldStrength = oseries->magneticFieldStrength;
    oseries->convolutionKernel = convolutionKernel;
    oseries->kvp = kvp;
    oseries->mas = mas;
    oseries->acquisitionDate = acquisitionDate;
    oseries->acquisitionTime = acquisitionTime;
    oseries->seriesFolder = seriesFolder;
    oseries->seriesThumbFile = seriesThumbFile;
    oseries->seriesUID = seriesUID;
    return true;
  }

  void pushSeriesParametersToPipeline(ebiMultiImageReader::Pointer multiReader) const { 
    ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
    SetMetaData(multiReader->GetImage(imageID), seriesType_token, seriesType.toStdString()); 
    SetMetaData(multiReader->GetImage(imageID), dicomUID_token, dicomUID.toStdString()); 
    SetMetaData(multiReader->GetImage(imageID), seriesAttributes_token, seriesAttributes.toStdString());
    SetMetaData(multiReader->GetImage(imageID), anatomy_token, anatomy.toStdString());
    SetMetaData(multiReader->GetImage(imageID), make_token, make.toStdString());
    SetMetaData(multiReader->GetImage(imageID), model_token, model.toStdString());
    SetMetaData(multiReader->GetImage(imageID), sliceThickness_token, sliceThickness.toStdString());
    SetMetaData(multiReader->GetImage(imageID), acquisitionContrast_token, acquisitionContrast.toStdString());
    SetMetaData(multiReader->GetImage(imageID), contrastAgent_token, contrastAgent.toStdString());
    SetMetaData(multiReader->GetImage(imageID), modality_token, modality.toStdString());
    SetMetaData(multiReader->GetImage(imageID), magneticFieldStrength_token, magneticFieldStrength.toStdString());
    SetMetaData(multiReader->GetImage(imageID), convolutionKernel_token, convolutionKernel.toStdString());
    SetMetaData(multiReader->GetImage(imageID), kvp_token, kvp.toStdString());
    SetMetaData(multiReader->GetImage(imageID), mas_token, mas.toStdString());
    SetMetaData(multiReader->GetImage(imageID), acquisitionDate_token, acquisitionDate.toStdString());
    SetMetaData(multiReader->GetImage(imageID), acquisitionTime_token, acquisitionTime.toStdString());
  }
};
/** @} */

namespace Ui {
class seriesSurvey;
}

/**
 * \ingroup seriesSurvey
 * @{ 
 ** 
 * @brief class seriesSurvey (in namepace Ui) (subclassed from QWidget): the main class with the series set as a whole and the methods to 
 * display and interact with it. Manages the screen elements associatd with the set of image series associated with a given workItem analysis.
 */
class seriesSurvey : public QWidget
{
  Q_OBJECT

public:
  explicit seriesSurvey(QWidget *p=0, QMenu *m=NULL, bool masterWithRespectToMenu=false);
  ~seriesSurvey();
  void preloadSurvey(QString product, int index, ebiVesselPipeline::Pointer p, QList<imageSeries> *imageSeriesSet);
  void resetWI(ebiVesselPipeline::Pointer p, QList<imageSeries> *imageSeriesSet);
  void whenSeriesHeadersIn(QString);
  void whenSeriesAllIn(QString);
  void establishTools(QString product, int index, bool masterWithRespectToMenu);
  QList<imageSeries> *imageSeriesSet() { /*ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; */ return images; };
  vtkSmartPointer<ebvLinkedViewers2> viewers;
  void disconnectMenuActions();
  void connectMenuActions();
  void disableMenuActions();
  void enableMenuActions();

public slots:
  void ensureOnSeriesSurveyPage();
  void on_proceedToAnalysisButton_clicked();

private:
  QString thisProduct;
  Ui::seriesSurvey *ui;
  cap *owner;
  QMenu *seriesSurveyMenu;
  bool preLoading;
  int viableSeriesCount;
  imageSeries *currentBackingSeries; // various methods need to know what series to effect
  ebID backingImageId; 
  QErrorMessage *message;
  int sessionItemIndex;
  ebiVesselPipeline::Pointer pipeline;
  QList<imageSeries> *images; // actual list is declared as a member of workItemListEntry, this is a pointer to it
  ebID ida, idc, ids; // viewer identifiers (axial, coronal, sagittal) on the seriesSurvey screen itself
  capTools *tools;
  QAction *addSeriesAction, *specifySeriesAction, *gotoSeriesSurveyAction;
  std::map<QAction *, bool> *menuActionEnabledMap;
  void enableToolButtons();
  void disableToolButtons();
  void addSeries(int index, bool makeBackingIfViable, imageSeries *series);
  void displaySeriesPropertyValues(imageSeries *series);
  void connectSeriesProperties(imageSeries *series);
  void disconnectSeriesProperties();
  void selectBackingSeries(imageSeries *series);
  bool confirmPropertyChange();
  bool eventFilter(QObject *obj, QEvent *event);

private slots:
  void on_addSeriesButton_clicked();
  void removeSeries();
  void specifySeries(); void specifySeriesCore();
  void on_doneWithPropertiesButton_clicked();
  void onSeriesType(QString str) { ebLog eblog(Q_FUNC_INFO); eblog << str.toStdString() << std::endl; currentBackingSeries->seriesType = str; currentBackingSeries->pushSeriesParametersToPipeline(pipeline->GetMultiImageReader()); currentBackingSeries->seriesLabel->setText(currentBackingSeries->seriesType); QString step = "seriesSurvey:specifySeries"; emit logUpdate(step, currentBackingSeries->token, NULL, sessionItemIndex); emit resetAllTargetsDueToChangesInImages();  }
  void onAnatomy(QString str) { ebLog eblog(Q_FUNC_INFO); eblog << str.toStdString() << std::endl; currentBackingSeries->anatomy = str; currentBackingSeries->pushSeriesParametersToPipeline(pipeline->GetMultiImageReader()); QString step = "seriesSurvey:specifySeries"; emit logUpdate(step, currentBackingSeries->token, NULL, sessionItemIndex); emit resetAllTargetsDueToChangesInImages();  }
  void onMake(QString str) { ebLog eblog(Q_FUNC_INFO); eblog << str.toStdString() << std::endl; currentBackingSeries->make = str; currentBackingSeries->pushSeriesParametersToPipeline(pipeline->GetMultiImageReader()); QString step = "seriesSurvey:specifySeries"; emit logUpdate(step, currentBackingSeries->token, NULL, sessionItemIndex); emit resetAllTargetsDueToChangesInImages();  }
  void onModality(QString str) { ebLog eblog(Q_FUNC_INFO); eblog << str.toStdString() << std::endl; currentBackingSeries->modality = str; currentBackingSeries->pushSeriesParametersToPipeline(pipeline->GetMultiImageReader()); QString step = "seriesSurvey:specifySeries"; emit logUpdate(step, currentBackingSeries->token, NULL, sessionItemIndex); emit resetAllTargetsDueToChangesInImages();  }
  void onFieldStrength(QString str) { ebLog eblog(Q_FUNC_INFO); eblog << str.toStdString() << std::endl; currentBackingSeries->magneticFieldStrength = str; currentBackingSeries->pushSeriesParametersToPipeline(pipeline->GetMultiImageReader()); QString step = "seriesSurvey:specifySeries"; emit logUpdate(step, currentBackingSeries->token, NULL, sessionItemIndex); emit resetAllTargetsDueToChangesInImages();  }
  void onSeriesAttributes(QString str) { ebLog eblog(Q_FUNC_INFO); eblog << str.toStdString() << std::endl; currentBackingSeries->seriesAttributes = str; currentBackingSeries->pushSeriesParametersToPipeline(pipeline->GetMultiImageReader()); QString step = "seriesSurvey:specifySeries"; emit logUpdate(step, currentBackingSeries->token, NULL, sessionItemIndex); emit resetAllTargetsDueToChangesInImages();  }
  void onModel(QString str) { ebLog eblog(Q_FUNC_INFO); eblog << str.toStdString() << std::endl; currentBackingSeries->model = str; currentBackingSeries->pushSeriesParametersToPipeline(pipeline->GetMultiImageReader()); QString step = "seriesSurvey:specifySeries"; emit logUpdate(step, currentBackingSeries->token, NULL, sessionItemIndex); emit resetAllTargetsDueToChangesInImages();  }
  void onAgent(QString str) { ebLog eblog(Q_FUNC_INFO); eblog << str.toStdString() << std::endl; currentBackingSeries->contrastAgent = str; currentBackingSeries->pushSeriesParametersToPipeline(pipeline->GetMultiImageReader()); QString step = "seriesSurvey:specifySeries"; emit logUpdate(step, currentBackingSeries->token, NULL, sessionItemIndex); emit resetAllTargetsDueToChangesInImages();  }
  void onThickness(QString str) { ebLog eblog(Q_FUNC_INFO); eblog << str.toStdString() << std::endl; currentBackingSeries->sliceThickness = str; currentBackingSeries->pushSeriesParametersToPipeline(pipeline->GetMultiImageReader()); QString step = "seriesSurvey:specifySeries"; emit logUpdate(step, currentBackingSeries->token, NULL, sessionItemIndex); emit resetAllTargetsDueToChangesInImages();  }
  void onKernel(QString str) { ebLog eblog(Q_FUNC_INFO); eblog << str.toStdString() << std::endl; currentBackingSeries->convolutionKernel = str; currentBackingSeries->pushSeriesParametersToPipeline(pipeline->GetMultiImageReader()); QString step = "seriesSurvey:specifySeries"; emit logUpdate(step, currentBackingSeries->token, NULL, sessionItemIndex); emit resetAllTargetsDueToChangesInImages();  }
  void onKVP(QString str) { ebLog eblog(Q_FUNC_INFO); eblog << str.toStdString() << std::endl; currentBackingSeries->kvp = str; currentBackingSeries->pushSeriesParametersToPipeline(pipeline->GetMultiImageReader()); QString step = "seriesSurvey:specifySeries"; emit logUpdate(step, currentBackingSeries->token, NULL, sessionItemIndex); emit resetAllTargetsDueToChangesInImages();  }
  void onMAS(QString str) { ebLog eblog(Q_FUNC_INFO); eblog << str.toStdString() << std::endl; currentBackingSeries->mas = str; currentBackingSeries->pushSeriesParametersToPipeline(pipeline->GetMultiImageReader()); QString step = "seriesSurvey:specifySeries"; emit logUpdate(step, currentBackingSeries->token, NULL, sessionItemIndex); emit resetAllTargetsDueToChangesInImages();  }
  void onSequence(QString str) { ebLog eblog(Q_FUNC_INFO); eblog << str.toStdString() << std::endl; currentBackingSeries->acquisitionContrast = str; currentBackingSeries->pushSeriesParametersToPipeline(pipeline->GetMultiImageReader()); QString step = "seriesSurvey:specifySeries"; emit logUpdate(step, currentBackingSeries->token, NULL, sessionItemIndex); emit resetAllTargetsDueToChangesInImages();  }

signals:
  void logUpdate(QString step, QString ID, stageParameters *stageParams, int sessionItemIndex);
  void giveScreenControlToDefineFromSurvey(QStackedWidget *seriesSelectionArea, imageSeries *series);
  void backingSeriesChanged(imageSeries *series);
  void resetAllTargetsDueToChangesInImages();
  void resetAllTargetsDueToDifferentImages();
};

/**
 * \ingroup seriesSurvey
 * @{ 
 ** 
 * @brief class UpdateSeriesInBackground provides means for background loading
 */
class UpdateSeriesInBackground : public vtkCommand 
{
public:
  static UpdateSeriesInBackground *New() { ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; return new UpdateSeriesInBackground; }
  virtual void Execute(vtkObject *caller, unsigned long eventId, void*) override {
    ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
    owner->whenSeriesAllIn(uid);
  }
  QString uid;
  seriesSurvey *owner;
};
/** @} */

#endif // SERIESSURVEY_H
