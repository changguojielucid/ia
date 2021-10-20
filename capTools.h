// Copyright (c) Elucid Bioimaging

#ifndef CAPTOOLS_H
#define CAPTOOLS_H

#include "ebLog.h"
#include "ebvLinkedViewers2.h"
#include <vtkSmartPointer.h>
#include <QToolButton>
#include <QTableWidget>
#include <QDesktopWidget>
#include <QLabel>
#include <QGroupBox>
#include <QAction>
#include <QMenu>
#include <QErrorMessage>
//#include <string.h>
#include <iostream>

// forward declarations
class cap;

#define HIDECONTROL(toolButton, menuAction) toolButton->setVisible(false); menuAction->setVisible(false)
#define ENABLECONTROL(toolButton, menuAction, toolTipText) toolButton->setEnabled(true); menuAction->setEnabled(true); (*menuActionEnabledMap)[menuAction] = true; toolButton->setToolTip(toolTipText)
#define DISABLECONTROL(toolButton, menuAction, toolTipText) toolButton->setDisabled(true); menuAction->setDisabled(true); (*menuActionEnabledMap)[menuAction] = false; toolButton->setToolTip(toolTipText)

/**
 * @{ 
 ** 
 * @brief class capTools provides common basis for tool bars as exist across modules.
 *
 *  \copyright Elucid Bioimaging
 *  \ingroup cap
 */
class capTools : public QWidget
{
  Q_OBJECT

public:
  QToolButton *ToggleLumenTool; QAction *ToggleLumenAction; 
  QToolButton *ToggleWallTool; QAction *ToggleWallAction; 
  QToolButton *MaxStenosisTool; QAction *MaxStenosisAction; 
  QToolButton *MaxDilationTool; QAction *MaxDilationAction; 
  QToolButton *MaxRRTool; QAction *MaxRRAction; 
  QToolButton *MaxWTTool; QAction *MaxWTAction; 
  QToolButton *ToggleCornerTool; QAction *ToggleCornerAction; 
  QToolButton *OptimizeSubvolumeTool; QAction *OptimizeSubvolumeAction; 
  QToolButton *ShapeIntensityTool; QAction *ShapeIntensityAction; 
  QToolButton *SaveViewSettingsTool; QAction *SaveViewSettingsAction; 
  QToolButton *RestoreViewSettingsTool; QAction *RestoreViewSettingsAction; 
  QToolButton *AnnotateTool; QAction *AnnotateAction; 
  QToolButton *MeasureTool; QAction *MeasureAction;
  QToolButton *MaxCalcTool; QAction *MaxCalcAction;
  QToolButton *MaxLRNCTool; QAction *MaxLRNCAction;
  QToolButton *MaxMATXTool; QAction *MaxMATXAction;
  QToolButton *MaxIPHTool; QAction *MaxIPHAction;
  QToolButton *MaxUlcTool; QAction *MaxUlcAction;
  QToolButton *MaxThrTool; QAction *MaxThrAction;
  QToolButton *TargetPathSwitchTool; QAction *TargetPathSwitchAction;
  QToolButton *TargetCreateTool; QAction *TargetCreateAction;
  QToolButton *TargetDeleteTool; QAction *TargetDeleteAction;
  QToolButton *TargetModifyTool; QAction *TargetModifyAction;
  QToolButton *ToggleObliqueTool; QAction *ToggleObliqueAction;
  QToolButton *VolumeRenderTool; QAction *VolumeRenderAction;
  QToolButton *ToggleCoronalTool; QAction *ToggleCoronalAction;
  QToolButton *ToggleSagittalTool; QAction *ToggleSagittalAction;
  QToolButton *ToggleAxialTool; QAction *ToggleAxialAction;
  QToolButton *ShowAsMovieTool; QAction *ShowAsMovieAction;
  QToolButton *ToggleColorBarTool; QAction *ToggleColorBarAction;
  QToolButton *CenterAtCursorTool; QAction *CenterAtCursorAction;
  QToolButton *MoveProximalTool; QAction *MoveProximalAction;
  QToolButton *MoveDistalTool; QAction *MoveDistalAction;
  QToolButton *SaveToReportTool; QAction *SaveToReportAction;
  QString editSegmentationActionLabel;
  bool editSegmentationEnableState;
  QString markLesionActionLabel;
  bool markLesionEnableState;
  QString clearLesionsActionLabel;
  bool clearLesionsEnableState;
  capTools(QWidget *p, 
           QString product, 
           int index, 
           vtkSmartPointer<ebvLinkedViewers2> v, 
           ebID vId, 
           ebvViewer::ViewerType vType, 
           QWidget *controlsBox, 
           QMenu *m, 
           bool masterWithRespectToMenu, 
           QString setName,
           std::map<QAction *, bool> *mMap);
  ~capTools();
  void disableToolButtons();
  void enableToolButtons();
  void disconnectMenuActions();
  void connectMenuActions();
  void setBackingImageId(ebID id) { backingImageId = id; }
  void resetViewer(ebID vId, ebvViewer::ViewerType vType) { viewerId = vId; viewerType = vType; }

private:
  cap *owner;
  QMenu *sisterMenu;
  QString thisProduct;
  int sessionItemIndex;
  ebID backingImageId; 
  vtkSmartPointer<ebvLinkedViewers2> viewers;
  ebID viewerId; // viewer identifier on which this instance of capTools has been constructed
  ebvViewer::ViewerType viewerType; // // viewer type on which this instance of capTools has been constructed
  QErrorMessage *error;
  ebvLinkedViewers2::PointType cursorPosition;
  ebvViewer::CameraParams cameraParams;
  double window, level;
  bool viewSettingsSaved;
  std::map<QAction *, bool> *menuActionEnabledMap;

public slots:
  void ToggleLumen(bool checked); void ToggleLumenNoArg() { ToggleLumenTool->setChecked(!ToggleLumenTool->isChecked()); ToggleLumen(ToggleLumenTool->isChecked()); }
  void ToggleWall(bool checked); void ToggleWallNoArg() { ToggleWallTool->setChecked(!ToggleWallTool->isChecked()); ToggleWall(ToggleWallTool->isChecked()); }
  void MaxStenosis();
  void MaxDilation();
  void MaxRR();
  void MaxWT();
  void ToggleCorner(bool checked); void ToggleCornerNoArg() { ToggleCornerTool->setChecked(!ToggleCornerTool->isChecked()); ToggleCorner(ToggleCornerTool->isChecked()); }
  void OptimizeSubvolume(bool checked); void OptimizeSubvolumeNoArg() { OptimizeSubvolumeTool->setChecked(!OptimizeSubvolumeTool->isChecked()); OptimizeSubvolume(OptimizeSubvolumeTool->isChecked()); }
  void ShapeIntensity();
  void SaveViewSettings(); 
  void RestoreViewSettings(); 
  void Annotate(bool checked); void AnnotateNoArg() { AnnotateTool->setChecked(!AnnotateTool->isChecked()); Annotate(AnnotateTool->isChecked()); }
  void AnnotateContextMenu(QPoint pos);
  void anAction();
  void anotherAction();
  void Measure(bool checked); void MeasureNoArg() { MeasureTool->setChecked(!MeasureTool->isChecked()); Measure(MeasureTool->isChecked()); }
  void MaxCalc();
  void MaxLRNC();
  void MaxMATX();
  void MaxIPH();
  void MaxUlc();
  void MaxThr();
  void TargetPathSwitch();
  void TargetCreate(bool checked); void TargetCreateNoArg() { TargetCreateTool->setChecked(!TargetCreateTool->isChecked()); TargetCreate(TargetCreateTool->isChecked()); }
  void TargetCreateWithSpec();
  void TargetDelete();
  void TargetModify();
  void AddDistalVessel();
  void MarkLesion();
  void ResetWall();
  void ClearLesions();
  void ResetTarget();
  void EvaluateLumen();
  void EvaluateWall();
  void EditSegmentation();
  void ToggleOblique(bool checked); void ToggleObliqueNoArg() { ToggleObliqueTool->setChecked(!ToggleObliqueTool->isChecked()); ToggleOblique(ToggleObliqueTool->isChecked()); }
  void VolumeRender(bool checked); void VolumeRenderNoArg() { VolumeRenderTool->setChecked(!VolumeRenderTool->isChecked()); VolumeRender(VolumeRenderTool->isChecked()); }
  void ToggleCoronal(bool checked); void ToggleCoronalNoArg() { ToggleCoronalTool->setChecked(!ToggleCoronalTool->isChecked()); ToggleCoronal(ToggleCoronalTool->isChecked()); }
  void ToggleSagittal(bool checked); void ToggleSagittalNoArg() { ToggleSagittalTool->setChecked(!ToggleSagittalTool->isChecked()); ToggleSagittal(ToggleSagittalTool->isChecked()); }
  void ToggleAxial(bool checked); void ToggleAxialNoArg() { ToggleAxialTool->setChecked(!ToggleAxialTool->isChecked()); ToggleAxial(ToggleAxialTool->isChecked()); }
  void ShowAsMovie();
  void ToggleColorBar(bool checked); void ToggleColorBarNoArg() { ToggleColorBarTool->setChecked(!ToggleColorBarTool->isChecked()); ToggleColorBar(ToggleColorBarTool->isChecked()); }
  void CenterAtCursor();
  void MoveProximal();
  void MoveDistal();  
  void SaveToReport();
};
/** @} */

#endif // CAPTOOLS_H
