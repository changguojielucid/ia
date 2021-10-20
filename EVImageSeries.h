#pragma once

#include "EVFirst.h"
#include "manifest.h"
#include "EVWorkItemListFileToken.h"

typedef itk::Image<float, 3> ImageType;
class EVImageSeries
{
public:
  QString token;      // unique to current session, not stored in work item list files
  QString seriesUID;  // dicom series uid
  QString seriesType; // the list of items that are stored in work item list files
  QString dicomUID;
  QString seriesAttributes;
  QString anatomy;
  QString make;
  QString model;
  QString sliceThickness;
  QString acquisitionContrast;
  QString contrastAgent;
  QString modality;
  QString magneticFieldStrength;
  QString convolutionKernel;
  QString kvp;
  QString mas;
  QString acquisitionDate;
  QString acquisitionTime;
  QString seriesFolder;
  QString seriesThumbFile;

  ebID imageID;
  ebID image4IDdefine;

  bool isViable;
  bool makeBackingIfViable;
  double window;
  double level;

  EVImageSeries() {
    isViable           = false;
    imageID            = null_ebID;
    image4IDdefine     = null_ebID;

    kvp                = "120"; // default
    seriesType         = "CT"; // needed by AddImage4
    window             = 800;
    level              = 600;

  };

  virtual ~EVImageSeries() {
  };

  void readImageSeries(const QJsonObject &json) {
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

  bool updateSeries(EVImageSeries *oseries) {
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



