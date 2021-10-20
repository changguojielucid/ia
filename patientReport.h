// Copyright (c) Elucid Bioimaging

#ifndef PATIENTREPORT_H
#define PATIENTREPORT_H

#include <QWidget>
#include <QWebView>
#include <QNetworkRequest>
#include <QProgressDialog>
#include <QErrorMessage>

#include "patientAnalyze.h"

// forward declaration
class cap;

/****************************************************** REPORT PACKAGE ****************************************************************
 *

 *
 **************************************************************************************************************************************/


/**
 * @{ 
 ** 
 * @brief Report Package
 *
 * The report package comprises the following related classes:
 *    
 * 1.        workItemUpdate
 *                 |
 * 2.         patientReport
 *
 *  \copyright Elucid Bioimaging
 *  \ingroup patientReport
 */

/**
 * @brief class workItemUpdate implementing updates, which are placed on a list of updates for each workItemListEntry for reference during
 * the reporting activity.
 */
class workItemUpdate
{
public:
  QString performer, performDateTime, step, ID;
  stageParameters *parametersForStage; // payload used for computational updates

  workItemUpdate();
  workItemUpdate(const QString &stepStr, const QString &IDStr, stageParameters *stageParams, const QString &performerName, const QString &performDateTimeStr);

  void readWorkItemUpdate(const QJsonObject &json) { 
    //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
          performer = json[performer_token].toString(); 
          performDateTime = json[performDateTime_token].toString(); 
          step = json[step_token].toString(); 
    ID = json[updateID_token].toString(); 
    if (step.contains("targetDefine")) {
      QJsonObject settingsObject = json[processingParameters_token].toObject();
      parametersForStage = new stageParameters();
      parametersForStage->readStageParameters(settingsObject);
    }
  };

  void writeWorkItemUpdate(QJsonObject &json) const { 
    //ebLog eblog(Q_FUNC_INFO); eblog << step.toStdString() << std::endl;
          json[performer_token] = performer; 
          json[performDateTime_token] = performDateTime; 
          json[step_token] = step; 
    json[updateID_token] = ID;

    if (parametersForStage != NULL) {
      QJsonObject settingsObject;
      parametersForStage->writeStageParameters(settingsObject);
      json[processingParameters_token] = settingsObject;
    }
  };

  bool updateUpdate(workItemUpdate *oitem)
  {
    // qInfo() << "update key " << ID << " " << performDateTime;
    oitem->performer = performer;
    oitem->performDateTime = performDateTime;
    oitem->step = step;
    oitem->ID = ID;
    if (NULL != parametersForStage) {
        stageParameters *nstg = new stageParameters();
        parametersForStage->update( nstg );
    }
    return true;
  }
};
/** @} */

namespace Ui {
class patientReport;
}

/**
 * \ingroup patientReport
 * @{ 
 ** 
 * @brief class patientReport (in namepace Ui) (subclassed from QWidget): the main class with the list of definitions as a whole and the 
 * methods to access and interact with it. Supports fucntionality to transfer appropriate data and interact with server for reporting functions.
 */
class patientReport : public QWidget
{
  Q_OBJECT
  
  int sessionItemIndex;

public:
  explicit patientReport(QWidget *owner=0, QMenu *m=NULL, bool masterWithRespectToMenu=false);
  ~patientReport();
  void preloadReport(QString product, int index, QList<workItemUpdate> *updateList);
  void resetWI(QList<workItemUpdate> *updateList);
  void disconnectMenuActions();
  void connectMenuActions();
  void reconnectMenuActions();
  void disableMenuActions();
  void enableMenuActions();
  
public slots:
  void on_backToAnalyzeButton_clicked();

signals:
  void logUpdate(QString step, QString ID, stageParameters *stageParams, int sessionItemIndex);

private:
  Ui::patientReport *ui;
  cap *owner;
  QMenu *patientReportMenu;
  QString thisProduct;
  QList<workItemUpdate> *updates;
  QNetworkAccessManager XDSGetReportRAD_68;
  QFile reportOutput;
  QNetworkReply *XDSGetReportRAD_68Reply;
  QProgressDialog *downloadProgressIndicator;
  QString workItemID;
  QAction *gotoPatientReportAction;
  QErrorMessage *message;
  std::map<QAction *, bool> menuActionEnabledMap;
  QProgressDialog *progressIndicator;

private slots:
  void ensureOnPatientReportPage();
  void acceptScreenControl(QString ID, QUrl url, QProgressDialog *generateProgressIndicator);
  void downloadReport(QNetworkRequest request);
  void parseXDSGetReportRAD_68Reply();
};
/** @} */

#endif // PATIENTREPORT_H
