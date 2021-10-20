// Copyright (c) Elucid Bioimaging

#include "cap.h"
#include "ImageDatabase.h"
#include <QDir>
#include <QStandardPaths>
#include <QErrorMessage>
#include <QMessageBox>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <fstream>
#include <stack>

// for DCMTK DICOM tag name constants see /usr/local/include/dcmtk/dcmdata/dcdeftag.h

// =====================================================================
// public member functions
// =====================================================================

/*static*/ bool ImageDatabase::IsDICOMFile(std::string filename) {
  std::ifstream is(filename,std::ifstream::binary);
  if (is) {
    char magicNumber[5];
    is.seekg(128);
    is.read(magicNumber,4);
    magicNumber[4] = '\0';
    if (std::string(magicNumber) == "DICM")
      return true;
  }
  return false;
}

/*static*/ bool ImageDatabase::AcceptDICOMFile(DcmFileFormat &dcm, bool requireOriginal, bool excludeLocalizer, bool excludePreContrast) {
  bool filterImagesOnly = true;
  if (filterImagesOnly) {  
    // check for existence of pixel data
    if (!dcm.getDataset()->tagExists(DCM_PixelData) &&
  !dcm.getDataset()->tagExists(DCM_DoubleFloatPixelData) &&
  !dcm.getDataset()->tagExists(DCM_FloatPixelData) &&
  !dcm.getDataset()->tagExists(DCM_RETIRED_VariablePixelData))
      return false;
  }
  OFString value;
  if (requireOriginal) {
    dcm.getDataset()->findAndGetOFString(DCM_ImageType,value,0);
    if (value != "ORIGINAL")  return false;
  }
  if (excludeLocalizer) {
    dcm.getDataset()->findAndGetOFString(DCM_ImageType,value,2);
    if (value == "LOCALIZER")  return false;
  }
  if (excludePreContrast) {
    dcm.getDataset()->findAndGetOFString(DCM_ImageType,value,2);
    if (value == "PRE_CONTRAST")  return false;
  }
  bool filterModality = true;
  if (filterModality) {
    // only use CT/MR images
    OFString value;
    dcm.getDataset()->findAndGetOFString(DCM_Modality,value);
    if (!value.empty() && (value != "CT") && (value != "MR"))  return false;
  }
  return true;
}

ImageDatabase::ImageDatabase() {
  ebLog eblog(Q_FUNC_INFO);
  imagesPath = cap::getImagesDir();
  indexPath = imagesPath + "/index.json";
  OpenIndex();
  //bool changed = CleanupIndex();
  //if (changed)  SaveIndex();
  priorSeriesPath = "";
}

unsigned int ImageDatabase::ImportImages(QString path, bool recursive, bool requireOriginal, bool excludeLocalizer, bool excludePreContrast, bool showProgress) {
  LockType lock(mutex);
  ebLog eblog(Q_FUNC_INFO);
  CleanupIndex();
  unsigned int N = 0;
  if (showProgress) {
    QProgressDialog progressDialog;
    progressDialog.setWindowModality(Qt::NonModal);
    progressDialog.setMinimum(0);  // for spinning bar since number not known in advance
    progressDialog.setMaximum(0);  // for spinning bar since number not known in advance
    progressDialog.setCancelButton(nullptr);   // no cancel button on dialog
    progressDialog.setWindowFlags(progressDialog.windowFlags()|Qt::FramelessWindowHint);
    progressDialog.resize(970, progressDialog.height());
    progressDialog.show();
    int retval = AddImages(&progressDialog,path,recursive,requireOriginal,excludeLocalizer,excludePreContrast);
    if (retval < 0) {
      QMessageBox msg;
      msg.setText("Hash conflict, please clear imported images prior to importing new data.");
      msg.exec();
      return N; // the number added before the conflict
    } else {
      N = retval;
    }
    progressDialog.setValue(0);  // make dialog go away
  } else {
    int retval = AddImages(nullptr,path,recursive,requireOriginal,excludeLocalizer,excludePreContrast);
    if (retval < 0) {
      QMessageBox msg;
      msg.setText("Hash conflict, please clear imported images prior to importing new data.");
      msg.exec();
      return N; // the number added before the conflict
    } else {
      N = retval;
    }
  }
  std::cerr << N << " images imported from " << path.toStdString() << std::endl;
  SaveIndex();
  return N;
}

void ImageDatabase::PopulateDICOMImagesTreeWidget(QTreeWidget *dicomImagesTreeWidget, double importDays) const {
  LockType lock(mutex);
  ebLog eblog(Q_FUNC_INFO);
  std::cerr << "importDays=" << importDays << std::endl;
  if (!dicomImagesTreeWidget)  return;
  // clear DICOM images tree widget (note: QTreeWidget deletes items)
  dicomImagesTreeWidget->clear();
  // populate with patient/study/series
  for (auto const &patient : patients) {
    QTreeWidgetItem *patientItem = new QTreeWidgetItem;
    patientItem->setText(DICOM_IMAGES_COLUMN_DESCRIPTION,(patient.first+" ("+patient.second.patientName+")").c_str());
    patientItem->setText(DICOM_IMAGES_COLUMN_TYPE,"PATIENT");
    patientItem->setText(DICOM_IMAGES_COLUMN_PATH,GetPath(patient.first).c_str());
    patientItem->setText(DICOM_IMAGES_COLUMN_FIRST_IMAGE,GetFirstImage(patient.first).c_str());
    for (auto const &study : patient.second.studies) {
      QTreeWidgetItem *studyItem = new QTreeWidgetItem;
      studyItem->setText(DICOM_IMAGES_COLUMN_DESCRIPTION,study.second.studyDescription.c_str());
      studyItem->setText(DICOM_IMAGES_COLUMN_TYPE,"STUDY");
      studyItem->setText(DICOM_IMAGES_COLUMN_PATH,GetPath(patient.first,study.first).c_str());
      studyItem->setText(DICOM_IMAGES_COLUMN_FIRST_IMAGE,GetFirstImage(patient.first,study.first).c_str());
      for (auto const &series : study.second.series) {
        QDateTime importDateTime = QDateTime::fromString(series.second.seriesImportDateTime.c_str(),"yyyyMMdd-hhmm");
        qint64 secs = importDateTime.secsTo(QDateTime::currentDateTime());
        double days = secs/60.0/60.0/24.0;
        if (days <= importDays) {
          QTreeWidgetItem *seriesItem = new QTreeWidgetItem;
          seriesItem->setText(DICOM_IMAGES_COLUMN_DESCRIPTION,series.second.seriesDescription.c_str());
          seriesItem->setText(DICOM_IMAGES_COLUMN_TYPE,"SERIES");
          seriesItem->setText(DICOM_IMAGES_COLUMN_PATH,GetPath(patient.first,study.first,series.first).c_str());
          seriesItem->setText(DICOM_IMAGES_COLUMN_FIRST_IMAGE,GetFirstImage(patient.first,study.first,series.first).c_str());
          seriesItem->setData(DICOM_IMAGES_COLUMN_NUMBER_IMAGES,Qt::DisplayRole,(int)series.second.images.size());
          studyItem->addChild(seriesItem);  // note: QTreeWidget takes ownership of item
        }
      }
      if (studyItem->childCount() > 0)
        patientItem->addChild(studyItem);  // note: QTreeWidget takes ownership of item
      else
        delete studyItem;
    }
    if (patientItem->childCount() > 0)
      dicomImagesTreeWidget->addTopLevelItem(patientItem);  // note: QTreeWidget takes ownership of item
    else
      delete patientItem;
  }
  dicomImagesTreeWidget->expandAll();
}

void ImageDatabase::PopulateDICOMMetadataTreeWidget(QTreeWidget *dicomMetadataTreeWidget, QTreeWidgetItem *dicomImagesTreeWidgetItem) {
  LockType lock(mutex);
  ebLog eblog(Q_FUNC_INFO);
  if (!dicomMetadataTreeWidget || !dicomImagesTreeWidgetItem)  return;
  // clear DICOM metdata tree widget (note: QTreeWidget deletes items)
  dicomMetadataTreeWidget->clear();
  // read in first DICOM image
  QString firstImage = dicomImagesTreeWidgetItem->text(DICOM_IMAGES_COLUMN_FIRST_IMAGE);  // get hidden path column
  unsigned int maxElementLength = 10000;
  OFCondition status = metadcm.loadFile(firstImage.toStdString().c_str(),EXS_Unknown,EGL_noChange,maxElementLength);
  std::stack<QTreeWidgetItem *> treeWidgetItemStack;  // stack of tree widget items
  if (status.good()) {
    for (int iteration = 0; iteration < 2; ++iteration) {
      // recursively iterate over DICOM dataset     
      DcmItem *dcmdataset = (iteration ? dynamic_cast<DcmItem*>(metadcm.getDataset()) : dynamic_cast<DcmItem*>(metadcm.getMetaInfo()));
      DcmStack dcmstack;
      while (dcmdataset->nextObject(dcmstack,OFTrue).good()) {
  // get stack depth
  int depth = dcmstack.card() - 2;
  while (treeWidgetItemStack.size() > depth)
    treeWidgetItemStack.pop();
  // tag and tag name
  DcmTag dcmtag = dcmstack.top()->getTag();
  std::stringstream tagss;
  tagss << dcmtag;
  std::string tag = tagss.str();
  std::string tagName = dcmtag.getTagName();
  //std::cerr << depth << " " << tag << " " << tagName;
  // create tree widget item
  QTreeWidgetItem *treeWidgetItem = new QTreeWidgetItem;
  treeWidgetItem->setText(DICOM_METADATA_COLUMN_TAG,tag.c_str());
  treeWidgetItem->setText(DICOM_METADATA_COLUMN_NAME,tagName.c_str());
  // process element or item
  DcmElement *elem = dynamic_cast<DcmElement *>(dcmstack.top());
  if (elem) {  // nullptr if stack top is DcmItem instead of DcmElement
    std::stringstream valuess;
    std::string value;
    for (unsigned long int i = 0; i < elem->getVM(); ++i) {
      OFString ofstring;
      elem->getOFString(ofstring,i);
      valuess << ofstring << " ";
      unsigned long int MAX_VALUES_PRINTED = 5;  // cut off after 5 values printed
      if (i > MAX_VALUES_PRINTED) {
        valuess << "...";  // ellipsis to indicate values cut off
        break;
      }
    }
    value = valuess.str();
    //std::cerr << value;
    treeWidgetItem->setText(DICOM_METADATA_COLUMN_VALUE,value.c_str());
  }
  //std::cerr << std::endl;
  // add tree widget item to tree
  if (treeWidgetItemStack.size() > 0)
    treeWidgetItemStack.top()->addChild(treeWidgetItem);  // note: QTreeWidget takes ownership of item
  else
    dicomMetadataTreeWidget->addTopLevelItem(treeWidgetItem);  // note: QTreeWidget takes ownership of item
  treeWidgetItemStack.push(treeWidgetItem);
      }
    }
  }
  dicomMetadataTreeWidget->resizeColumnToContents(DICOM_METADATA_COLUMN_TAG);
}

std::string ImageDatabase::GetDICOMMetadata(unsigned int group, unsigned int element, unsigned int index/*=0*/) {
  LockType lock(mutex);
  ebLog eblog(Q_FUNC_INFO);
  OFString value;
  if (group == 0x0002)
    metadcm.getMetaInfo()->findAndGetOFString(DcmTag(group,element),value,index);
  else
    metadcm.getDataset()->findAndGetOFString(DcmTag(group,element),value,index);
  return std::string(value.c_str());
}

// =====================================================================
// protected member functions
// =====================================================================

unsigned int ImageDatabase::AddImages(QProgressDialog *progressDialog, QString path, bool recursive, bool requireOriginal, bool excludeLocalizer, bool excludePreContrast) {
  if (progressDialog)
    progressDialog->setLabelText(QApplication::translate("workItem", "Importing ", 0)+path);
  QFileInfo fileInfo(path);
  if (fileInfo.isDir()) {
    // --- path is directory ---
    unsigned int numberAdded = 0;
    // get all files
    QDir dir(path);
    dir.setFilter(QDir::Files);
    QFileInfoList fileInfoList = dir.entryInfoList();
    for (int i = 0; i < fileInfoList.size(); ++i) {
      int retval = AddImages(progressDialog,fileInfoList.at(i).filePath(),recursive,requireOriginal,excludeLocalizer,excludePreContrast);
      if (retval < 0) {
        return retval;
      }
      numberAdded += retval;
    }
    if (recursive) {
      // get all directories (ignore ".", "..", symbolic links)
      dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
      QFileInfoList fileInfoList = dir.entryInfoList();
      for (int i = 0; i < fileInfoList.size(); ++i) {
        int retval = AddImages(progressDialog,fileInfoList.at(i).filePath(),recursive,requireOriginal,excludeLocalizer,excludePreContrast);
        if (retval < 0) {
          return retval;
        }
        numberAdded += retval;
      }
    }
    return numberAdded;
  } else if (fileInfo.isFile()) {
    // --- path is file ---
    std::string fileName = fileInfo.fileName().toStdString();
    if ((fileName != "DICOMDIR") && IsDICOMFile(path.toStdString())) {
      // --- path is DICOM file (other than DICOMDIR) ---
      DcmFileFormat dcm;
      unsigned int maxElementLength = 10000;
      OFCondition status = dcm.loadFile(path.toStdString().c_str(),EXS_Unknown,EGL_noChange,maxElementLength);
      if (status.good()) {
        if (!AcceptDICOMFile(dcm,requireOriginal,excludeLocalizer,excludePreContrast))
          return 0;
        OFString patientID,patientName;
        OFString studyInstanceUID,studyDescription;
        OFString seriesInstanceUID,seriesDescription;
        OFString sopInstanceUID;  
        QString emptyQ = QApplication::translate("workItem", "EMPTY", 0);
        const char *empty = emptyQ.toStdString().c_str();
        dcm.getDataset()->findAndGetOFString(DCM_PatientID,patientID);
        dcm.getDataset()->findAndGetOFString(DCM_PatientName,patientName);
        dcm.getDataset()->findAndGetOFString(DCM_StudyInstanceUID,studyInstanceUID);
        dcm.getDataset()->findAndGetOFString(DCM_StudyDescription,studyDescription);
        dcm.getDataset()->findAndGetOFString(DCM_SeriesInstanceUID,seriesInstanceUID);
        dcm.getDataset()->findAndGetOFString(DCM_SeriesDescription,seriesDescription);
        dcm.getDataset()->findAndGetOFString(DCM_SOPInstanceUID,sopInstanceUID);
        if (patientID.empty())          patientID = empty;
        if (studyInstanceUID.empty())   studyInstanceUID = empty;
        if (seriesInstanceUID.empty())  seriesInstanceUID = empty;
        std::cerr << "  patient ID = " << patientID << std::endl; 
        std::cerr << "  study instance UID = " << studyInstanceUID << std::endl;
        std::cerr << "  series instance UID = " << seriesInstanceUID << std::endl;
        std::cerr << "  SOP instance UID = " << sopInstanceUID << std::endl;
        // check for existence of patient ID , study instance UID , series instance UID , SOP instance UID
        if (Exists(patientID.c_str(),studyInstanceUID.c_str(),seriesInstanceUID.c_str(),sopInstanceUID.c_str()))
          return 0;
        // if patient/study/series doesn't exist in index yet, auto fill metadata 
        if (patients.find(patientID.c_str()) == patients.cend()) {
          patients[patientID.c_str()].patientName = patientName.c_str();
        }
        Patient *patient = &patients.at(patientID.c_str());
        
        // try to add the study, ensuring no conflict
        std::string studyInstanceUIDhash = HashUID(patients[patientID.c_str()].studyHashes, studyInstanceUID);
        if (studyInstanceUIDhash.empty()) {
          return -1; // hash conflict
        }
        std::cerr << "  study hashes for patient " << patientID << ": " /*<< patients[patientID.c_str()].studyHashes*/ << std::endl;
        if (patient->studies.find(studyInstanceUID.c_str()) == patient->studies.cend())
          patient->studies[studyInstanceUID.c_str()].studyDescription = studyDescription.c_str();
        Study *study = &patient->studies.at(studyInstanceUID.c_str());

        // try to add the series, ensuring no conflict
        std::string seriesInstanceUIDhash = HashUID(patient->studies[studyInstanceUID.c_str()].seriesHashes, seriesInstanceUID);
        if (seriesInstanceUIDhash.empty()) {
          return -1; // hash conflict
        }
        std::cerr << "  series hashes for study " << studyInstanceUID << ": " /*<< study[studyInstanceUID.c_str()].c_str()].seriesHashes*/ << std::endl;
        if (study->series.find(seriesInstanceUID.c_str()) == study->series.cend()) {
          study->series[seriesInstanceUID.c_str()].seriesImportDateTime = QDateTime::currentDateTime().toString("yyyyMMdd-hhmm").toStdString();
          study->series[seriesInstanceUID.c_str()].seriesDescription = seriesDescription.c_str();
        }
        Series *series = &study->series.at(seriesInstanceUID.c_str());

        // create CAP Exam Data/Images sub-directory
        QString patientPath = imagesPath  + "/" + QString(patientID.c_str());
        QString studyPath   = patientPath + "/" + QString(studyInstanceUIDhash.c_str());
        QString seriesPath  = studyPath   + "/" + QString(seriesInstanceUIDhash.c_str());
        QDir dir(QDir::current());
        if (!dir.mkpath(seriesPath)) {
          //QMessageBox::warning(this, tr("Couldn't create images folder"), tr("Couldn't create images folder"));
          return 0;
        }
        if (series->images.find(sopInstanceUID.c_str()) == series->images.cend())
          series->images[sopInstanceUID.c_str()] = fileName;
        // copy image file
        QString destinationPath = seriesPath+"/"+fileName.c_str();
        if (!QFile::copy(path,destinationPath)) {
          //QMessageBox::warning(this, tr("Couldn't copy image"), tr("Couldn't copy image"));
          return 0;
        }
        // update progress dialog
        if (progressDialog) {
          progressDialog->setValue(progressDialog->value());
          QApplication::processEvents();
        }
        return 1;
      }
    }
  }
  return 0;
}

bool ImageDatabase::CleanupIndex() {
  //LockType lock(mutex);
  ebLog eblog(Q_FUNC_INFO);
  bool changed = false;
  // erase missing patient/study/series/image from index
  for (auto patient = patients.cbegin(); patient != patients.cend(); )
    if (!QFile::exists(GetPath(patient->first).c_str())) {
      std::cerr << "MISSING " << GetPath(patient->first) << std::endl;
      patient = patients.erase(patient);
      changed = true;
    } else {
      for (auto study = patient->second.studies.cbegin(); study != patient->second.studies.cend(); )
  if (!QFile::exists(GetPath(patient->first,study->first).c_str())) {
    std::cerr << "MISSING " << GetPath(patient->first,study->first) << std::endl;
    study = patients.at(patient->first).studies.erase(study);
    changed = true;
  } else {
    for (auto series = study->second.series.cbegin(); series != study->second.series.cend(); )
      if (!QFile::exists(GetPath(patient->first,study->first,series->first).c_str())) {
        std::cerr << "MISSING " << GetPath(patient->first,study->first,series->first) << std::endl;
        series = patients.at(patient->first).studies.at(study->first).series.erase(series);
        changed = true;
      } else {
        for (auto image = series->second.images.cbegin(); image != series->second.images.cend(); )
    if (!QFile::exists(GetPath(patient->first,study->first,series->first,image->second).c_str())) {
      std::cerr << "MISSING " << GetPath(patient->first,study->first,series->first,image->second) << std::endl;
      image = patients.at(patient->first).studies.at(study->first).series.at(series->first).images.erase(image);
      changed = true;
    } else {
      ++image;
    }
        ++series;
      }
    ++study;
  }
      ++patient;
    }
  return changed;
}

bool ImageDatabase::OpenIndex() {
  //LockType lock(mutex);
  ebLog eblog(Q_FUNC_INFO);
  QFile file(indexPath);
  if (file.open(QIODevice::ReadOnly)) {
    patients.clear();
    QString val;
    val = file.readAll();
    file.close();    
    QJsonDocument doc = QJsonDocument::fromJson(val.toUtf8());
    QJsonObject indexObject = doc.object();
    QJsonArray patientArray = indexObject.value("patients").toArray();
    for (auto const &patientValue : patientArray) {
      std::string patientID = patientValue.toObject().value("patientID").toString().toStdString();
      std::string patientName = patientValue.toObject().value("patientName").toString().toStdString();
      Patient *patient = &patients[patientID];
      patient->patientName = patientName;
      for (auto const &studyValue : patientValue.toObject().value("studies").toArray()) {
  std::string studyInstanceUID = studyValue.toObject().value("studyInstanceUID").toString().toStdString();
  std::string studyDescription = studyValue.toObject().value("studyDescription").toString().toStdString();
  Study *study = &patient->studies[studyInstanceUID];
  study->studyDescription = studyDescription;
  for (auto const &seriesValue : studyValue.toObject().value("series").toArray()) {
    std::string seriesInstanceUID    = seriesValue.toObject().value("seriesInstanceUID"   ).toString().toStdString();
    std::string seriesDescription    = seriesValue.toObject().value("seriesDescription"   ).toString().toStdString();
    std::string seriesImportDateTime = seriesValue.toObject().value("seriesImportDateTime").toString().toStdString();   
    Series *series = &study->series[seriesInstanceUID];
    series->seriesDescription = seriesDescription;
    series->seriesImportDateTime = seriesImportDateTime;
    for (auto const &imageValue : seriesValue.toObject().value("images").toArray()) {
      std::string SOPInstanceUID = imageValue.toObject().value("SOPInstanceUID").toString().toStdString();
      std::string fileName = imageValue.toObject().value("fileName").toString().toStdString();      
      series->images[SOPInstanceUID] = fileName;
    }
  }
      }
    }
    return true;
  }
  return false;
}

bool ImageDatabase::SaveIndex() const {
  //LockType lock(mutex);
  ebLog eblog(Q_FUNC_INFO);
  QFile file(indexPath);
  if (file.open(QIODevice::WriteOnly)) {
    QJsonObject indexObject;
    QJsonArray patientArray;
    for (auto const &patient : patients) {
      QJsonObject patientObject;
      patientObject["patientID"] = QJsonValue(patient.first.c_str());
      patientObject["patientName"] = QJsonValue(patient.second.patientName.c_str());
      QJsonArray studyArray;
      for (auto const &study : patient.second.studies) {
  QJsonObject studyObject;
  studyObject["studyInstanceUID"] = QJsonValue(study.first.c_str());
  studyObject["studyDescription"] = QJsonValue(study.second.studyDescription.c_str());
  QJsonArray seriesArray;
  for (auto const &series : study.second.series) {
    QJsonObject seriesObject;
    seriesObject["seriesInstanceUID"   ] = QJsonValue(series.first.c_str());
    seriesObject["seriesImportDateTime"] = QJsonValue(series.second.seriesImportDateTime.c_str());
    seriesObject["seriesDescription"   ] = QJsonValue(series.second.seriesDescription.c_str());
    QJsonArray imageArray;
    for (auto const &image : series.second.images) {
      QJsonObject imageObject;
      imageObject["SOPInstanceUID"] = QJsonValue(image.first.c_str());
      imageObject["fileName"] = QJsonValue(image.second.c_str());
      imageArray.append(imageObject);
    }
    seriesObject["images"] = imageArray;
    seriesArray.append(seriesObject);
  }
  studyObject["series"] = seriesArray;
  studyArray.append(studyObject);  
      }
      patientObject["studies"] = studyArray;
      patientArray.append(patientObject);
    }
    indexObject["patients"] = patientArray;    
    QJsonDocument doc(indexObject);
    file.write(doc.toJson());
    file.close();
    return true;
  } else {
    // couldn't write

    return false;
  }    
}

bool ImageDatabase::Exists(std::string patientID, std::string studyInstanceUID, std::string seriesInstanceUID, std::string sopInstanceUID) const {
  auto patient = patients.find(patientID);
  if (patient == patients.cend())
    return false;
  auto study = patient->second.studies.find(studyInstanceUID);
  if (study == patient->second.studies.cend())
    return false;
  auto series = study->second.series.find(seriesInstanceUID);
  if (series == study->second.series.cend())
    return false;
  return (series->second.images.find(sopInstanceUID) != series->second.images.cend());
}

std::string ImageDatabase::HashUID(std::map<std::string,OFString> &hashes, OFString UID) {
  std::map<std::string,OFString>::iterator it;
  std::string hash = toHash(UID.c_str());
  it = hashes.find(hash);
  if (it == hashes.end()) {
    // no hit, so no problem to add it
    hashes[hash] = UID;
    return hash;
  } else if (hashes.find(hash)->second == UID) {
    // it was already there but for same UID, no conflict
    return hash;
  } else {
    // conflict, some other UID hased to this same string!
    return std::string();
  }
}

std::string ImageDatabase::GetFirstImage(std::string patientID, std::string studyInstanceUID/*=""*/, std::string seriesInstanceUID/*=""*/) const {
  // patient
  if (patients.find(patientID) == patients.cend())  return "";
  const Patient *patient = &patients.at(patientID);
  if (patient->studies.empty())  return "";
  // study
  if (studyInstanceUID == "")
    studyInstanceUID = patient->studies.cbegin()->first;
  if (patient->studies.find(studyInstanceUID) == patient->studies.cend())  return "";
  const Study *study = &patient->studies.at(studyInstanceUID);
  // series
  if (seriesInstanceUID == "")
    seriesInstanceUID = study->series.cbegin()->first;
  if (study->series.find(seriesInstanceUID) == study->series.cend())  return "";
  const Series *series = &study->series.at(seriesInstanceUID);
  // image    
  if (series->images.empty())  return "";
  std::string imageFileName = series->images.begin()->second;
  // return full path to first image
  return imagesPath.toStdString() + "/" + patientID + "/" + toHash(studyInstanceUID) + "/" + toHash(seriesInstanceUID) + "/" + imageFileName;
}
