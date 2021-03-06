// Copyright (c) Elucid Bioimaging
#ifndef IMAGEDATABASE_H_
#define IMAGEDATABASE_H_

#include "ebLog.h"
#include <QCoreApplication>
#include <QtDebug>
#include <dcmtk/dcmdata/dctk.h>
#include <QtWidgets/QApplication>
#include <QString>
#include <QTreeWidget>
#include <QProgressDialog>
#include <QList>
#include <QMap>
#include <string>
#include <map>
#include <mutex>

// Column numbers for dicomImagesTreeWidget
#define DICOM_IMAGES_COLUMN_DESCRIPTION   0
#define DICOM_IMAGES_COLUMN_TYPE          1  // hidden ("PATIENT","STUDY","SERIES")
#define DICOM_IMAGES_COLUMN_PATH          2  // hidden (file system path)
#define DICOM_IMAGES_COLUMN_FIRST_IMAGE   3  // hidden (first image in series)
#define DICOM_IMAGES_COLUMN_NUMBER_IMAGES 4

// Column numbers for dicomMetadataTreeWidget
#define DICOM_METADATA_COLUMN_TAG         0
#define DICOM_METADATA_COLUMN_NAME        1
#define DICOM_METADATA_COLUMN_VALUE       2

/** \brief Database of DICOM images
 *
 *  This class maintains a database of imported images for the CAP application.  Currently, it is implemented with
 *  a simple patient/study/series/image datastructure, which is saved to disk as a JSON file.  In the future, this
 *  could be made more robust using a true database with ACID properties.
 *
 *  Currently, ignores DICOMDIR files during import.
 *
 *  Files structure
 *  * Files saved in <DOCUMENTS>/CAP Image Data/Images/<PatientID>/<StudyInstanceUID>/<SeriesInstanceUID>/<Image>
 *  * Index saved in <DOCUMENTS>/CAP Image Data/Images/index.json
 *
 *  NOTES
 *  * Currently, importing the same SOP instance UID but with different patient ID will cause duplicates
 *
 *  TODO
 *  * handle errors/exceptions during import
 *
 *  \copyright Elucid Bioimaging
 *  \ingroup workItem
 **/

class ImageDatabase {
public:
  //! fast test for DICOM magic number ("DICM" starting at 128th byte); doesn't check IOD type
  static bool IsDICOMFile(std::string filename);

  //! filter images for original, not DERIVED/LOCALIZER, CT/MR, etc. (doesn't modify dcm but dcmtk requires non-const for read-only access)
  static bool AcceptDICOMFile(DcmFileFormat &dcm, bool requireOriginal, bool excludeLocalizer, bool excludePreContrast);
  
  //! constructor (make Images directory, open and update index.json)
  ImageDatabase();

  //!@{ \name Image Import
  
  //! import DICOM images by file copy with optional recursion into subdirectories and update/save index.json, return number imported
  unsigned int ImportImages(QString path, bool recursive, bool requireOriginal, bool excludeLocalizer, bool excludePreContrast, bool showProgress);

  //! populate tree widget with patient/study/series hierarchy
  void PopulateDICOMImagesTreeWidget(QTreeWidget *dicomImagesTreeWidget, double importDays) const;

  //! populate DICOM metadata tree widget given the selected DICOM Images tree widget item (non-const since it sets metadcm)
  void PopulateDICOMMetadataTreeWidget(QTreeWidget *dicomMetadataTreeWidget, QTreeWidgetItem *dicomImagesTreeWidgetItem);

  //! get DICOM metadata (from last populate of DICOM metadata tree widget) (doesn't modify metadcm but dcmtk requires non-const for read-only access)
  std::string GetDICOMMetadata(unsigned int group, unsigned int element, unsigned int index=0);

  //!@}

  //! three methods for manipulating the current and prior selections
  QString GetPriorSeriesPath() { 
    ebLog eblog(Q_FUNC_INFO); eblog << std::endl; 
    /*LockType lock(mutex);*/ 
    qDebug() << "returning priorSeriesPath: " << priorSeriesPath;
    return priorSeriesPath; 
  }
  void SetSelectedSeriesPath(QString selection) { 
    ebLog eblog(Q_FUNC_INFO); eblog << selection.toStdString() << std::endl;
    if (selection != priorSeriesPath) {
      // THIS IS WHERE THE METADATA "CACHE" WOULD BE RESET FROM WHATEVER IT WAS TO WHAT IT NOW SHOULD BE
      // (I.e., the first part of PopulateDICOMMetadataTreeWidget, which it would then not need to do, and noting that 
      // workItem::on_dicomImagesTreeWidget_itemPressed calls this fucntion when a new one has been pressed whether by 
      // the user of system generated by workItem selection, causing the correct state transition implictly.)
      // (Further note, this would then allow (but not require) the GetDICOMMetadata calls to be more robust by implementing
      // an "&ok" arg that could be checked by caller as an error check for ensuring that there was valid metadata at the
      // time of the call.)
      ;
      priorSeriesPath = selection;
    }
  }
  QString GetSelectedSeriesPath(QTreeWidget *dicomImagesTreeWidget) {
    ebLog eblog(Q_FUNC_INFO); eblog << std::endl;
    QString selectedSeriesPath;
    if (dicomImagesTreeWidget) {
      if (dicomImagesTreeWidget->selectedItems().size() == 1) {
        selectedSeriesPath = dicomImagesTreeWidget->selectedItems().first()->text(DICOM_IMAGES_COLUMN_PATH);
      }
    }
    else {
      selectedSeriesPath = "";
    }
    qDebug() << "returning selectedSeriesPath: " << selectedSeriesPath;
    return selectedSeriesPath;
  }
  QString ResetSelectedSeriesPath(QTreeWidget *dicomImagesTreeWidget) {
    ebLog eblog(Q_FUNC_INFO); eblog << std::endl;
    QString selectedSeriesPath = GetSelectedSeriesPath(dicomImagesTreeWidget);
    SetSelectedSeriesPath(selectedSeriesPath);
    qDebug() << "returning selectedSeriesPath: " << selectedSeriesPath;
    return selectedSeriesPath;
  }

  struct Sop 
  {
    QString uid;
    QString filepath;
  };

  QMap<QString, Sop> getStudySeriesSops(QString patid, QString studyUid)
  {
    QMap<QString, Sop> filemap;

    auto patient = patients.find(patid.toStdString());
    if (patient == patients.cend())
        return filemap;
    auto study = patient->second.studies.find(studyUid.toStdString());
    if (study == patient->second.studies.cend())
        return filemap;
    for (auto se : study->second.series) {
        for (auto sop : se.second.images) {
            Sop instance;
            instance.uid = QString::fromLatin1(sop.first.c_str());
            instance.filepath = QString::fromLatin1(sop.second.c_str());
            filemap[instance.filepath] = instance;
        }
    }
    return filemap;
  }

protected:
  // typedefs for clarity
  typedef std::string ImageFileNameType;
  typedef std::string SOPInstanceUIDType;
  typedef std::string SeriesInstanceUIDType;
  typedef std::string StudyInstanceUIDType;  
  typedef std::string PatientIDType;
  typedef std::lock_guard<std::mutex> LockType;
  
  //! DICOM Image Series
  struct Series {
    std::string seriesImportDateTime;
    std::string seriesDescription;
    std::map<SOPInstanceUIDType,ImageFileNameType> images;  //!< image file name mapped by SOP Instance UID
  };
  
  //! DICOM Study
  struct Study {
    std::string studyDescription;
    std::map<SeriesInstanceUIDType,Series> series;  //!< mapped by Series Instance UID
    std::map<std::string,OFString> seriesHashes;  //!< mapped by Series Instance UID hash
  };
  
  //! DICOM Patient
  struct Patient {
    std::string patientName;
    std::map<StudyInstanceUIDType,Study> studies;  //!< mapped by Study Instance UID
    std::map<std::string,OFString> studyHashes;  //!< mapped by Study Instance UID hash
  };
  
  //! hierarchy of patient/study/series/image (patients mapped by Patient ID)
  typedef std::map<PatientIDType,Patient> IndexType;
  
  QString       imagesPath;  //!< CAP Exam Data/Images directory
  QString       indexPath;   //!< filename for JSON index file
  IndexType     patients;    //!< patients mapped by Patient ID
  DcmFileFormat metadcm;     //!< file format for most recently loaded DICOM file used for filling in metadata widget
  mutable std::mutex mutex;
  
  
  QString priorSeriesPath; // to allow checking if they pressed the same one again

 
  //! import any new images in Images and update index.json (doesn't save index.json)
  unsigned int AddImages(QProgressDialog *progressDialog, QString path, bool recursive, bool requireOriginal, bool excludeLocalizer, bool excludePreContrast);
  
  //! clear the current index
  void ClearIndex() { LockType lock(mutex); patients.clear(); }
  
  //! remove from index any empty/missing images/series/study/patient, return if changes made
  bool CleanupIndex();

  //! load Images/index.json off disk, return true if successful
  bool OpenIndex();
  
  //! save Images/index.json to disk, return true if successful
  bool SaveIndex() const;

  //! does patientID , studyInstanceUID , seriesInstanceUID , sopInstanceUID exist in database?
  bool Exists(std::string patientID, std::string studyInstanceUID, std::string seriesInstanceUID, std::string sopInstanceUID) const;
  
  //! function to perform the hashes, checking for conflicts
  std::string toHash(std::string UID) const { return(std::to_string(qHash(QString(UID.c_str())))); }
  //LINE USED TO TEST THAT IT FINDS A CONFLICTstd::string toHash(std::string UID) const { return("a hash value"); }
  std::string HashUID(std::map<std::string,OFString> &hashes, OFString UID);
  
  //! get path to patient directory
  std::string GetPath(std::string patientID) const {
    return imagesPath.toStdString() + "/" + patientID;
  }

  //! get path to study directory
  std::string GetPath(std::string patientID, std::string studyInstanceUID) const {
    return imagesPath.toStdString() + "/" + patientID + "/" + toHash(studyInstanceUID);
  }

  //! get path to series directory
  std::string GetPath(std::string patientID, std::string studyInstanceUID, std::string seriesInstanceUID) const {
    return imagesPath.toStdString() + "/" + patientID + "/" + toHash(studyInstanceUID) + "/" + toHash(seriesInstanceUID);
  }

  //! get path to image file
  std::string GetPath(std::string patientID, std::string studyInstanceUID, std::string seriesInstanceUID, std::string imageFileName) const {
    return imagesPath.toStdString() + "/" + patientID + "/" + toHash(studyInstanceUID) + "/" + toHash(seriesInstanceUID) + "/" + imageFileName;
  }

  //! get path to first image file for a given patient/study/series
  std::string GetFirstImage(std::string patientID, std::string studyInstanceUID="", std::string seriesInstanceUID="") const;

};

#endif  // IMAGEDATABASE_H_
