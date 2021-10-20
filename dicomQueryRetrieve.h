// Copyright (c) Elucid Bioimaging
#ifndef DICOMQUERYRETRIEVE_H
#define DICOMQUERYRETRIEVE_H

#include "ebLog.h"
#include "ImageDatabase.h"
#include <dcmtk/dcmnet/dimse.h>
#include <QDialog>
#include <QTreeWidget>
#include <string>
#include <map>
#include <atomic>
#include <mutex>

#define QUERY_RESULTS_COLUMN_DESCRIPTION   0
#define QUERY_RESULTS_COLUMN_TYPE          1  // hidden ("PATIENT","STUDY","SERIES")
#define QUERY_RESULTS_COLUMN_ID            2  // hidden (Patient ID, Study Instance UID, Series Instance UID)
#define QUERY_RESULTS_COLUMN_NUMBER_IMAGES 3

// forward declarations
class workItem;
class DcmTLSTransportLayer;

namespace Ui {
  class dicomQueryRetrieve;
}

/** \brief PACS query/retrieve
 *
 *  Performs DICOM Query/Retrieve operation using C-ECHO, C-FIND, C-MOVE, and C-STORE.  The query is done
 *  in two stages, first query at the study level to find study instance UIDs according to search criteria.
 *  Then, the second query is at the series level to find series instances.  Note that C-GET is not used.
 *  The code is based on DCMTK tools.
 *
 *  **DICOM Ports**:
 *  - 1024: standard privileged DICOM port
 *  - 11112: standard insecure non-privilged DICOM port
 *  - 2761: ISCL secure DICOM port
 *  - 2762: TLS secure DICOM port
 *
 *  **THREADING**: The query operation is not threaded since it should generally be fast, although the ability
 *  to cancel is given in case a user receives too many results.  The retrieve operation occurs in a background
 *  thread to not block the GUI.  Import happens in that background thread once retrieve is finished.
 *  The import operation of ImageDatabase must be thread-safe.  The background thread may update the 
 *  status label so access to the status label is locked with a mutex.
 *
 *  **TODO**
 *  - making sure that re-retrieving a series doesn't cause redundancy
 *  - handle case of importing already existing series
 *  - way to detect if local AE title doesn't match location in Horos (for example)
 *
 *  \copyright Elucid Bioimaging
 *  \ingroup workItem
 **/

class dicomQueryRetrieve : public QDialog
{
  Q_OBJECT
public:
  //! constructor
  explicit dicomQueryRetrieve(QWidget *parent = 0);

  //! destructor
  virtual ~dicomQueryRetrieve();

  //! function to emit signals (useful for calling from other threads)
  void emitErrorMessageSignal(QString str) { emit errorMessageSignal(str); }

  //! function to emit signals (useful for calling from other threads)
  void emitSetStatusSignal(std::string str) { emit setStatusSignal(str); }
                                                
public slots:
  //! initialize DICOM network parameters
  void init(std::string _serverAETitle, std::string _serverHost, unsigned short _serverPort,
            std::string _localAETitle, unsigned short _localPort, bool _secure);

  //! raise window to top
  void showAndRaise();

signals:
  void errorMessageSignal(QString str1, QString str2="");
  void setStatusSignal(std::string str);
  void importImagesSignal(QString path, bool recursive, bool requireOriginal, bool excludeLocalizer, bool excludePreContrast, bool showProgress);
  
protected:
  //! dcmtk network
  struct Network {
    Network(dicomQueryRetrieve *qr, T_ASC_NetworkRole role, int acceptorPort, bool secure);
    virtual ~Network();
    dicomQueryRetrieve   *queryRetrieve;  //!< owning dicomQueryRetrieve instance
    T_ASC_Network        *net;            //!< network
    DcmTLSTransportLayer *tLayer;         //!< TLS layer
    OFCondition           cond;           //!< condition of the network
  };

  //! dcmtk association
  struct Association {
    Association(dicomQueryRetrieve *qr,
                std::string _serverAETitle, std::string _serverHost, unsigned short _serverPort,                
                std::string _localAETitle, std::string _localHost);
    virtual ~Association();
    void addPresentationContext(T_ASC_PresentationContextID presentationContextID, const char* abstractSyntax,
                                const char* transferSyntaxList[], int transferSyntaxListCount,
                                T_ASC_SC_ROLE proposedRole=ASC_SC_ROLE_DEFAULT);
    void requestAssociation(Network &network);
    T_ASC_PresentationContextID findAcceptedPresentationContextID(const char *abstractSyntax) const;
    dicomQueryRetrieve *queryRetrieve;  //!< owning dicomQueryRetrieve instance
    std::string         localAETitle,localHost,serverAETitle,serverHost;
    unsigned short      serverPort;
    T_ASC_Parameters   *params;         //!< association parameters
    T_ASC_Association  *assoc;          //!< association
    OFCondition         cond;           //!< condition of the association
  };

  //! callback data for C-FIND SCU
  struct FindSCUCallbackData {
    dicomQueryRetrieve         *queryRetrieve;  //!< owning dicomQueryRetrieve instance
    bool                        studyLevel;     //!< study level or series level query
    T_ASC_Association          *assoc;          //!< association
    T_ASC_PresentationContextID presId;         //!< presentation context ID
  };
  
  //! callback data for C-MOVE SCU
  struct MoveSCUCallbackData {
    dicomQueryRetrieve         *queryRetrieve;  //!< owning dicomQueryRetrieve instance
    T_ASC_Association          *assoc;          //!< association
    T_ASC_PresentationContextID presId;         //!< presentation context ID
  };

  //! callback data for C-STORE SCP
  struct StoreSCPCallbackData {
    dicomQueryRetrieve *queryRetrieve;   //!< owning dicomQueryRetrieve instance
    std::string         dirName;         //!< name of directory to store file
    std::string         imageFileName;   //!< name of file to be stored
    DcmFileFormat      *dcmff;           //!< DICOM file to be stored
    T_ASC_Association  *assoc;           //!< association
  };

  //! callback data for combined C-ECHO SCP / C-STORE SCP for retrieve
  struct SubOpCallbackData {
    dicomQueryRetrieve   *queryRetrieve;  //!< owning dicomQueryRetrieve instance
    std::set<std::string> dirNames;       //!< name of dirs to be imported
  };
    
  // typedefs for clarity
  typedef std::map<std::string,std::string> QueryMapType;  //!< maps DICOM tag to query value
  typedef std::string StudyInstanceUIDType;                //!< study instance UID
  typedef std::string SeriesInstanceUIDType;               //!< series instance UID

  workItem *owner;  //!< owning workItem instance
  
  // network variables
  std::string    serverAETitle;  //!< PACS AE title
  std::string    serverHost;     //!< PACS hostname/IP address
  unsigned short serverPort;     //!< PACS port number
  std::string    localAETitle;   //!< our AE title (hostname)
  std::string    localHost;      //!< our hostname
  unsigned short localPort;      //!< for receiving store requests
  bool           secure;         //!< use TLS secure transport layer
  
  // current query variables
  unsigned int queryResultCount;  //!< number of results from current query
  bool         cancelQuery;       //!< indicator to cancel current query

  // accumulated query result variables
  std::map<StudyInstanceUIDType ,DcmDataset> studyResponses;   //!< study level query responses
  std::map<SeriesInstanceUIDType,DcmDataset> seriesResponses;  //!< series level query responses

  // retrieve variables
  bool                   threadedRetrieve;       //!< whether or not to use background thread for retrieve
  std::atomic<bool>      retrieveThreadRunning;  //!< true if retrieve thread is currently running in background
  std::atomic<bool>      cancelRetrieve;         //!< indicator to cancel all outstanding retrieves
  std::mutex             retrieveQueueMutex;     //!< to lock retrieveQueue access from different threads
  std::deque<DcmDataset> retrieveQueue;          //!< queue of query responses to retrieve
  
  //! print out an error string to std::cerr(may run in background thread)
  static void error(std::string str);

  //! create a QMessageBox to present to user
  static void errorMessage(QString str);

  //! called per-result (study or series); optionally save response file; cancel/abort if user cancels or result limit reached
  static void findSCUCallback(void *callbackData, T_DIMSE_C_FindRQ *request, int responseCount, T_DIMSE_C_FindRSP *rsp, DcmDataset *responseIdentifiers);

  //! check if user canceled retrieve (runs in background thread)
  static void moveSCUCallback(void *callbackData, T_DIMSE_C_MoveRQ *request, int responseCount, T_DIMSE_C_MoveRSP *rsp);

  //! called per-file; if StoreEnd, create file name and save file (runs in background thread)
  static void storeSCPCallback(void *callbackData, T_DIMSE_StoreProgress *progress, T_DIMSE_C_StoreRQ *req,
                               char *imageFileName, DcmDataset **imageDataSet, T_DIMSE_C_StoreRSP *rsp, DcmDataset **statusDetail);

  //! called per-file; process C-STORE-RQ (create dicom file format, set source application AE title in metaheader) (runs in background thread)
  static OFCondition storeSCP(T_ASC_Association *assoc, T_DIMSE_Message *msg, T_ASC_PresentationContextID presID, StoreSCPCallbackData &storeSCPCallbackData);

  //! negotiation sub-association, receive/process C-ECHO-RQ or C-STORE-RQ (call storeSCP) (runs in background thread)
  static void subOpCallback(void *callbackData, T_ASC_Network *aNet, T_ASC_Association **subAssoc);
    
  //! DICOM C-ECHO SCU, return success
  bool c_echo_scu();

  //! DICOM C-FIND SCU (study level if studyInstanceUID empty, series level otherwise), return success
  bool c_find_scu(const QueryMapType &queryMap, std::string studyInstanceUID="");

  //! Combined DICOM C-MOVE SCU and C-STORE SCP (calls moveSCUCallback and subOpCallback) (runs in background thread)
  void c_move_scu_store_scp();
  
  //! get query result limit from UI
  unsigned int getQueryResultLimit() const;
  
  //! get DICOM dataset for query
  QueryMapType getQueryMap() const;

  //! add query result
  void addQueryResult(std::string patientID,         std::string patientName,
                      std::string studyDescription,  std::string studyInstanceUID,
                      std::string seriesDescription, std::string seriesInstanceUID);

private:
  Ui::dicomQueryRetrieve *ui;

private slots:
  //! thread-safe method to set text following "Status: " in status label
  void setStatus(std::string str);
  
  //! cancel DICOM query
  void on_cancelQueryButton_clicked();

  //! DICOM query using C-FIND (builds queryMap)
  void on_queryPACSButton_clicked();

  //! clear DICOM query results
  void on_clearQueryResultsButton_clicked();

  //! cancel DICOM retrieve
  void on_cancelRetrieveButton_clicked();

  //! DICOM retrieve using C-STORE and C-MOVE (get selected results and spawn c_move_scu thread)
  void on_retrieveSelectedResultsButton_clicked();

  //! query criterion tag combobox changed
  void queryCriterionTagChanged();

  //! query criterion match combobox changed
  void queryCriterionMatchChanged();
  
  //! add query criterion
  void addQueryCriterion();

  //! remove query criterion
  void removeQueryCriterion();
};


#endif // DICOMQUERYRETRIEVE_H
