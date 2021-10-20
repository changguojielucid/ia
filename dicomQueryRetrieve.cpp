// Copyright (c) Elucid Bioimaging

#include "dicomQueryRetrieve.h"
#include "ui_dicomQueryRetrieve.h"
#include "workItem.h"
#include "cap.h"
#include "ebAssert.h"
#include <dcmtk/ofstd/ofcond.h>
#include <dcmtk/dcmdata/dcpath.h>
#include <dcmtk/dcmnet/diutil.h>
#include <dcmtk/dcmtls/tlstrans.h>
#include <dcmtk/dcmtls/tlslayer.h>
#include <QDateTime>
#include <QMessageBox>
#include <thread>
#ifdef _MSC_VER
#include <winsock.h>  // for gethostname()
#endif

// =====================================================================
// dicomQueryRetrieve::Network
// =====================================================================

dicomQueryRetrieve::Network::Network(dicomQueryRetrieve *qr, T_ASC_NetworkRole role, int acceptorPort, bool secure) {
  // initialize variables
  queryRetrieve = qr;
  net = nullptr;
  tLayer = nullptr;

  // winsock stuff
#ifdef HAVE_WINSOCK_H
  WSAData winSockData;
  /* we need at least version 1.1 */
  WORD winSockVersionNeeded = MAKEWORD( 1, 1 );
  WSAStartup(winSockVersionNeeded, &winSockData);
#endif

  // initialize network
  int timeout = 30;
  unsigned long options = 0;
  cond = ASC_initializeNetwork(role,acceptorPort,timeout,&net,options);
  if (cond.bad()) {
    error("Network initialization failure");
    return;
  }

  // optionally set up TLS transport layer
  if (secure) {
    // create TLS transport layer
    const char *readSeedFile = nullptr;
    tLayer = new DcmTLSTransportLayer(DICOM_APPLICATION_REQUESTOR,readSeedFile);
    if (!tLayer) {
      error("Can't create TLS transport layer");
      return;
    }

    // add trusted certificates (file and dir)
    int keyFileFormat = SSL_FILETYPE_PEM; // PEM key file format
    //int keyFileFormat = SSL_FILETYPE_ASN1; // DER key file format
    const char *certFile = nullptr;
    if (certFile)
      if (tLayer->addTrustedCertificateFile(certFile,keyFileFormat) != TCS_ok) {
        error("unable to load certificate file");
        return;
      }
    const char *certDir = nullptr;
    if (certDir)
      if (tLayer->addTrustedCertificateDir(certDir,keyFileFormat) != TCS_ok) {        
        error("unable to load certificate dir");
        return;
      }

    // set Diffie-Hellman parameters
    const char *dhParamFile = nullptr;
    if (dhParamFile)
      if (!tLayer->setTempDHParameters(dhParamFile)) {
        error("unable to load temporary DH parameter file");
        return;
      }

    // optionally authenticate (if true, supply private key and certificate; if false, anonymous TLS)
    bool authenticate = false;  // use anonymous TLS
    if (authenticate) {
      // authenticate by password
      const char *passwd = nullptr;
      if (passwd)
        tLayer->setPrivateKeyPasswd(passwd);

      // set private key file
      const char *privateKeyFile = nullptr;
      if (tLayer->setPrivateKeyFile(privateKeyFile,keyFileFormat) != TCS_ok) {
        error("unable to load private TLS key");
        return;
      }

      // set certificate file
      const char *certificateFile = nullptr;
      if (tLayer->setCertificateFile(certificateFile,keyFileFormat) != TCS_ok) {
        error("unable to load certificate file");
        return;
      }

      // check if private key matches certificate
      if (!tLayer->checkPrivateKeyMatchesCertificate()) {
        error("key does not match certificate");
        return;
      }
    }

    // set cipher suites
    OFString ciphersuites(TLS1_TXT_RSA_WITH_AES_128_SHA ":" SSL3_TXT_RSA_DES_192_CBC3_SHA);
    if (TCS_ok != tLayer->setCipherSuites(ciphersuites.c_str())) {
      error("unable to set selected cipher suites");
      return;
    }

    // optionally verify certificate
    DcmCertificateVerification certVerification = DCV_requireCertificate;    
    //certVerification = DCV_checkCertificate;
    certVerification = DCV_ignoreCertificate;
    tLayer->setCertificateVerification(certVerification);

    // set TLS transport layer
    int takeoverOwnership = 0;
    cond = ASC_setTransportLayer(net,tLayer,takeoverOwnership);
    if (cond.bad())
      error("couldn't set transport layer");
  }
}

/*virtual*/ dicomQueryRetrieve::Network::~Network() {
  // drop network
  cond = ASC_dropNetwork(&net);
  if (cond.bad())
    queryRetrieve->emitErrorMessageSignal(tr("Drop network failed"));
  
  // winsock stuff
#ifdef HAVE_WINSOCK_H
  WSACleanup();
#endif
  
  // delete TLS transport layer
  if (tLayer)
    delete tLayer;
}

// =====================================================================
// dicomQueryRetrieve::Association
// =====================================================================

dicomQueryRetrieve::Association::
Association(dicomQueryRetrieve *qr, std::string _serverAETitle, std::string _serverHost, unsigned short _serverPort,
            std::string _localAETitle,  std::string _localHost) {
  // initialize variables
  queryRetrieve = qr;
  params = nullptr;
  assoc  = nullptr;
  serverAETitle = _serverAETitle;
  serverHost    = _serverHost;
  serverPort    = _serverPort;
  localAETitle  = _localAETitle;
  localHost     = _localHost;

  // create association parameters
  OFCmdUnsignedInt maxReceivePDULength = ASC_DEFAULTMAXPDU;
  cond = ASC_createAssociationParameters(&params,maxReceivePDULength);
  if (cond.bad()) {
    error("Association parameters failure");
    return;
  }
  
  // set AP titles
  cond = ASC_setAPTitles(params, localAETitle.c_str(), serverAETitle.c_str(), NULL);
  if (cond.bad()) {
    error("Association set AP titles failure");
    return;
  }

  // set presentation addresses
  std::string server = serverHost + ":" + std::to_string(serverPort);
  ASC_setPresentationAddresses(params, localHost.c_str(), server.c_str());
}

void dicomQueryRetrieve::Association::
addPresentationContext(T_ASC_PresentationContextID presentationContextID,
                       const char* abstractSyntax,
                       const char* transferSyntaxList[],
                       int transferSyntaxListCount,
                       T_ASC_SC_ROLE proposedRole/*= ASC_SC_ROLE_DEFAULT*/) {
  cond = ASC_addPresentationContext(params,presentationContextID,abstractSyntax,transferSyntaxList,proposedRole);
  if (cond.bad())
    error("Add presentation context failed");
}

void dicomQueryRetrieve::Association::requestAssociation(Network &network) {
  // set transport layer type
  bool secure = (network.tLayer != nullptr);
  cond = ASC_setTransportLayerType(params,secure);
  if (cond.bad()) {
    error("Transport layer failure");
    return;
  }

  // request association
  cond = ASC_requestAssociation(network.net,params,&assoc);
  if (cond.bad()) {
    if (cond == DUL_ASSOCIATIONREJECTED)
      error("Association rejected");
    else
      error("Association request rejected");
    return;
  }
  
  // make sure at least 1 presentation context accepted
  if (ASC_countAcceptedPresentationContexts(params) == 0)
    error("No Acceptable Presentation Contexts");
}

T_ASC_PresentationContextID
dicomQueryRetrieve::Association::findAcceptedPresentationContextID(const char *abstractSyntax) const {
  return ASC_findAcceptedPresentationContextID(assoc,abstractSyntax);
}

/*virtual*/ dicomQueryRetrieve::Association::~Association() {
  // release association
  if (cond == EC_Normal) {
    cond = ASC_releaseAssociation(assoc);
    if (cond.bad()) {
      queryRetrieve->emitErrorMessageSignal(tr("Association release failed"));
      // release failed, try abort
      cond = ASC_abortAssociation(assoc);
      if (cond.bad())
        queryRetrieve->emitErrorMessageSignal(tr("Association abort failed"));
    }
  } else if (cond == DUL_PEERREQUESTEDRELEASE) {
    // server requested release so abort
    cond = ASC_abortAssociation(assoc);
    if (cond.bad())
      queryRetrieve->emitErrorMessageSignal(tr("Association abort failed"));
  } else if (cond == DUL_PEERABORTEDASSOCIATION) {
    // server aborted, nothing to do
  }

  // destroy association
  cond = ASC_destroyAssociation(&assoc);
  if (cond.bad())
    queryRetrieve->emitErrorMessageSignal(tr("Destroy association failed"));
}

// =====================================================================
// dicomQueryRetrieve
// =====================================================================

// ---------------------------------------------------------------------
// public methods
// ---------------------------------------------------------------------

dicomQueryRetrieve::dicomQueryRetrieve(QWidget *parent) : QDialog(parent) {
  ebLog eblog(Q_FUNC_INFO);
  ui = new Ui::dicomQueryRetrieve;
  ui->setupUi(this);
  owner = dynamic_cast<workItem *>(parent);
  ebAssert(owner);
  serverAETitle = "Server AE Title";
  serverHost = "127.0.0.1";
  serverPort = 104;  // standard privileged DICOM port
  // set localHost and localAETitle (as local hostname)
  char hostname[1024];
  gethostname(hostname,sizeof(hostname)-1);
  localAETitle = hostname;
  localHost = hostname;
  localPort = 11112;  // standard insecure non-privileged dicom port
  // 2761 ISCL secure port
  // 2762 TLS secure port
  secure = false;  
  queryResultCount = 0;
  cancelQuery = false;
  cancelRetrieve = false;
  threadedRetrieve = true;
  retrieveThreadRunning = false;
  
  // set vertical layout for query criteria scroll area
  ui->queryCriteriaScrollArea->setLayout(new QVBoxLayout);

  // validate limit to line edit
  QValidator *validator = new QIntValidator(1,1000,this);
  ui->limitToLineEdit->setValidator(validator);

  // add first query criterion
  addQueryCriterion();
  
  // hide columns of query results tree widget
  ui->queryResultsTreeWidget->setSelectionMode(QAbstractItemView::MultiSelection);
  ui->queryResultsTreeWidget->setColumnHidden(QUERY_RESULTS_COLUMN_TYPE,true);
  ui->queryResultsTreeWidget->setColumnHidden(QUERY_RESULTS_COLUMN_ID,true);
  ui->queryResultsTreeWidget->setColumnWidth(DICOM_IMAGES_COLUMN_DESCRIPTION,600);
  ui->lastQueriedLabel->setText("Last Query: ");

  // connect signals/slots
  connect(this,&dicomQueryRetrieve::errorMessageSignal,&dicomQueryRetrieve::errorMessage);
  connect(this,&dicomQueryRetrieve::setStatusSignal,&dicomQueryRetrieve::setStatus);
  connect(this,&dicomQueryRetrieve::importImagesSignal,owner,&workItem::importImages,Qt::BlockingQueuedConnection);
}

/*virtual*/ dicomQueryRetrieve::~dicomQueryRetrieve() {
  ebLog eblog(Q_FUNC_INFO);
  cancelRetrieve = true;
  int n = 0;
  while (retrieveThreadRunning) {
    QThread::msleep(100);
    if (n++ > 20)  break;  // don't wait too long
  }
  delete ui;
}

// ---------------------------------------------------------------------
// public slots
// ---------------------------------------------------------------------

void dicomQueryRetrieve::init(std::string _serverAETitle, std::string _serverHost, unsigned short _serverPort,
                              std::string _localAETitle, unsigned short _localPort, bool _secure) {
  ebLog eblog(Q_FUNC_INFO);

  // set server/local AE title, host, port
  serverAETitle = _serverAETitle;
  serverHost    = _serverHost;
  serverPort    = _serverPort;
  localAETitle  = _localAETitle;
  localPort     = _localPort;
  secure        = _secure;
  bool success = c_echo_scu();
  std::string serverString = tr("Server").toStdString() + ": " + _serverAETitle;
  serverString += " (" + _serverHost + ":" + std::to_string(serverPort) + " ";
  serverString += (secure ? tr("secure") : tr("not secure")).toStdString() + ") --  ";
  serverString += (success ? tr("OK") : tr("Not responding")).toStdString();
  serverString += "          " + tr("Local").toStdString() + ": " + _localAETitle + " (:" + std::to_string(localPort) + ")";
  ui->serverLabel->setText(serverString.c_str());
  if (!success)
    errorMessage(tr("Server not responding"));
}

void dicomQueryRetrieve::showAndRaise() {
  // raise window to top
  hide();
  show();
  raise();
}

// ---------------------------------------------------------------------
// protected static variables and methods
// ---------------------------------------------------------------------

/*static*/ void dicomQueryRetrieve::error(std::string str) {
  std::cerr << "DICOM Error: " << str << std::endl;
}

/*static*/ void dicomQueryRetrieve::errorMessage(QString str) {
  error(str.toStdString());
  QMessageBox msgBox;
  msgBox.setText(tr("Error: ")+str);
  msgBox.exec();
}

/*static*/ void dicomQueryRetrieve::findSCUCallback(void *callbackData, T_DIMSE_C_FindRQ *request,
                                                    int responseCount, T_DIMSE_C_FindRSP *rsp,
                                                    DcmDataset *responseIdentifiers) {
  std::cerr << "findSCUCallback" << std::endl;
  std::cerr << "  Received C-FIND Response " << responseCount << std::endl;
  FindSCUCallbackData *findSCUCallbackData = OFstatic_cast(FindSCUCallbackData*, callbackData);

  // ---=== optionally, save C-FIND responses to file for debugging purposes ===---
  bool extractResponsesToFile = false;
  if (extractResponsesToFile) {
    // create DICOM repsonse file name
    OFString outputFilename;
    char rspIdsFileName[1024];
    if (findSCUCallbackData->studyLevel)
      sprintf(rspIdsFileName,"study.rsp%04d.dcm",responseCount);
    else
      sprintf(rspIdsFileName,"series.rsp%04d.dcm",responseCount);
    OFStandard::combineDirAndFilename(outputFilename,cap::getImagesQueryDir().toStdString().c_str(),rspIdsFileName,OFTrue /*allowEmptyDirName*/);
    // save DICOM response file
    std::cerr << "Writing C-FIND response message to file: " << outputFilename << std::endl;
    DcmFileFormat dcmff(responseIdentifiers);
    dcmff.saveFile(outputFilename);
  }

  if (findSCUCallbackData->studyLevel) {
    // ---=== store study query response ===---
    OFString patientName,patientID,studyInstanceUID;
    responseIdentifiers->findAndGetOFString(DCM_StudyInstanceUID,studyInstanceUID);
    responseIdentifiers->findAndGetOFString(DCM_PatientID,patientID);
    responseIdentifiers->findAndGetOFString(DCM_PatientName,patientName);
    findSCUCallbackData->queryRetrieve->studyResponses[studyInstanceUID.c_str()] = *responseIdentifiers;
    findSCUCallbackData->queryRetrieve->emitSetStatusSignal(("Query study result " + patientName + " " + patientID).c_str());
  } else {
    // ---=== store series query response ===---
    OFString patientID,patientName;
    OFString seriesDescription,seriesInstanceUID;
    OFString studyDescription,studyInstanceUID;
    responseIdentifiers->findAndGetOFString(DCM_SeriesInstanceUID,seriesInstanceUID);
    findSCUCallbackData->queryRetrieve->seriesResponses[seriesInstanceUID.c_str()] = *responseIdentifiers;
    // get patient and study metadata in order to add query result
    responseIdentifiers->findAndGetOFString(DCM_SeriesDescription,seriesDescription);
    responseIdentifiers->findAndGetOFString(DCM_StudyInstanceUID,studyInstanceUID);
    DcmDataset *study = &findSCUCallbackData->queryRetrieve->studyResponses.at(studyInstanceUID.c_str());
    study->findAndGetOFString(DCM_PatientID,patientID);
    study->findAndGetOFString(DCM_PatientName,patientName);
    study->findAndGetOFString(DCM_StudyDescription,studyDescription);
    if (findSCUCallbackData->queryRetrieve->queryResultCount < findSCUCallbackData->queryRetrieve->getQueryResultLimit()) {
      findSCUCallbackData->queryRetrieve->addQueryResult(patientID.c_str(),patientName.c_str(),
                                                         studyDescription.c_str(),studyInstanceUID.c_str(),
                                                         seriesDescription.c_str(),seriesInstanceUID.c_str());
      ++findSCUCallbackData->queryRetrieve->queryResultCount;
    }
    findSCUCallbackData->queryRetrieve->emitSetStatusSignal(("Query series result " + seriesDescription).c_str());
  }
  
  // cancel (or abort if cancel fails)
  bool overLimit = (findSCUCallbackData->queryRetrieve->queryResultCount >= findSCUCallbackData->queryRetrieve->getQueryResultLimit());
  if (findSCUCallbackData->queryRetrieve->cancelQuery || overLimit) {
    // ==========================================
    // send C-CANCEL-FIND-RQ
    // ==========================================
    std::cerr << "Sending C-CANCEL-FIND Request" << std::endl;
    OFCondition cond = DIMSE_sendCancelRequest(findSCUCallbackData->assoc,findSCUCallbackData->presId, request->MessageID);
    if (cond.bad()) {
      // server didn't like it, try sending 'impolite' A-ABORT
      // NOTE: Osirix/Horos seems to commit a protocol error sending P-DATA PDUs after cancel
      cond = ASC_abortAssociation(findSCUCallbackData->assoc);
      if (cond.bad())
        error("Association abort failed");
    }
    findSCUCallbackData->queryRetrieve->emitSetStatusSignal("");
  }
}

/*static*/ void dicomQueryRetrieve::moveSCUCallback(void *callbackData, T_DIMSE_C_MoveRQ *request,
                                                    int responseCount, T_DIMSE_C_MoveRSP *rsp) {
  std::cerr << "moveSCUCallback" << std::endl;
  // see if user has canceled the retrieve
  MoveSCUCallbackData *moveSCUCallbackData = OFstatic_cast(MoveSCUCallbackData*, callbackData);
  if (moveSCUCallbackData->queryRetrieve->cancelRetrieve) {
    // ==========================================
    // send C-CANCEL-MOVE-RQ
    // ==========================================
    OFCondition cond = DIMSE_sendCancelRequest(moveSCUCallbackData->assoc,moveSCUCallbackData->presId,request->MessageID);
    if (cond != EC_Normal)
      moveSCUCallbackData->queryRetrieve->emitErrorMessageSignal(tr("Cancel Request Failed"));
    moveSCUCallbackData->queryRetrieve->emitSetStatusSignal("");
  }
}

/*static*/ void dicomQueryRetrieve::storeSCPCallback(void *callbackData,
                                                     T_DIMSE_StoreProgress *progress,
                                                     T_DIMSE_C_StoreRQ *req,
                                                     char *imageFileName, DcmDataset **imageDataSet,
                                                     T_DIMSE_C_StoreRSP *rsp,
                                                     DcmDataset **statusDetail) {
  // see if user has canceled the retrieve
  StoreSCPCallbackData *storeSCPCallbackData = OFstatic_cast(StoreSCPCallbackData*, callbackData);
  if (storeSCPCallbackData->queryRetrieve->cancelRetrieve)
    return;

  // progress->state may be DIMSE_StoreBegin, DIMSE_StoreEnd or other
  if (progress->state == DIMSE_StoreEnd) {
    std::cerr << "storeSCPCallback" << std::endl;  // don't bother logging unless store end
    *statusDetail = NULL;
    if ((imageDataSet != NULL) && (*imageDataSet != NULL)) {
      // set status string
      OFString seriesDescription,seriesInstanceUID;
      (*imageDataSet)->findAndGetOFString(DCM_SeriesInstanceUID,seriesInstanceUID);
      (*imageDataSet)->findAndGetOFString(DCM_SeriesDescription,seriesDescription);
      storeSCPCallbackData->queryRetrieve->emitSetStatusSignal(("Retrieving " +
                                                                seriesDescription + " " +
                                                                storeSCPCallbackData->imageFileName.c_str()).c_str());
      
      // make CAP Exam Data/Images/tmp/Retrieve/<seriesinstanceUID> dir exist
      QString retrieveSeriesDir = cap::getImagesRetrieveDir() + "/" + seriesInstanceUID.c_str();       
      storeSCPCallbackData->dirName = retrieveSeriesDir.toStdString();
      QDir().mkpath(retrieveSeriesDir);
      
      // write DICOM file to tmp dir
      OFString path;
      OFStandard::combineDirAndFilename(path,QDir::toNativeSeparators(retrieveSeriesDir).toStdString().c_str(),
                                        storeSCPCallbackData->imageFileName.c_str(),OFTrue/*allowEmptyDirName*/);
      std::cerr << "  writing DICOM file " << path << std::endl;
      if (OFStandard::fileExists(path))
        std::cerr << "    DICOM file already exists, overwriting " << path << std::endl;
      OFCondition cond = storeSCPCallbackData->dcmff->saveFile(path.c_str());
      if (cond.bad()) {
        storeSCPCallbackData->queryRetrieve->emitErrorMessageSignal(tr("Cannot write DICOM file: ",path.c_str()));
        rsp->DimseStatus = STATUS_STORE_Refused_OutOfResources;
      }

      // set response status
      if (rsp->DimseStatus == STATUS_Success) {
        // which SOP class and SOP instance?
        DIC_UI sopClass;
        DIC_UI sopInstance;
        if (!DU_findSOPClassAndInstanceInDataSet(*imageDataSet, sopClass, sopInstance, OFFalse/*opt_correctUIDPadding*/)) {
          storeSCPCallbackData->queryRetrieve->emitErrorMessageSignal(tr("bad DICOM file: ")+imageFileName);
          rsp->DimseStatus = STATUS_STORE_Error_CannotUnderstand;
        } else if (strcmp(sopClass, req->AffectedSOPClassUID) != 0)
          rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
        else if (strcmp(sopInstance, req->AffectedSOPInstanceUID) != 0)
          rsp->DimseStatus = STATUS_STORE_Error_DataSetDoesNotMatchSOPClass;
      }
    }
  }
}

/*static*/ OFCondition dicomQueryRetrieve::storeSCP(T_ASC_Association *assoc, T_DIMSE_Message *msg,
                                                    T_ASC_PresentationContextID presID, StoreSCPCallbackData &storeSCPCallbackData) {
  ebLog eblog(Q_FUNC_INFO);

  // create retrieved filename as (modality abbreviation).(SOP instance UID)
  T_DIMSE_C_StoreRQ *req = &msg->msg.CStoreRQ;
  std::string imageFileName = std::string(dcmSOPClassUIDToModality(req->AffectedSOPClassUID))
                              + "." + req->AffectedSOPInstanceUID;
  
  // create store callback data
  DcmFileFormat dcmff;
  storeSCPCallbackData.assoc = assoc;
  storeSCPCallbackData.imageFileName = imageFileName;
  storeSCPCallbackData.dcmff = &dcmff;
  
  // store SourceApplicationEntityTitle in file metaheader
  if (assoc && assoc->params) {
    const char *aet = assoc->params->DULparams.callingAPTitle;
    if (aet) dcmff.getMetaInfo()->putAndInsertString(DCM_SourceApplicationEntityTitle, aet);
  }
  
  // not bit preserving, allow implicit format conversion
  OFBool useMetaheader = OFTrue;
  int dimse_timeout = 0;
  T_DIMSE_BlockingMode blockMode = DIMSE_BLOCKING;
  DcmDataset *dset = dcmff.getDataset();
  // ==========================================
  // C-STORE SCP
  // ==========================================
  OFCondition cond = DIMSE_storeProvider(assoc, presID, req, NULL, useMetaheader,
                                         &dset, storeSCPCallback, &storeSCPCallbackData,
                                         blockMode, dimse_timeout);
  if (cond.bad()) {
    storeSCPCallbackData.queryRetrieve->emitErrorMessageSignal(tr("Store SCP Failed"));
    // remove file
    if (imageFileName == NULL_DEVICE_NAME)
      unlink(imageFileName.c_str());
  }
  
  return cond;
}

/*static*/ void dicomQueryRetrieve::subOpCallback(void *callbackData, T_ASC_Network *aNet, T_ASC_Association **subAssoc) {
  SubOpCallbackData *subOpCallbackData = OFstatic_cast(SubOpCallbackData*, callbackData);
  if (aNet == NULL) {
    subOpCallbackData->queryRetrieve->emitErrorMessageSignal(tr("No association"));
    return;
  }

  // see if user has canceled retrieve
  if (subOpCallbackData->queryRetrieve->cancelRetrieve) {
    if (subAssoc) {
      ASC_abortAssociation(*subAssoc);
      ASC_dropAssociation(*subAssoc);
      ASC_destroyAssociation(subAssoc);
    }
    return;
  }

  if (*subAssoc == NULL) {    
    // negotiate association
    std::cerr << "  subOpCallback: negotiate association" << std::endl;
    // adapted from movescu.cc acceptSubAssoc(aNet, subAssoc);
    const char *knownAbstractSyntaxes[] = { UID_VerificationSOPClass };
    const char *transferSyntaxes[] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
    int numTransferSyntaxes;

    // create sub-association
    OFCondition cond = ASC_receiveAssociation(aNet, subAssoc, ASC_DEFAULTMAXPDU);
    if (cond.good()) {
      transferSyntaxes[0] = UID_JPEG2000TransferSyntax;
      transferSyntaxes[1] = UID_JPEG2000LosslessOnlyTransferSyntax;
      transferSyntaxes[2] = UID_JPEGProcess2_4TransferSyntax;
      transferSyntaxes[3] = UID_JPEGProcess1TransferSyntax;
      transferSyntaxes[4] = UID_JPEGProcess14SV1TransferSyntax;
      transferSyntaxes[5] = UID_JPEGLSLossyTransferSyntax;
      transferSyntaxes[6] = UID_JPEGLSLosslessTransferSyntax;
      transferSyntaxes[7] = UID_RLELosslessTransferSyntax;
      transferSyntaxes[8] = UID_MPEG2MainProfileAtMainLevelTransferSyntax;
      transferSyntaxes[9] = UID_MPEG2MainProfileAtHighLevelTransferSyntax;
      transferSyntaxes[10] = UID_MPEG4HighProfileLevel4_1TransferSyntax;
      transferSyntaxes[11] = UID_MPEG4BDcompatibleHighProfileLevel4_1TransferSyntax;
      transferSyntaxes[12] = UID_DeflatedExplicitVRLittleEndianTransferSyntax;
      // local little endian
      transferSyntaxes[13] = UID_LittleEndianExplicitTransferSyntax;
      transferSyntaxes[14] = UID_BigEndianExplicitTransferSyntax;
      transferSyntaxes[15] = UID_LittleEndianImplicitTransferSyntax;
      numTransferSyntaxes = 16;
      // accept the Verification SOP Class if presented
      cond = ASC_acceptContextsWithPreferredTransferSyntaxes((*subAssoc)->params,
                             knownAbstractSyntaxes, DIM_OF(knownAbstractSyntaxes),
                             transferSyntaxes, numTransferSyntaxes);      
      if (cond.good())
        // the array of Storage SOP Class UIDs comes from dcuid.h
        cond = ASC_acceptContextsWithPreferredTransferSyntaxes((*subAssoc)->params,
                               dcmAllStorageSOPClassUIDs, numberOfAllDcmStorageSOPClassUIDs,
                               transferSyntaxes, numTransferSyntaxes);
    }
    if (cond.good())
      cond = ASC_acknowledgeAssociation(*subAssoc);
    if (cond.good()) {
      if (ASC_countAcceptedPresentationContexts((*subAssoc)->params) == 0)
        subOpCallbackData->queryRetrieve->emitErrorMessageSignal(tr("No valid presentation contexts"));
    } else {
      subOpCallbackData->queryRetrieve->emitErrorMessageSignal(tr("Acknowledge Association Failed"));
      ASC_dropAssociation(*subAssoc);
      ASC_destroyAssociation(subAssoc);
    }
  } else {
    // be a service class provider
    // adapted from movescu.cc subOpSCP()
    T_DIMSE_Message msg;
    T_ASC_PresentationContextID presID;
    
    if (!ASC_dataWaiting(*subAssoc, 0)) /* just in case */
      return; //DIMSE_NODATAAVAILABLE;
    
    T_DIMSE_BlockingMode blockMode = DIMSE_BLOCKING;
    int dimse_timeout = 0;
    OFCondition cond = DIMSE_receiveCommand(*subAssoc, blockMode, dimse_timeout, &presID, &msg, NULL);
    
    StoreSCPCallbackData storeSCPCallbackData;
    if (cond == EC_Normal) {
      switch (msg.CommandField) {
      case DIMSE_C_ECHO_RQ:
        // process C-ECHO-RQ
        std::cerr << "  subOpCallback: process echo request" << std::endl;
        // ==========================================
        // C-ECHO SCP
        // ==========================================
        cond = DIMSE_sendEchoResponse(*subAssoc, presID, &msg.msg.CEchoRQ, STATUS_Success, NULL);
        break;
      case DIMSE_C_STORE_RQ:
        // process C-STORE-RQ
        std::cerr << "  subOpCallback: process store request" << std::endl;
        storeSCPCallbackData.queryRetrieve = subOpCallbackData->queryRetrieve;
        cond = storeSCP(*subAssoc, &msg, presID, storeSCPCallbackData);
        // add dir name from this store to the sub op callback data
        if (!storeSCPCallbackData.dirName.empty())
          subOpCallbackData->dirNames.insert(storeSCPCallbackData.dirName);
        else
          subOpCallbackData->queryRetrieve->emitErrorMessageSignal(tr("Directory name empty"));
        break;
      default:
        // we cannot handle this kind of message
        cond = DIMSE_BADCOMMANDTYPE;
        std::cerr << "  subOpCallback: unhandled message" << std::endl;
        subOpCallbackData->queryRetrieve->emitErrorMessageSignal(tr("Expected C-ECHO or C-STORE request but received other DIMSE command"));
        break;
      }
    }
    // clean up on association termination
    if (cond == DUL_PEERREQUESTEDRELEASE) {
      std::cerr << "  subOpCallback: process peer requested release" << std::endl;
      ASC_acknowledgeRelease(*subAssoc);
      ASC_dropAssociation(*subAssoc);
      ASC_destroyAssociation(subAssoc);
    } else if (cond == DUL_PEERABORTEDASSOCIATION) {
      std::cerr << "  subOpCallback: process peer aborted assoication" << std::endl;
      ASC_dropAssociation(*subAssoc);
      ASC_destroyAssociation(subAssoc);
    } else if (cond != EC_Normal) {
      // some kind of error so abort the association
      subOpCallbackData->queryRetrieve->emitErrorMessageSignal(tr("DIMSE failure"));
      ASC_abortAssociation(*subAssoc);
      ASC_dropAssociation(*subAssoc);
      ASC_destroyAssociation(subAssoc);
    }    
  }
}

// ---------------------------------------------------------------------
// protected methods
// ---------------------------------------------------------------------

bool dicomQueryRetrieve::c_echo_scu() {
  // adapted from echoscu.cc
  ebLog eblog(Q_FUNC_INFO);

  // network
  Network network(this,NET_REQUESTOR,0,secure);
  if (network.cond.bad())
    return false;

  // association
  const char* transferSyntaxes[] = {
    UID_LittleEndianImplicitTransferSyntax, // default xfer syntax first
    UID_LittleEndianExplicitTransferSyntax,
    UID_BigEndianExplicitTransferSyntax,  
  };
  Association association(this,serverAETitle,serverHost,serverPort,localAETitle,localHost);
  association.addPresentationContext(1,UID_VerificationSOPClass,transferSyntaxes,3);
  association.requestAssociation(network);
  if (association.cond.bad())
    return false;
  
  DIC_US msgId = association.assoc->nextMsgID++;
  DIC_US status;
  DcmDataset *statusDetail = NULL;
  T_DIMSE_BlockingMode blockMode = DIMSE_BLOCKING;
  int dimse_timeout = 0;
  // ==========================================
  // C-ECHO SCU
  // ==========================================
  association.cond = DIMSE_echoUser(association.assoc, msgId,
                                    blockMode, dimse_timeout,
                                    &status, &statusDetail);
  if (statusDetail)  delete statusDetail;
  if (association.cond.bad()) {
    error("Echo failed");
    return false;
  }
  
  return true;
}

bool dicomQueryRetrieve::c_find_scu(const QueryMapType &queryMap, std::string studyInstanceUID/*=""*/) {
  // adapted from findscu.cc
  ebLog eblog(Q_FUNC_INFO);

  // network
  Network network(this,NET_REQUESTOR,0,secure);
  if (network.cond.bad())
    return false;

  // association
  Association association(this,serverAETitle,serverHost,serverPort,localAETitle,localHost);
  const char *abstractSyntax = nullptr;
  if (ui->rootComboBox->currentText() == tr("Study"))
    abstractSyntax = UID_FINDStudyRootQueryRetrieveInformationModel;
  else if (ui->rootComboBox->currentText() == tr("Patient"))
    abstractSyntax = UID_FINDPatientRootQueryRetrieveInformationModel;
  else
    ebAssert(false);
  const char* transferSyntaxes[] = {
    UID_LittleEndianImplicitTransferSyntax, // default xfer syntax first
    UID_LittleEndianExplicitTransferSyntax,
    UID_BigEndianExplicitTransferSyntax,  
  };
  association.addPresentationContext(1,abstractSyntax,transferSyntaxes,3);
  association.requestAssociation(network);
  if (association.cond.bad())
    return false;
  
  // create needed override keys
  bool studyLevel = studyInstanceUID.empty();
  OFList<OFString> overrideKeys;
  if (studyLevel) {
    overrideKeys.push_back("(0008,0052)=STUDY");  // query/retrieve level
    if (queryMap.find("(0010,0010)") == queryMap.end())  overrideKeys.push_back("(0010,0010)");  // patient name
    if (queryMap.find("(0010,0020)") == queryMap.end())  overrideKeys.push_back("(0010,0020)");  // patient ID
    if (queryMap.find("(0020,000d)") == queryMap.end())  overrideKeys.push_back("(0020,000d)");  // study instance UID  
    if (queryMap.find("(0008,1030)") == queryMap.end())  overrideKeys.push_back("(0008,1030)");  // study description
  } else {
    overrideKeys.push_back("(0008,0052)=SERIES");  // query/retrieve level
    overrideKeys.push_back("(0020,000d)="+OFString(studyInstanceUID.c_str()));  // study instance UID  
    if (queryMap.find("(0020,000e)") == queryMap.end())  overrideKeys.push_back("(0020,000e)");  // series instance UID
    if (queryMap.find("(0008,103e)") == queryMap.end())  overrideKeys.push_back("(0008,103e)");  // series description
  }
  // add additional override keys
  for (auto const &tagValue : queryMap)
    overrideKeys.push_back((tagValue.first+"="+tagValue.second).c_str());

  // convert override keys into DICOM dataset
  DcmFileFormat dcmff;
  DcmDataset *dset = dcmff.getDataset();
  DcmPathProcessor proc;
  proc.setItemWildcardSupport(OFFalse);
  proc.checkPrivateReservations(OFFalse);
  for (auto overrideKey : overrideKeys) {
    auto cond = proc.applyPathWithValue(dset,overrideKey);
    if (cond.bad()) {
      error("bad key");
      return false;
    }
  }

  // prepare C-FIND-RQ message
  T_DIMSE_C_FindRQ req;      
  bzero(OFreinterpret_cast(char*, &req), sizeof(req));
  strcpy(req.AffectedSOPClassUID, abstractSyntax);
  req.DataSetType = DIMSE_DATASET_PRESENT;
  req.Priority = DIMSE_PRIORITY_LOW;

  // find parameters
  T_DIMSE_BlockingMode blockMode = DIMSE_BLOCKING;
  int dimse_timeout = 0;
  OFCmdUnsignedInt maxReceivePDULength = ASC_DEFAULTMAXPDU;
  OFBool abortAssociation = OFFalse;
  DcmDataset *statusDetail = NULL;
  T_DIMSE_C_FindRSP rsp;
  T_ASC_PresentationContextID presId = association.findAcceptedPresentationContextID(abstractSyntax);

  // prepare the callback data
  FindSCUCallbackData findSCUCallbackData;
  findSCUCallbackData.studyLevel = studyLevel;
  findSCUCallbackData.assoc = association.assoc;
  findSCUCallbackData.presId = presId;
  findSCUCallbackData.queryRetrieve = this;
  
  // ==========================================
  // C-FIND SCU
  // ==========================================
  association.cond = DIMSE_findUser(association.assoc,presId,&req,dset,
                                    findSCUCallback, &findSCUCallbackData, blockMode, dimse_timeout,
                                    &rsp, &statusDetail);
  if (statusDetail != NULL)
    delete statusDetail;
  if (association.cond.bad()) {
    error("FindSCU query error");
    return false;
  }
  
  return true;
}

void dicomQueryRetrieve::c_move_scu_store_scp() {
  try {
    retrieveThreadRunning = true;

    // adapted movescu.cc
    ebLog eblog(Q_FUNC_INFO);
    
    // network
    Network network(this,NET_ACCEPTORREQUESTOR,localPort,secure);
    if (network.cond.bad()) {
      emit errorMessageSignal(tr("couldn't create network"));
      retrieveThreadRunning = false;
      return;
    }
    
    // NOTE: at least some DICOM servers do not work with multiple C-MOVE on a single association
    //   so the creation of association is put inside the loop
    while (true) {
      DcmDataset dataset;
      {
        std::lock_guard<std::mutex> lock(retrieveQueueMutex);
        std::cerr << "retrieveQueue.size()=" << retrieveQueue.size() << std::endl;
        if (retrieveQueue.empty())
          break;  // queue is empty, break out of while loop
        else
          dataset = retrieveQueue.front();  // don't remove until done retrieving to disallow user from redundantly adding
      }
      // association
      typedef struct {
        const char *findSyntax;
        const char *moveSyntax;
      } QuerySyntax;
      QuerySyntax querySyntax[3] = {
        { UID_FINDPatientRootQueryRetrieveInformationModel,
          UID_MOVEPatientRootQueryRetrieveInformationModel },
        { UID_FINDStudyRootQueryRetrieveInformationModel,
          UID_MOVEStudyRootQueryRetrieveInformationModel },
        { UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel,
          UID_RETIRED_MOVEPatientStudyOnlyQueryRetrieveInformationModel }
      };
      typedef enum {
        QMPatientRoot = 0,
        QMStudyRoot = 1,
        QMPatientStudyOnly = 2
      } QueryModel;
      QueryModel queryModel = QMStudyRoot; // QMPatientRoot
      const char* transferSyntaxes[] = {
        UID_LittleEndianImplicitTransferSyntax, // default xfer syntax first
        UID_LittleEndianExplicitTransferSyntax,
        UID_BigEndianExplicitTransferSyntax,  
      };
      Association association(this,serverAETitle,serverHost,serverPort,localAETitle,localHost);
      association.addPresentationContext(1,querySyntax[queryModel].findSyntax,transferSyntaxes,3);
      association.addPresentationContext(3,querySyntax[queryModel].moveSyntax,transferSyntaxes,3);
      association.requestAssociation(network);
      if (association.cond.bad()) {
        emit errorMessageSignal(tr("couldn't create association"));
        retrieveThreadRunning = false;
        return;
      }
      
      // adapted from from movescu.cc cmove() and moveSCU()
      
      // which presentation context should be used
      OFCondition cond = EC_Normal;
      const char *sopClass = querySyntax[queryModel].moveSyntax;
      T_ASC_PresentationContextID presId = association.findAcceptedPresentationContextID(sopClass);
      if (presId == 0) {
        cond = association.cond = DIMSE_NOVALIDPRESENTATIONCONTEXTID;
        emit errorMessageSignal(tr("no valid presentation context"));
        continue;
      }
      
      T_DIMSE_C_MoveRQ  req;
      T_DIMSE_C_MoveRSP rsp;
      DIC_US            msgId = association.assoc->nextMsgID++;
      DcmDataset       *rspIds = NULL;
      req.MessageID = msgId;
      strcpy(req.AffectedSOPClassUID,sopClass);
      req.Priority = DIMSE_PRIORITY_MEDIUM;
      req.DataSetType = DIMSE_DATASET_PRESENT;
      // set the C-MOVE-RQ destination to be me
      ASC_getAPTitles(association.assoc->params, req.MoveDestination, NULL, NULL);
      
      // set up move and sub op callback data
      MoveSCUCallbackData moveSCUCallbackData;
      moveSCUCallbackData.assoc = association.assoc;
      moveSCUCallbackData.presId = presId;
      moveSCUCallbackData.queryRetrieve = this;
      SubOpCallbackData subOpCallbackData;
      subOpCallbackData.queryRetrieve = this;
      
      T_DIMSE_BlockingMode blockMode = DIMSE_BLOCKING;
      int dimse_timeout = 0;
      OFBool ignorePendingDatasets = OFTrue;
      DcmDataset *statusDetail = NULL; 
      std::cerr << "DIMSE_moveUser" << std::endl;
      OFString seriesInstanceUID;
      dataset.findAndGetOFString(DCM_SeriesInstanceUID,seriesInstanceUID);
      std::cerr << "uid=" << seriesInstanceUID << std::endl;
      // ==========================================
      // C-MOVE SCU (C-STORE SCP in storeSCP())
      // ==========================================
      association.cond = DIMSE_moveUser(association.assoc, presId, &req, &dataset,
                                        moveSCUCallback, &moveSCUCallbackData,
                                        blockMode, dimse_timeout, network.net,
                                        subOpCallback, &subOpCallbackData,
                                        &rsp, &statusDetail, &rspIds, ignorePendingDatasets);    
      if (statusDetail != NULL)  delete statusDetail;      
      if (rspIds       != NULL)  delete rspIds;
      if (cond == EC_Normal) {
        // move/store succeeded
        std::cerr << "Move/Store Succeeded" << std::endl;
        // import images into image database
        if (subOpCallbackData.dirNames.empty())
          std::cerr << "No retrieved directories to import" << std::endl;
        for (auto dirName : subOpCallbackData.dirNames) {
          OFString seriesInstanceUID;
          dataset.findAndGetOFString(DCM_SeriesInstanceUID,seriesInstanceUID); 
          QString retrieveSeriesDir = cap::getImagesRetrieveDir() + "/" + seriesInstanceUID.c_str();       
          if (!cancelRetrieve) {
            std::cerr << "Importing " << dirName << std::endl;
            emit setStatusSignal(tr("Importing").toStdString()+" "+dirName);
            bool original = owner->getOwner()->systemPreferencesObject->getRequireOriginal();
            bool localizer = owner->getOwner()->systemPreferencesObject->getExcludeLocalizer();
            bool precontrast = owner->getOwner()->systemPreferencesObject->getExcludePreContrast();
            emit importImagesSignal(retrieveSeriesDir,false,original,localizer,precontrast,false);
          }
          // remove retrieve series dir
          QDir rdir(retrieveSeriesDir);
          rdir.setFilter(QDir::Files);
          QFileInfoList fileInfoList = rdir.entryInfoList();
          for (int i = 0; i < fileInfoList.size(); ++i)
            rdir.remove(fileInfoList.at(i).fileName());
          rdir.rmdir(retrieveSeriesDir);
        }
      } else {
        // move/store failed
        emit errorMessageSignal(tr("Move/Store failed"));
      }
      {
        std::lock_guard<std::mutex> lock(retrieveQueueMutex);
        if (!retrieveQueue.empty())
          retrieveQueue.pop_front();
      }
    }  // end while loop
    emit setStatusSignal("");
  } catch (std::exception &e) {
    std::cerr << "EXCEPTION CAUGHT: " << e.what() << std::endl;
    emit errorMessageSignal(tr("Move/Store exception thrown"));
  } catch (...) {
    std::cerr << "EXCEPTION CAUGHT: non-standard exception " << std::endl;
    emit errorMessageSignal(tr("Move/Store exception thrown"));
  }
  retrieveThreadRunning = false;
}

unsigned int dicomQueryRetrieve::getQueryResultLimit() const {
  return ui->limitToLineEdit->text().toInt();
}

dicomQueryRetrieve::QueryMapType dicomQueryRetrieve::getQueryMap() const {
  // get scroll area vertical layout
  QVBoxLayout *vlayout = dynamic_cast<QVBoxLayout *>(ui->queryCriteriaScrollArea->layout());
  ebAssert(vlayout);
  // find horizontal layout that contains remove button that was pressed
  QueryMapType queryMap;
  for (int i = 0; i < vlayout->count(); ++i) {
    QHBoxLayout *hlayout = dynamic_cast<QHBoxLayout*>(vlayout->itemAt(i));
    ebAssert(hlayout && sender());    
    QComboBox *tagComboBox    = dynamic_cast<QComboBox*>(hlayout->itemAt(0)->widget());
    QComboBox *matchComboBox  = dynamic_cast<QComboBox*>(hlayout->itemAt(1)->widget());
    QLineEdit *valueLineEdit  = dynamic_cast<QLineEdit*>(hlayout->itemAt(2)->widget());
    QLineEdit *value2LineEdit = dynamic_cast<QLineEdit*>(hlayout->itemAt(4)->widget());
    ebAssert(tagComboBox && matchComboBox && valueLineEdit && value2LineEdit);
    std::string tagString    = tagComboBox->itemData(tagComboBox->currentIndex()).toString().toStdString();
    std::string valueString  = valueLineEdit->text().toStdString();
    std::string value2String = value2LineEdit->text().toStdString();
    std::string value;
    // convert to value matching strings
    if (matchComboBox->currentText() == tr("contains"))
      value = "*" + valueString + "*";
    else if (matchComboBox->currentText() == tr("begins with"))
      value = valueString + "*";
    else if (matchComboBox->currentText() == tr("ends with"))
      value = "*" + valueString;
    else if (matchComboBox->currentText() == tr("on or before"))
      value = "-" + valueString;
    else if (matchComboBox->currentText() == tr("on or after"))
      value = valueString + "-";
    else if (matchComboBox->currentText() == tr("in range"))
      value = valueString + "-" + value2String;
    else
      value = valueString;  
    queryMap[tagString] = value;
  }
  return queryMap;
}

void dicomQueryRetrieve::addQueryResult(std::string patientID, std::string patientName,
                                        std::string studyDescription, std::string studyInstanceUID,
                                        std::string seriesDescription, std::string seriesInstanceUID) {
  std::cerr << "  adding query result to tree widget" << std::endl;
  std::cerr << "    " << patientName << " " << patientID << std::endl;
  std::cerr << "    " << studyDescription << " " << studyInstanceUID << std::endl;
  std::cerr << "    " << seriesDescription << " " << seriesInstanceUID << std::endl;
  // find or create patient level item
  QTreeWidgetItem *patientItem = nullptr;
  for (int i = 0; i < ui->queryResultsTreeWidget->topLevelItemCount(); ++i)
    if (ui->queryResultsTreeWidget->topLevelItem(i)->text(QUERY_RESULTS_COLUMN_ID) == patientID.c_str() &&
        ui->queryResultsTreeWidget->topLevelItem(i)->text(QUERY_RESULTS_COLUMN_DESCRIPTION) == patientName.c_str())
      patientItem = ui->queryResultsTreeWidget->topLevelItem(i);
  if (!patientItem) {
    patientItem = new QTreeWidgetItem;
    patientItem->setFlags(Qt::ItemIsEnabled);
    patientItem->setText(QUERY_RESULTS_COLUMN_ID,patientID.c_str());
    patientItem->setText(QUERY_RESULTS_COLUMN_TYPE,"PATIENT");
    patientItem->setText(QUERY_RESULTS_COLUMN_DESCRIPTION,patientName.c_str());
    ui->queryResultsTreeWidget->addTopLevelItem(patientItem);  // note: QTreeWidget takes ownership of item    
  }
  patientItem->setExpanded(true);
  // find or create study level item
  QTreeWidgetItem *studyItem = nullptr;
  for (int i = 0; i < patientItem->childCount(); ++i)
    if (patientItem->child(i)->text(QUERY_RESULTS_COLUMN_ID) == studyInstanceUID.c_str() &&
        patientItem->child(i)->text(QUERY_RESULTS_COLUMN_DESCRIPTION) == studyDescription.c_str())
      studyItem = patientItem->child(i);
  if (!studyItem) {
    studyItem = new QTreeWidgetItem;
    studyItem->setFlags(Qt::ItemIsEnabled);
    studyItem->setText(QUERY_RESULTS_COLUMN_ID,studyInstanceUID.c_str());
    studyItem->setText(QUERY_RESULTS_COLUMN_TYPE,"STUDY");
    studyItem->setText(QUERY_RESULTS_COLUMN_DESCRIPTION,studyDescription.c_str());
    patientItem->addChild(studyItem);  // note: QTreeWidget takes ownership of item    
  }
  studyItem->setExpanded(true);
  // find or create series level item
  QTreeWidgetItem *seriesItem = nullptr;
  for (int i = 0; i < studyItem->childCount(); ++i)
    if (studyItem->child(i)->text(QUERY_RESULTS_COLUMN_ID) == seriesInstanceUID.c_str() &&
        studyItem->child(i)->text(QUERY_RESULTS_COLUMN_DESCRIPTION) == seriesDescription.c_str())
      seriesItem = studyItem->child(i);
  if (!seriesItem) {
    seriesItem = new QTreeWidgetItem;
    seriesItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    seriesItem->setText(QUERY_RESULTS_COLUMN_ID,seriesInstanceUID.c_str());
    seriesItem->setText(QUERY_RESULTS_COLUMN_TYPE,"SERIES");
    seriesItem->setText(QUERY_RESULTS_COLUMN_DESCRIPTION,seriesDescription.c_str());
    studyItem->addChild(seriesItem);  // note: QTreeWidget takes ownership of item    
  }
  seriesItem->setExpanded(true);
}

void dicomQueryRetrieve::setStatus(std::string str) {
  ui->statusLabel->setText(tr("Status: ") + str.c_str());
}
 
// ---------------------------------------------------------------------
// private slots
// ---------------------------------------------------------------------

void dicomQueryRetrieve::on_cancelQueryButton_clicked() {
  ebLog eblog(Q_FUNC_INFO);
  cancelQuery = true;
}

void dicomQueryRetrieve::on_queryPACSButton_clicked() {
  ebLog eblog(Q_FUNC_INFO);

  // empty query dir
  QDir queryDir(cap::getImagesQueryDir());
  queryDir.setFilter(QDir::Files);
  QFileInfoList fileInfoList = queryDir.entryInfoList();
  for (int i = 0; i < fileInfoList.size(); ++i)
    queryDir.remove(fileInfoList.at(i).fileName());
  
  // initialize query (but not cummulative GUI and responses)
  queryResultCount = 0;
  cancelQuery = false;

  // build map of query key/values
  QueryMapType queryMap = getQueryMap();

  // query at study level to match patient/study criteria
  bool success = c_find_scu(queryMap);
  if (!success) {
    errorMessage(tr("Query failed"));
    return;
  }
  
  // query at series level using study instance UID and series criteria
  for (auto &study : studyResponses) {
    success = c_find_scu(queryMap,study.first);
    if (!success) {
      errorMessage(tr("Query failed"));
      return;
    }
    if (queryResultCount >= getQueryResultLimit())
      break;
  }

  // set last query label
  ui->lastQueriedLabel->setText(tr("Last Query: ") + QDateTime::currentDateTime().toString());
  setStatus("");
}

void dicomQueryRetrieve::on_clearQueryResultsButton_clicked() {
  ebLog eblog(Q_FUNC_INFO);

  // clear recorded responses
  studyResponses.clear();
  seriesResponses.clear();

  // clear GUI
  ui->queryResultsTreeWidget->clear();
  ui->lastQueriedLabel->setText(tr("Last Query: "));
  setStatus("");
}

void dicomQueryRetrieve::on_cancelRetrieveButton_clicked() {
  ebLog eblog(Q_FUNC_INFO);
  cancelRetrieve = true;
}

void dicomQueryRetrieve::on_retrieveSelectedResultsButton_clicked() {
  // initialize retrieve
  cancelRetrieve = false;
  
  // get selected query results
  {
    ebLog eblog(Q_FUNC_INFO);  // make sure log finishes before launching retrieve thread
    std::lock_guard<std::mutex> lock(retrieveQueueMutex);   
    for (auto item : ui->queryResultsTreeWidget->selectedItems()) {
      std::string uid = item->text(QUERY_RESULTS_COLUMN_ID).toStdString();
      std::cerr << "retrieving " << uid << std::endl;
      if (item->text(QUERY_RESULTS_COLUMN_TYPE) == "SERIES") {
        if (seriesResponses.find(uid) != seriesResponses.end()) {
          bool alreadyInQueue = false;
          for (auto &dataset : retrieveQueue) {
            OFString seriesInstanceUID;
            dataset.findAndGetOFString(DCM_SeriesInstanceUID,seriesInstanceUID);
            if (uid == seriesInstanceUID.c_str())
              alreadyInQueue = true;
          }
          if (!alreadyInQueue)
            retrieveQueue.push_back(seriesResponses.at(uid));
        } else
          errorMessage(tr("series result not found"));
      } else if (item->text(QUERY_RESULTS_COLUMN_TYPE) == "STUDY") {
        if (studyResponses.find(uid) != studyResponses.end()) {
          bool alreadyInQueue = false;
          for (auto &dataset : retrieveQueue) {
            OFString studyInstanceUID;
            dataset.findAndGetOFString(DCM_StudyInstanceUID,studyInstanceUID); 
            if (uid == studyInstanceUID.c_str())
              alreadyInQueue = true;
          }
          if (!alreadyInQueue)
            retrieveQueue.push_back(studyResponses.at(uid));
        } else
          errorMessage(tr("study result not found"));
      } else {
        errorMessage(tr("bad result type"));
      }
    }
  }

  if (!retrieveThreadRunning) {
    if (threadedRetrieve) {
      // spawn c_move_scu_store_scp() in background thread
      std::thread retrieveThread(&dicomQueryRetrieve::c_move_scu_store_scp,this);
      retrieveThread.detach();
    } else {
      // do c_move_scu_store_scp() in main thread
      c_move_scu_store_scp();
    }
  }
}

void dicomQueryRetrieve::queryCriterionTagChanged() {
  // get scroll area vertical layout
  QVBoxLayout *vlayout = dynamic_cast<QVBoxLayout*>(ui->queryCriteriaScrollArea->layout());
  ebAssert(vlayout);
  // find horizontal layout that contains combo box that was changed
  for (int i = 0; i < vlayout->count(); ++i) {
    QHBoxLayout *hlayout = dynamic_cast<QHBoxLayout*>(vlayout->itemAt(i));
    ebAssert(hlayout && sender());
    // tag combobox should be first widget
    if (sender() == hlayout->itemAt(0)->widget()) {
      QComboBox *tagComboBox   = dynamic_cast<QComboBox*>(hlayout->itemAt(0)->widget());
      QComboBox *matchComboBox = dynamic_cast<QComboBox*>(hlayout->itemAt(1)->widget());
      ebAssert(tagComboBox && matchComboBox);
      if ((tagComboBox->currentText() == tr("Patient Birth Date (YYYYMMDD)")) ||
          (tagComboBox->currentText() == tr("Study Date (YYYYMMDD)"))) {
        // temporal attribute
        matchComboBox->clear();
        matchComboBox->addItem(tr("on"));
        matchComboBox->addItem(tr("on or before"));
        matchComboBox->addItem(tr("on or after"));
        matchComboBox->addItem(tr("in range"));
      } else if (tagComboBox->currentText() == tr("Series Number")) {
        // other attribute
        matchComboBox->clear();
        matchComboBox->addItem(tr("="));
      } else {
        // string attribute
        matchComboBox->clear();
        matchComboBox->addItem(tr("is"));
        matchComboBox->addItem(tr("contains"));
        matchComboBox->addItem(tr("begins with"));
        matchComboBox->addItem(tr("ends with"));
      }
    }
  }
}

void dicomQueryRetrieve::queryCriterionMatchChanged() {
  // get scroll area vertical layout
  QVBoxLayout *vlayout = dynamic_cast<QVBoxLayout*>(ui->queryCriteriaScrollArea->layout());
  ebAssert(vlayout);
  // find horizontal layout that contains combo box that was changed
  for (int i = 0; i < vlayout->count(); ++i) {
    QHBoxLayout *hlayout = dynamic_cast<QHBoxLayout*>(vlayout->itemAt(i));
    ebAssert(hlayout && sender());
    // match combobox should be second widget
    if (sender() == hlayout->itemAt(1)->widget()) {
      QComboBox *matchComboBox  = dynamic_cast<QComboBox*>(hlayout->itemAt(1)->widget());
      QLabel    *toLabel        = dynamic_cast<QLabel   *>(hlayout->itemAt(3)->widget());
      QLineEdit *value2LineEdit = dynamic_cast<QLineEdit*>(hlayout->itemAt(4)->widget());
      ebAssert(matchComboBox && toLabel && value2LineEdit);
      if (matchComboBox->currentText() == tr("in range")) {
        toLabel->show();
        value2LineEdit->show();
      } else {
        toLabel->hide();
        value2LineEdit->hide();
      }
    }
  }
}

void dicomQueryRetrieve::addQueryCriterion() {
  // get scroll area vertical layout
  QVBoxLayout *vlayout = dynamic_cast<QVBoxLayout*>(ui->queryCriteriaScrollArea->layout());
  ebAssert(vlayout);
  // create new horizontal layout
  QHBoxLayout *hlayout = new QHBoxLayout;
  // tag combo box
  QComboBox *tagComboBox = new QComboBox;
  tagComboBox->addItem(tr("Patient Name")      ,"(0010,0010)");  // PN
  tagComboBox->addItem(tr("Patient ID")        ,"(0010,0020)");  // LO
  tagComboBox->addItem(tr("Patient Sex")       ,"(0010,0040)");  // CS
  tagComboBox->addItem(tr("Patient Birth Date (YYYYMMDD)"),"(0010,0030)");  // DA
  tagComboBox->addItem(tr("Study Date (YYYYMMDD)")        ,"(0008,0020)");  // DA
  tagComboBox->addItem(tr("Accession Number")  ,"(0008,0050)");  // SH
  tagComboBox->addItem(tr("Study ID")          ,"(0020,0010)");  // SH
  tagComboBox->addItem(tr("Study Description") ,"(0008,1030)");  // LO
  tagComboBox->addItem(tr("Series Number")     ,"(0020,0011)");  // IS
  tagComboBox->addItem(tr("Series Description"),"(0008,103e)");  // LO
  tagComboBox->addItem(tr("Modality")          ,"(0008,0060)");  // CS
  hlayout->addWidget(tagComboBox);  
  connect(tagComboBox,SIGNAL(currentIndexChanged(const QString &)),
          this,SLOT(queryCriterionTagChanged()));
  // match criterion combo box
  QComboBox *matchComboBox = new QComboBox;
  hlayout->addWidget(matchComboBox);
  connect(matchComboBox,SIGNAL(currentIndexChanged(const QString &)),
          this,SLOT(queryCriterionMatchChanged()));

  // String Attribute: PN, LO, CS, SH, IS
  matchComboBox->addItem(tr("is"));
  matchComboBox->addItem(tr("contains"));
  matchComboBox->addItem(tr("begins with"));
  matchComboBox->addItem(tr("ends with"));

  // Other Atrribute: SL, SS, UL, US, FL, FD, OB, OW, UN, AT, DS, IS, AS, UI
  //matchComboBox->addItem(tr("is"));

  // Temporal Attribute: DA, TM, DT
  //matchComboBox->addItem("on");
  //matchComboBox->addItem("on or before");
  //matchComboBox->addItem("on or after");
  //matchComboBox->addItem("in range");
  
  // value line edit (and hidden "to" label and value2 line edit)
  QLineEdit *valueLineEdit = new QLineEdit;
  hlayout->addWidget(valueLineEdit);
  QLabel *toLabel = new QLabel("to");
  hlayout->addWidget(toLabel);
  toLabel->hide();
  QLineEdit *value2LineEdit = new QLineEdit;
  hlayout->addWidget(value2LineEdit);
  value2LineEdit->hide();
  
  // remove criterion button (second to last widget)
  QPushButton *removeButton = new QPushButton("-");
  hlayout->addWidget(removeButton);
  connect(removeButton,SIGNAL(clicked(bool)),
          this,SLOT(removeQueryCriterion()));
  // add criterion button
  QPushButton *addButton = new QPushButton("+");
  hlayout->addWidget(addButton);
  connect(addButton,SIGNAL(clicked(bool)),
          this,SLOT(addQueryCriterion()));
  // add horizontal layout to vertical layout
  vlayout->addLayout(hlayout);
}
  
void dicomQueryRetrieve::removeQueryCriterion() {
  // get scroll area vertical layout
  QVBoxLayout *vlayout = dynamic_cast<QVBoxLayout*>(ui->queryCriteriaScrollArea->layout());
  ebAssert(vlayout);
  // find horizontal layout that contains remove button that was pressed
  for (int i = 0; i < vlayout->count(); ++i) {
    QHBoxLayout *hlayout = dynamic_cast<QHBoxLayout*>(vlayout->itemAt(i));
    ebAssert(hlayout);
    ebAssert(sender());
    // remove button should be second to last widget
    if (sender() == hlayout->itemAt(hlayout->count()-2)->widget()) {
      // clear hlayout
      QLayoutItem *child;
      while ((child = hlayout->takeAt(0)) != 0) {
        delete child->widget();
        delete child;
      }
      // remove hlayout from vlayout
      child = vlayout->takeAt(i);
      delete child;
      break;
    }
  }
  // make sure at least one criterion is left
  if (vlayout->count() == 0)
    addQueryCriterion();
}
