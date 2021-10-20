// Copyright (c) Elucid Bioimaging

#ifndef PATIENTANALYZE_H
#define PATIENTANALYZE_H

#include "seriesSurvey.h"
#include "targetDefine.h"
#include <QWidget>
#include <QStatusBar>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QRadioButton>

// forward declarations
class cap;

/**
 * @{ 
 ** 
 * @brief Patient AnalyzePackage
 *
 * The analyze package comprises the following related classes:
 *    
 * 1.        readingsSelector
 *                 |
 * 2.         patientAnalyze
 *
 *  \copyright Elucid Bioimaging
 *  \ingroup patientAnalyze
 */

/**
 * @brief class readingsSelector provides functionality for user selection and specification for readings asociated with analysis of targets 
 */
class readingsSelector : public QWidget
{
  Q_OBJECT

public:
  explicit readingsSelector(QWidget *owner, QString m, vtkSmartPointer<ebvLinkedViewers2> v, ebID c);
  ~readingsSelector() { ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; delete Dialog;  }
  void setTargetId(ebID t);
  void presentDialog();

private:
  QString measurandCategory;
  QList<QString> availableMeasurands;
  QList<QString> availableScaleBases;
  QDialog *Dialog;
  QGridLayout *gridLayout_3;
  QGroupBox *measurandsBox;
  QVBoxLayout *measurands;
  QGroupBox *scaleBaseBox;
  QVBoxLayout *scaleBases;
  QDialogButtonBox *buttonBox;
  vtkSmartPointer<ebvLinkedViewers2> viewers;
  ebID chartId, targetId;

private slots:
  void acceptReadingsSelection();

public slots:
  void cancelReadingsSelection();
};
/** @} */

namespace Ui {
class patientAnalyze;
}

typedef std::pair<ebiMultiImageReader*,ebID> MultiReaderIDPair;

/**
 * \ingroup patientAnalyze
 * @{ 
 ** 
 * @brief class patientAnalyze (in namepace Ui) (subclassed from QWidget): the main class with the list of definitions as a whole and the 
 * methods to access and interact with it. Manages all computation and display aspects analyses comprising multiple targets and multiple 
 * image series for a given workItem session.  
 */
class patientAnalyze : public QWidget
{
  Q_OBJECT

public:
  explicit patientAnalyze(QWidget *owner=0, QMenu *m=NULL, bool masterWithRespectToMenu=false);
  ~patientAnalyze();
  cap *owner;
  void preloadAnalyze(QString product, int index, ebiVesselPipeline::Pointer p, QList<imageSeries> *imageSeriesSet, QList<targetDef> *targetDefs);
  void resetWI(ebiVesselPipeline::Pointer p, QList<imageSeries> *imageSeriesSet, QList<targetDef> *targetDefs);
  void saveToReport(ebvViewer::ViewerType viewerType, bool forceSliceViewerForTarget, bool includeReportUpdate);
  void toggleOblique(bool checked);
  void toggleCorner(bool checked);
  void toggleColorBar(bool checked);
  bool jumpTo(QString tag);
  void giveToDefineIfNotAlreadyThere();
  void removeAllTargetDisplays();
  void displayTargets();
  void markLesion();
  void clearLesions();
  bool switchTargetPath();
  bool markingLesion() { return markingLesionFlag; }
  void setMarkingLesionFlag(bool flag) { markingLesionFlag = flag; }
  void labelMarkedLesion();
  void toggleSegmentation(QString segName, bool checked);
  void establishTools(QString product, int index, bool masterWithRespectToMenu);
  vtkSmartPointer<ebvLinkedViewers2> viewers;
  void disconnectMenuActions();
  void connectMenuActions();
  void disableMenuActions();
  void enableMenuActions();
  void dismissCompositionControls();
  bool newTargetReadingsFlag;
  bool newLesionReadingsFlag;
  void displayTargetsAndAnticipateReportContent();

public slots:
  void setCurrentTarget(targetDef *def);
  void on_proceedToReportButton_clicked();
  void on_backToDefineButton_clicked();
  void processCompositionSettingsChange();

private:
  QString thisProduct;
  Ui::patientAnalyze *ui;
  QMenu *patientAnalyzeMenu;
  imageSeries *currentBackingSeries; // various methods need to know what series to effect
  ebID backingImageId; 
  targetDef *currentTarget; // various methods need to know what target to effect
  ebID currentTargetId; 
  QErrorMessage *message;
  int sessionItemIndex;
  ebiVesselPipeline::Pointer pipeline;
  QList<imageSeries> *images; // actual list is declared as a member of workItemListEntry, this is a pointer to it
  QList<targetDef> *targets; // actual list is declared as a member of workItemListEntry, this is a pointer to it
  ebID id3d, ebvIdSummaryAreaGraph1, ebvIdSummaryAreaGraph2,
        ebvIdSliceAreaRender0, ebvIdSliceAreaRender1, ebvIdSliceAreaRender2; // viewer identifiers on the patientAnalyze screen itself
  ebvViewer::ViewerType sliceAreaRender0Type, sliceAreaRender1Type, sliceAreaRender2Type;
  capTools *summaryTools;
  capTools *sliceTools;
  readingsSelector *structureSel;
  readingsSelector *compositionSel;
  bool markingLesionFlag;
  std::set< MultiReaderIDPair > added_multireader_images;
  QAction *gotoPatientAnalyzeAction;
  std::map<QAction *, bool> *menuActionEnabledMap;
  bool eventFilter(QObject *obj, QEvent *event);
  void enableControlsAsAppropriateWhenNoTargetFocus();
  void enableControlsAsAppropriateGivenATargetFocus();
  void addViewersLegend();						     
  bool updateServer();

private slots:
  void ensureOnPatientAnalyzePage();
  void acceptScreenControl(QStackedWidget *seriesSelectionArea, imageSeries *series);
  void resetBackingSeries(imageSeries *series);

signals:
  void logUpdate(QString step, QString ID, stageParameters *stageParams, int sessionItemIndex);
  void giveScreenControlToDefineFromAnalyze(QStackedWidget *seriesSelectionArea, imageSeries *series, targetDef *def);
  void giveScreenControlToReport(QUrl url);
  void packageDataAndTransitionToReport();
};
/** @} */

#endif // PATIENTANALYZE_H
