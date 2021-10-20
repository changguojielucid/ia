// Copyright (c) Elucid Bioimaging

#include <QToolButton>
#include <QLabel>
#include <QGroupBox>
#include <QErrorMessage>
#include <QDebug>
#include <QMessageBox>

#include <string.h>
#include <iostream>

#include "ebiHelper.h"
#include "ebvLinkedViewers2.h"
#include "ebvViewerSpatial.h"

#include "capTools.h"
#include "cap.h"

/**
 * \ingroup cap
 * @{
 *
 * See cap.h for description of the CAP package purpose and contents.  This file has member functions for classes in the package.
 */

/**
 * @page capTools member functions
 */
capTools::capTools(QWidget *p, 
                   QString product, 
                   int index, 
                   vtkSmartPointer<ebvLinkedViewers2> v, 
                   ebID vId, 
                   ebvViewer::ViewerType vType, 
                   QWidget *controlsBox, 
                   QMenu *m, 
                   bool masterWithRespectToMenu, 
                   QString setName,
                   std::map<QAction *, bool> *mMap) 
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner = dynamic_cast<cap *>(p);
  sisterMenu = m;
  thisProduct = product;
  sessionItemIndex = index;
  viewers = v;
  viewerId = vId;
  viewerType = vType;
  backingImageId = 0;
  menuActionEnabledMap = mMap;
  viewSettingsSaved = false;
  editSegmentationActionLabel = tr("Edit Segmentation");
  editSegmentationEnableState = false;
  markLesionActionLabel = tr("Mark Lesion");
  markLesionEnableState = false;
  clearLesionsActionLabel = tr("Clear Lesions");
  clearLesionsEnableState = false;

  if (masterWithRespectToMenu)
    sisterMenu->addSeparator();

  SaveToReportTool = new QToolButton(controlsBox);
  SaveToReportTool->setObjectName(QStringLiteral("SaveToReportTool"));
  SaveToReportTool->setMinimumSize(QSize(48, 48));
  SaveToReportTool->setMaximumSize(QSize(48, 48));
  QIcon iconSaveToReport;
  iconSaveToReport.addFile(QStringLiteral(":/cap/toolPixMaps/SaveToReport.png"), QSize(), QIcon::Normal, QIcon::On);
  SaveToReportTool->setIcon(iconSaveToReport);
  SaveToReportTool->setIconSize(QSize(48, 48));
  SaveToReportTool->setCheckable(false);
  if (setName.contains("SLICE"))
    SaveToReportTool->setToolTip(tr("Save Slice as Key Image to Report"));
  else if (setName.contains("SUMMARY"))
    SaveToReportTool->setToolTip(tr("Save 3D as Key Image to Report"));
  else
    SaveToReportTool->setToolTip(tr("Save Key Image to Report"));
  SaveToReportTool->setWhatsThis(tr("SaveToReport allows the user to capture a key image to report"));
  SaveToReportTool->setText(tr("SaveToReport"));
  connect(SaveToReportTool, SIGNAL(clicked()), this, SLOT(SaveToReport()));
  if (masterWithRespectToMenu) {
    SaveToReportAction = new QAction(SaveToReportTool->toolTip(), owner);
    QString objName = setName;
    SaveToReportAction->setObjectName(objName.append(SaveToReportTool->toolTip()));
    sisterMenu->addAction(SaveToReportAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions()) {
      if (a->text() == SaveToReportTool->toolTip())
        if (a->objectName().contains(setName))
          SaveToReportAction = a;
    }
  }

  CenterAtCursorTool = new QToolButton(controlsBox);
  CenterAtCursorTool->setObjectName(QStringLiteral("CenterAtCursorTool"));
  CenterAtCursorTool->setMinimumSize(QSize(48, 48));
  CenterAtCursorTool->setMaximumSize(QSize(48, 48));
  QIcon iconCenterAtCursor;
  iconCenterAtCursor.addFile(QStringLiteral(":/cap/toolPixMaps/CenterAtCursor.png"), QSize(), QIcon::Normal, QIcon::On);
  CenterAtCursorTool->setIcon(iconCenterAtCursor);
  CenterAtCursorTool->setIconSize(QSize(48, 48));
  CenterAtCursorTool->setCheckable(false);
  CenterAtCursorTool->setToolTip(tr("Center View at Cursor"));
  CenterAtCursorTool->setWhatsThis(tr("CenterAtCursor allows the user to center the viewers at the cursor position"));
  CenterAtCursorTool->setText(tr("CenterAtCursor"));
  connect(CenterAtCursorTool, SIGNAL(clicked()), this, SLOT(CenterAtCursor()));
  if (masterWithRespectToMenu) {
    CenterAtCursorAction = new QAction(CenterAtCursorTool->toolTip(), owner);
    QString objName = setName;
    CenterAtCursorAction->setObjectName(objName.append(CenterAtCursorTool->toolTip()));
    sisterMenu->addAction(CenterAtCursorAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == CenterAtCursorTool->toolTip())
        if (a->objectName().contains(setName))
          CenterAtCursorAction = a;
  }

  ToggleObliqueTool = new QToolButton(controlsBox);
  ToggleObliqueTool->setObjectName(QStringLiteral("ToggleObliqueTool"));
  ToggleObliqueTool->setMinimumSize(QSize(48, 48));
  ToggleObliqueTool->setMaximumSize(QSize(48, 48));
  QIcon iconToggleOblique;
  iconToggleOblique.addFile(QStringLiteral(":/cap/toolPixMaps/ToggleOblique.png"), QSize(), QIcon::Normal, QIcon::On);
  ToggleObliqueTool->setIcon(iconToggleOblique);
  ToggleObliqueTool->setIconSize(QSize(48, 48));
  ToggleObliqueTool->setCheckable(true);
  ToggleObliqueTool->setChecked(false);
  ToggleObliqueTool->setToolTip(tr("Toggle Axis-aligned vs. Oblique Display"));
  ToggleObliqueTool->setWhatsThis(tr("ToggleOblique allows the user to switch view scrolling to reformatted oblique (or axis aligned) perspectives"));
  ToggleObliqueTool->setText(tr("ToggleOblique"));
  connect(ToggleObliqueTool, SIGNAL(clicked(bool)), this, SLOT(ToggleOblique(bool)));
  if (masterWithRespectToMenu) {
    ToggleObliqueAction = new QAction(ToggleObliqueTool->toolTip(), owner);
    QString objName = setName;
    ToggleObliqueAction->setObjectName(objName.append(ToggleObliqueTool->toolTip()));
    sisterMenu->addAction(ToggleObliqueAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == ToggleObliqueTool->toolTip())
        if (a->objectName().contains(setName))
          ToggleObliqueAction = a;
  }

  SaveViewSettingsTool = new QToolButton(controlsBox);
  SaveViewSettingsTool->setObjectName(QStringLiteral("SaveViewSettingsTool"));
  SaveViewSettingsTool->setMinimumSize(QSize(48, 48));
  SaveViewSettingsTool->setMaximumSize(QSize(48, 48));
  QIcon iconSaveViewSettings;
  iconSaveViewSettings.addFile(QStringLiteral(":/cap/toolPixMaps/SaveViewSettings.png"), QSize(), QIcon::Normal, QIcon::On);
  SaveViewSettingsTool->setIcon(iconSaveViewSettings);
  SaveViewSettingsTool->setIconSize(QSize(48, 48));
  SaveViewSettingsTool->setCheckable(false);
  SaveViewSettingsTool->setToolTip(tr("Save View Settings"));
  SaveViewSettingsTool->setWhatsThis(tr("SaveViewSettings allows the user to preserve the viewer settings for later recall"));
  SaveViewSettingsTool->setText(tr("SaveViewSettings"));
  connect(SaveViewSettingsTool, SIGNAL(clicked()), this, SLOT(SaveViewSettings()));
  if (masterWithRespectToMenu) {
    SaveViewSettingsAction = new QAction(SaveViewSettingsTool->toolTip(), owner);
    QString objName = setName;
    SaveViewSettingsAction->setObjectName(objName.append(SaveViewSettingsTool->toolTip()));
    sisterMenu->addAction(SaveViewSettingsAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == SaveViewSettingsTool->toolTip())
        if (a->objectName().contains(setName))
          SaveViewSettingsAction = a;
  }

  RestoreViewSettingsTool = new QToolButton(controlsBox);
  RestoreViewSettingsTool->setObjectName(QStringLiteral("RestoreViewSettingsTool"));
  RestoreViewSettingsTool->setMinimumSize(QSize(48, 48));
  RestoreViewSettingsTool->setMaximumSize(QSize(48, 48));
  QIcon iconRestoreViewSettings;
  iconRestoreViewSettings.addFile(QStringLiteral(":/cap/toolPixMaps/RestoreViewSettings.png"), QSize(), QIcon::Normal, QIcon::On);
  RestoreViewSettingsTool->setIcon(iconRestoreViewSettings);
  RestoreViewSettingsTool->setIconSize(QSize(48, 48));
  RestoreViewSettingsTool->setCheckable(false);
  RestoreViewSettingsTool->setToolTip(tr("Restore View Settings"));
  RestoreViewSettingsTool->setWhatsThis(tr("RestoreViewSettings allows the user to reset the viewer to settings that had been previously saved"));
  RestoreViewSettingsTool->setText(tr("RestoreViewSettings"));
  connect(RestoreViewSettingsTool, SIGNAL(clicked()), this, SLOT(RestoreViewSettings()));
  if (masterWithRespectToMenu) {
    RestoreViewSettingsAction = new QAction(RestoreViewSettingsTool->toolTip(), owner);
    QString objName = setName;
    RestoreViewSettingsAction->setObjectName(objName.append(RestoreViewSettingsTool->toolTip()));
    sisterMenu->addAction(RestoreViewSettingsAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == RestoreViewSettingsTool->toolTip())
        if (a->objectName().contains(setName))
          RestoreViewSettingsAction = a;
  }

  ShapeIntensityTool = new QToolButton(controlsBox);
  ShapeIntensityTool->setObjectName(QStringLiteral("ShapeIntensityTool"));
  ShapeIntensityTool->setMinimumSize(QSize(48, 48));
  ShapeIntensityTool->setMaximumSize(QSize(48, 48));
  QIcon iconShapeIntensity;
  iconShapeIntensity.addFile(QStringLiteral(":/cap/toolPixMaps/ShapeIntensity.png"), QSize(), QIcon::Normal, QIcon::On);
  ShapeIntensityTool->setIcon(iconShapeIntensity);
  ShapeIntensityTool->setIconSize(QSize(48, 48));
  ShapeIntensityTool->setCheckable(false);
  ShapeIntensityTool->setToolTip(tr("Shape Intensity Characteristics"));
  ShapeIntensityTool->setWhatsThis(tr("ShapeIntensity allows the user to optimize the gray level settings depicted in the imagery"));
  ShapeIntensityTool->setText(tr("ShapeIntensity"));
  connect(ShapeIntensityTool, SIGNAL(clicked()), this, SLOT(ShapeIntensity()));
  if (masterWithRespectToMenu) {
    ShapeIntensityAction = new QAction(ShapeIntensityTool->toolTip(), owner);
    QString objName = setName;
    ShapeIntensityAction->setObjectName(objName.append(ShapeIntensityTool->toolTip()));
    sisterMenu->addAction(ShapeIntensityAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == ShapeIntensityTool->toolTip())
        if (a->objectName().contains(setName))
          ShapeIntensityAction = a;
  }

  VolumeRenderTool = new QToolButton(controlsBox);
  VolumeRenderTool->setObjectName(QStringLiteral("VolumeRenderTool"));
  VolumeRenderTool->setMinimumSize(QSize(48, 48));
  VolumeRenderTool->setMaximumSize(QSize(48, 48));
  QIcon iconVolumeRender;
  iconVolumeRender.addFile(QStringLiteral(":/cap/toolPixMaps/VolumeRender.png"), QSize(), QIcon::Normal, QIcon::On);
  VolumeRenderTool->setIcon(iconVolumeRender);
  VolumeRenderTool->setIconSize(QSize(48, 48));
  VolumeRenderTool->setCheckable(true);
  VolumeRenderTool->setChecked(false);
  VolumeRenderTool->setToolTip(tr("Toggle Volume Rendering Display"));
  VolumeRenderTool->setWhatsThis(tr("VolumeRender allows the user to disable (or enable) volume rendering"));
  VolumeRenderTool->setText(tr("VolumeRender"));
  connect(VolumeRenderTool, SIGNAL(clicked(bool)), this, SLOT(VolumeRender(bool)));
  if (masterWithRespectToMenu) {
    VolumeRenderAction = new QAction(VolumeRenderTool->toolTip(), owner);
    QString objName = setName;
    VolumeRenderAction->setObjectName(objName.append(VolumeRenderTool->toolTip()));
    sisterMenu->addAction(VolumeRenderAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == VolumeRenderTool->toolTip())
        if (a->objectName().contains(setName))
          VolumeRenderAction = a;
  }

  OptimizeSubvolumeTool = new QToolButton(controlsBox);
  OptimizeSubvolumeTool->setObjectName(QStringLiteral("OptimizeSubvolumeTool"));
  OptimizeSubvolumeTool->setMinimumSize(QSize(48, 48));
  OptimizeSubvolumeTool->setMaximumSize(QSize(48, 48));
  QIcon iconOptimizeSubvolume;
  iconOptimizeSubvolume.addFile(QStringLiteral(":/cap/toolPixMaps/OptimizeSubvolume.png"), QSize(), QIcon::Normal, QIcon::On);
  OptimizeSubvolumeTool->setIcon(iconOptimizeSubvolume);
  OptimizeSubvolumeTool->setIconSize(QSize(48, 48));
  OptimizeSubvolumeTool->setCheckable(true);
  OptimizeSubvolumeTool->setChecked(false);
  OptimizeSubvolumeTool->setToolTip(tr("Optimize Subvolume Rendering"));
  OptimizeSubvolumeTool->setWhatsThis(tr("OptimizeSubvolume allows the user to trim the gray level settings depicted in the imagery"));
  OptimizeSubvolumeTool->setText(tr("OptimizeSubvolume"));
  connect(OptimizeSubvolumeTool, SIGNAL(clicked(bool)), this, SLOT(OptimizeSubvolume(bool)));
  if (masterWithRespectToMenu) {
    OptimizeSubvolumeAction = new QAction(OptimizeSubvolumeTool->toolTip(), owner);
    QString objName = setName;
    OptimizeSubvolumeAction->setObjectName(objName.append(OptimizeSubvolumeTool->toolTip()));
    sisterMenu->addAction(OptimizeSubvolumeAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == OptimizeSubvolumeTool->toolTip())
        if (a->objectName().contains(setName))
          OptimizeSubvolumeAction = a;
  }

  ToggleAxialTool = new QToolButton(controlsBox);
  ToggleAxialTool->setObjectName(QStringLiteral("ToggleAxialTool"));
  ToggleAxialTool->setMinimumSize(QSize(48, 48));
  ToggleAxialTool->setMaximumSize(QSize(48, 48));
  QIcon iconToggleAxial;
  iconToggleAxial.addFile(QStringLiteral(":/cap/toolPixMaps/ToggleAxial.png"), QSize(), QIcon::Normal, QIcon::On);
  ToggleAxialTool->setIcon(iconToggleAxial);
  ToggleAxialTool->setIconSize(QSize(48, 48));
  ToggleAxialTool->setCheckable(true);
  ToggleAxialTool->setChecked(false);
  ToggleAxialTool->setToolTip(tr("Toggle Axial Plane Display"));
  ToggleAxialTool->setWhatsThis(tr("ToggleAxial allows the user to show (or hide) the axial plane in the 3D view"));
  ToggleAxialTool->setText(tr("ToggleAxial"));
  connect(ToggleAxialTool, SIGNAL(clicked(bool)), this, SLOT(ToggleAxial(bool)));
  if (masterWithRespectToMenu) {
    ToggleAxialAction = new QAction(ToggleAxialTool->toolTip(), owner);
    QString objName = setName;
    ToggleAxialAction->setObjectName(objName.append(ToggleAxialTool->toolTip()));
    sisterMenu->addAction(ToggleAxialAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == ToggleAxialTool->toolTip())
        if (a->objectName().contains(setName))
          ToggleAxialAction = a;
  }
  //ToggleAxialAction->setShortcut("a");

  ToggleSagittalTool = new QToolButton(controlsBox);
  ToggleSagittalTool->setObjectName(QStringLiteral("ToggleSagittalTool"));
  ToggleSagittalTool->setMinimumSize(QSize(48, 48));
  ToggleSagittalTool->setMaximumSize(QSize(48, 48));
  QIcon iconToggleSagittal;
  iconToggleSagittal.addFile(QStringLiteral(":/cap/toolPixMaps/ToggleSagittal.png"), QSize(), QIcon::Normal, QIcon::On);
  ToggleSagittalTool->setIcon(iconToggleSagittal);
  ToggleSagittalTool->setIconSize(QSize(48, 48));
  ToggleSagittalTool->setCheckable(true);
  ToggleSagittalTool->setChecked(false);
  ToggleSagittalTool->setToolTip(tr("Toggle Sagittal Plane Display"));
  ToggleSagittalTool->setWhatsThis(tr("ToggleSagittal allows the user to show (or hide) the sagittal plane in the 3D view"));
  ToggleSagittalTool->setText(tr("ToggleSagittal"));
  connect(ToggleSagittalTool, SIGNAL(clicked(bool)), this, SLOT(ToggleSagittal(bool)));
  if (masterWithRespectToMenu) {
    ToggleSagittalAction = new QAction(ToggleSagittalTool->toolTip(), owner);
    QString objName = setName;
    ToggleSagittalAction->setObjectName(objName.append(ToggleSagittalTool->toolTip()));
    sisterMenu->addAction(ToggleSagittalAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == ToggleSagittalTool->toolTip())
        if (a->objectName().contains(setName))
          ToggleSagittalAction = a;
  }
  //ToggleSagittalAction->setShortcut(Qt::Key_S|Qt::NoModifier);

  ToggleCoronalTool = new QToolButton(controlsBox);
  ToggleCoronalTool->setObjectName(QStringLiteral("ToggleCoronalTool"));
  ToggleCoronalTool->setMinimumSize(QSize(48, 48));
  ToggleCoronalTool->setMaximumSize(QSize(48, 48));
  QIcon iconToggleCoronal;
  iconToggleCoronal.addFile(QStringLiteral(":/cap/toolPixMaps/ToggleCoronal.png"), QSize(), QIcon::Normal, QIcon::On);
  ToggleCoronalTool->setIcon(iconToggleCoronal);
  ToggleCoronalTool->setIconSize(QSize(48, 48));
  ToggleCoronalTool->setCheckable(true);
  ToggleCoronalTool->setChecked(false);
  ToggleCoronalTool->setToolTip(tr("Toggle Coronal Plane Display"));
  ToggleCoronalTool->setWhatsThis(tr("ToggleCoronal allows the user to show (or hide) the coronal plane in the 3D view"));
  ToggleCoronalTool->setText(tr("ToggleCoronal"));
  connect(ToggleCoronalTool, SIGNAL(clicked(bool)), this, SLOT(ToggleCoronal(bool)));
  if (masterWithRespectToMenu) {
    ToggleCoronalAction = new QAction(ToggleCoronalTool->toolTip(), owner);
    QString objName = setName;
    ToggleCoronalAction->setObjectName(objName.append(ToggleCoronalTool->toolTip()));
    sisterMenu->addAction(ToggleCoronalAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == ToggleCoronalTool->toolTip())
        if (a->objectName().contains(setName))
          ToggleCoronalAction = a;
  }
  //ToggleCoronalAction->setShortcut(Qt::Key_C|Qt::NoModifier);

  ToggleWallTool = new QToolButton(controlsBox);
  ToggleWallTool->setObjectName(QStringLiteral("ToggleWallTool"));
  ToggleWallTool->setMinimumSize(QSize(48, 48));
  ToggleWallTool->setMaximumSize(QSize(48, 48));
  QIcon iconToggleWall;
  iconToggleWall.addFile(QStringLiteral(":/cap/toolPixMaps/ToggleWall.png"), QSize(), QIcon::Normal, QIcon::On);
  ToggleWallTool->setIcon(iconToggleWall);
  ToggleWallTool->setIconSize(QSize(48, 48));
  ToggleWallTool->setCheckable(true);
  ToggleWallTool->setChecked(false);
  ToggleWallTool->setToolTip(tr("Toggle Wall Display"));
  ToggleWallTool->setWhatsThis(tr("ToggleWall allows the user to show (or hide) the wall segmentation display"));
  ToggleWallTool->setText(tr("ToggleWall"));
  connect(ToggleWallTool, SIGNAL(clicked(bool)), this, SLOT(ToggleWall(bool)));
  if (masterWithRespectToMenu) {
    ToggleWallAction = new QAction(ToggleWallTool->toolTip(), owner);
    QString objName = setName;
    ToggleWallAction->setObjectName(objName.append(ToggleWallTool->toolTip()));
    sisterMenu->addAction(ToggleWallAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == ToggleWallTool->toolTip())
        if (a->objectName().contains(setName))
          ToggleWallAction = a;
  }

  ToggleLumenTool = new QToolButton(controlsBox);
  ToggleLumenTool->setObjectName(QStringLiteral("ToggleLumenTool"));
  ToggleLumenTool->setMinimumSize(QSize(48, 48));
  ToggleLumenTool->setMaximumSize(QSize(48, 48));
  QIcon iconToggleLumen;
  iconToggleLumen.addFile(QStringLiteral(":/cap/toolPixMaps/ToggleLumen.png"), QSize(), QIcon::Normal, QIcon::On);
  ToggleLumenTool->setIcon(iconToggleLumen);
  ToggleLumenTool->setIconSize(QSize(48, 48));
  ToggleLumenTool->setCheckable(true);
  ToggleLumenTool->setChecked(false);
  ToggleLumenTool->setToolTip(tr("Toggle Lumen Display"));
  ToggleLumenTool->setWhatsThis(tr("ToggleLumen allows the user to show (or hide) the lumen segmentation display"));
  ToggleLumenTool->setText(tr("ToggleLumen"));
  connect(ToggleLumenTool, SIGNAL(clicked(bool)), this, SLOT(ToggleLumen(bool)));
  if (masterWithRespectToMenu) {
    ToggleLumenAction = new QAction(ToggleLumenTool->toolTip(), owner);
    QString objName = setName;
    ToggleLumenAction->setObjectName(objName.append(ToggleLumenTool->toolTip()));
    sisterMenu->addAction(ToggleLumenAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == ToggleLumenTool->toolTip())
        if (a->objectName().contains(setName))
          ToggleLumenAction = a;
  }

  ToggleColorBarTool = new QToolButton(controlsBox);
  ToggleColorBarTool->setObjectName(QStringLiteral("ToggleColorBarTool"));
  ToggleColorBarTool->setMinimumSize(QSize(48, 48));
  ToggleColorBarTool->setMaximumSize(QSize(48, 48));
  QIcon iconToggleColorBar;
  iconToggleColorBar.addFile(QStringLiteral(":/cap/toolPixMaps/ToggleColorBar.png"), QSize(), QIcon::Normal, QIcon::On);
  ToggleColorBarTool->setIcon(iconToggleColorBar);
  ToggleColorBarTool->setIconSize(QSize(48, 48));
  ToggleColorBarTool->setCheckable(true);
  ToggleColorBarTool->setChecked(false);
  ToggleColorBarTool->setToolTip(tr("Toggle Tissue Characteristics Display"));
  ToggleColorBarTool->setWhatsThis(tr("ToggleColorBar allows the user to show (or hide) the color presentation bar"));
  ToggleColorBarTool->setText(tr("ToggleColorBar"));
  connect(ToggleColorBarTool, SIGNAL(clicked(bool)), this, SLOT(ToggleColorBar(bool)));
  if (masterWithRespectToMenu) {
    ToggleColorBarAction = new QAction(ToggleColorBarTool->toolTip(), owner);
    QString objName = setName;
    ToggleColorBarAction->setObjectName(objName.append(ToggleColorBarTool->toolTip()));
    sisterMenu->addAction(ToggleColorBarAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == ToggleColorBarTool->toolTip())
        if (a->objectName().contains(setName))
          ToggleColorBarAction = a;
  }

  ToggleCornerTool = new QToolButton(controlsBox);
  ToggleCornerTool->setObjectName(QStringLiteral("ToggleCornerTool"));
  ToggleCornerTool->setMinimumSize(QSize(48, 48));
  ToggleCornerTool->setMaximumSize(QSize(48, 48));
  QIcon iconToggleCorner;
  iconToggleCorner.addFile(QStringLiteral(":/cap/toolPixMaps/ToggleCorner.png"), QSize(), QIcon::Normal, QIcon::On);
  ToggleCornerTool->setIcon(iconToggleCorner);
  ToggleCornerTool->setIconSize(QSize(48, 48));
  ToggleCornerTool->setCheckable(true);
  ToggleCornerTool->setChecked(false);
  ToggleCornerTool->setToolTip(tr("Toggle Corner Annotations"));
  ToggleCornerTool->setWhatsThis(tr("ToggleCorner allows the user to show (or hide) the corner annotations"));
  ToggleCornerTool->setText(tr("ToggleCorner"));
  connect(ToggleCornerTool, SIGNAL(clicked(bool)), this, SLOT(ToggleCorner(bool)));
  if (masterWithRespectToMenu) {
    ToggleCornerAction = new QAction(ToggleCornerTool->toolTip(), owner);
    QString objName = setName;
    ToggleCornerAction->setObjectName(objName.append(ToggleCornerTool->toolTip()));
    sisterMenu->addAction(ToggleCornerAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == ToggleCornerTool->toolTip())
        if (a->objectName().contains(setName))
          ToggleCornerAction = a;
  }

  MoveDistalTool = new QToolButton(controlsBox);
  MoveDistalTool->setObjectName(QStringLiteral("MoveDistalTool"));
  MoveDistalTool->setMinimumSize(QSize(48, 48));
  MoveDistalTool->setMaximumSize(QSize(48, 48));
  QIcon iconMoveDistal;
  iconMoveDistal.addFile(QStringLiteral(":/cap/toolPixMaps/MoveDistal.png"), QSize(), QIcon::Normal, QIcon::On);
  MoveDistalTool->setIcon(iconMoveDistal);
  MoveDistalTool->setIcon(iconMoveDistal);
  MoveDistalTool->setIconSize(QSize(48, 48));
  MoveDistalTool->setCheckable(false);
  MoveDistalTool->setToolTip(tr("Move Distal"));
  MoveDistalTool->setWhatsThis(tr("MoveDistal moves the view to the next point on the current path, towards its distal end"));
  MoveDistalTool->setText(tr("MoveDistal"));
  connect(MoveDistalTool, SIGNAL(clicked()), this, SLOT(MoveDistal()));
  if (masterWithRespectToMenu) {
    MoveDistalAction = new QAction(MoveDistalTool->toolTip(), owner);
    QString objName = setName;
    MoveDistalAction->setObjectName(objName.append(MoveDistalTool->toolTip()));
    sisterMenu->addAction(MoveDistalAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == MoveDistalTool->toolTip())
        if (a->objectName().contains(setName))
          MoveDistalAction = a;
  }
  //MoveDistalAction->setShortcut(Qt::Key_Period);

  MoveProximalTool = new QToolButton(controlsBox);
  MoveProximalTool->setObjectName(QStringLiteral("MoveProximalTool"));
  MoveProximalTool->setMinimumSize(QSize(48, 48));
  MoveProximalTool->setMaximumSize(QSize(48, 48));
  QIcon iconMoveProximal;
  iconMoveProximal.addFile(QStringLiteral(":/cap/toolPixMaps/MoveProximal.png"), QSize(), QIcon::Normal, QIcon::On);
  MoveProximalTool->setIcon(iconMoveProximal);
  MoveProximalTool->setIconSize(QSize(48, 48));
  MoveProximalTool->setCheckable(false);
  MoveProximalTool->setToolTip(tr("Move Proximal"));
  MoveProximalTool->setWhatsThis(tr("MoveProximal moves the view to the next point on the current path, towards its proximal end"));
  MoveProximalTool->setText(tr("MoveProximal"));
  connect(MoveProximalTool, SIGNAL(clicked()), this, SLOT(MoveProximal()));
  if (masterWithRespectToMenu) {
    MoveProximalAction = new QAction(MoveProximalTool->toolTip(), owner);
    QString objName = setName;
    MoveProximalAction->setObjectName(objName.append(MoveProximalTool->toolTip()));
    sisterMenu->addAction(MoveProximalAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == MoveProximalTool->toolTip())
        if (a->objectName().contains(setName))
          MoveProximalAction = a;
  }
  //MoveProximalAction->setShortcut(Qt::Key_Comma);

  TargetPathSwitchTool = new QToolButton(controlsBox);
  TargetPathSwitchTool->setObjectName(QStringLiteral("TargetPathSwitchTool"));
  TargetPathSwitchTool->setMinimumSize(QSize(48, 48));
  TargetPathSwitchTool->setMaximumSize(QSize(48, 48));
  QIcon iconTargetPathSwitch;
  iconTargetPathSwitch.addFile(QStringLiteral(":/cap/toolPixMaps/TargetPathSwitch.png"), QSize(), QIcon::Normal, QIcon::On);
  TargetPathSwitchTool->setIcon(iconTargetPathSwitch);
  TargetPathSwitchTool->setIconSize(QSize(48, 48));
  TargetPathSwitchTool->setCheckable(false);
  TargetPathSwitchTool->setToolTip(tr("Switch Current Target/Path"));
  TargetPathSwitchTool->setWhatsThis(tr("TargetPathSwitch moves the view to the next available path"));
  TargetPathSwitchTool->setText(tr("TargetPathSwitch"));
  connect(TargetPathSwitchTool, SIGNAL(clicked()), this, SLOT(TargetPathSwitch()));
  if (masterWithRespectToMenu) {
    TargetPathSwitchAction = new QAction(TargetPathSwitchTool->toolTip(), owner);
    QString objName = setName;
    TargetPathSwitchAction->setObjectName(objName.append(TargetPathSwitchTool->toolTip()));
    sisterMenu->addAction(TargetPathSwitchAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == TargetPathSwitchTool->toolTip())
        if (a->objectName().contains(setName))
          TargetPathSwitchAction = a;
  }
  //TargetPathSwitchAction->setShortcut(Qt::Key_Greater);

  TargetDeleteTool = new QToolButton(controlsBox);
  TargetDeleteTool->setObjectName(QStringLiteral("TargetDeleteTool"));
  TargetDeleteTool->setMinimumSize(QSize(48, 48));
  TargetDeleteTool->setMaximumSize(QSize(48, 48));
  QIcon iconTargetDelete;
  iconTargetDelete.addFile(QStringLiteral(":/cap/toolPixMaps/TargetDelete.png"), QSize(), QIcon::Normal, QIcon::On);
  TargetDeleteTool->setIcon(iconTargetDelete);
  TargetDeleteTool->setIconSize(QSize(48, 48));
  TargetDeleteTool->setCheckable(false);
  TargetDeleteTool->setToolTip(tr("Delete Current Target"));
  TargetDeleteTool->setWhatsThis(tr("TargetDelete allows the user to delete the current analysis target"));
  TargetDeleteTool->setText(tr("TargetDelete"));
  connect(TargetDeleteTool, SIGNAL(clicked()), this, SLOT(TargetDelete()));
  if (masterWithRespectToMenu) {
    TargetDeleteAction = new QAction(TargetDeleteTool->toolTip(), owner);
    QString objName = setName;
    TargetDeleteAction->setObjectName(objName.append(TargetDeleteTool->toolTip()));
    sisterMenu->addAction(TargetDeleteAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == TargetDeleteTool->toolTip())
        if (a->objectName().contains(setName))
          TargetDeleteAction = a;
  }

  TargetModifyTool = new QToolButton(controlsBox);
  TargetModifyTool->setObjectName(QStringLiteral("TargetModifyTool"));
  TargetModifyTool->setMinimumSize(QSize(48, 48));
  TargetModifyTool->setMaximumSize(QSize(48, 48));
  QIcon iconTargetModify;
  iconTargetModify.addFile(QStringLiteral(":/cap/toolPixMaps/TargetModify.png"), QSize(), QIcon::Normal, QIcon::On);
  TargetModifyTool->setIcon(iconTargetModify);
  TargetModifyTool->setIconSize(QSize(48, 48));
  TargetModifyTool->setToolTip(tr("Modify Current Target"));
  TargetModifyTool->setWhatsThis(tr("TargetModify allows the user to modify the current analysis target"));
  TargetModifyTool->setText(tr("TargetModify"));
  connect(TargetModifyTool, SIGNAL(clicked()), this, SLOT(TargetModify()));
  if (masterWithRespectToMenu) {
    TargetModifyAction = new QAction(TargetModifyTool->toolTip(), owner);
    QString objName = setName;
    TargetModifyAction->setObjectName(objName.append(TargetModifyTool->toolTip()));
    sisterMenu->addAction(TargetModifyAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == TargetModifyTool->toolTip())
        if (a->objectName().contains(setName))
          TargetModifyAction = a;
  }

  TargetCreateTool = new QToolButton(controlsBox);
  TargetCreateTool->setObjectName(QStringLiteral("TargetCreateTool"));
  TargetCreateTool->setMinimumSize(QSize(48, 48));
  TargetCreateTool->setMaximumSize(QSize(48, 48));
  QIcon iconTargetCreate;
  iconTargetCreate.addFile(QStringLiteral(":/cap/toolPixMaps/TargetCreate.png"), QSize(), QIcon::Normal, QIcon::Off);
  iconTargetCreate.addFile(QStringLiteral(":/cap/toolPixMaps/EndPointCollection.png"), QSize(), QIcon::Normal, QIcon::On);
  TargetCreateTool->setIcon(iconTargetCreate);
  TargetCreateTool->setIconSize(QSize(48, 48));
  TargetCreateTool->setCheckable(true);
  TargetCreateTool->setChecked(false);
  TargetCreateTool->setToolTip(tr("Create New Target"));
  TargetCreateTool->setWhatsThis(tr("TargetCreate allows the user to add a new analysis target"));
  TargetCreateTool->setText(tr("TargetCreate"));
  connect(TargetCreateTool, SIGNAL(clicked(bool)), this, SLOT(TargetCreate(bool)));
  if (masterWithRespectToMenu) {
    TargetCreateAction = new QAction(TargetCreateTool->toolTip(), owner);
    QString objName = setName;
    TargetCreateAction->setObjectName(objName.append(TargetCreateTool->toolTip()));
    sisterMenu->addAction(TargetCreateAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == TargetCreateTool->toolTip())
        if (a->objectName().contains(setName))
          TargetCreateAction = a;
  }

  MaxStenosisTool = new QToolButton(controlsBox);
  MaxStenosisTool->setObjectName(QStringLiteral("MaxStenosisTool"));
  MaxStenosisTool->setMinimumSize(QSize(48, 48));
  MaxStenosisTool->setMaximumSize(QSize(48, 48));
  QIcon iconMaxStenosis;
  iconMaxStenosis.addFile(QStringLiteral(":/cap/toolPixMaps/MaxStenosis.png"), QSize(), QIcon::Normal, QIcon::On);
  MaxStenosisTool->setIcon(iconMaxStenosis);
  MaxStenosisTool->setIconSize(QSize(48, 48));
  MaxStenosisTool->setCheckable(false);
  MaxStenosisTool->setToolTip(tr("Snap to Max Stenosis"));
  MaxStenosisTool->setWhatsThis(tr("MaxStenosis snaps to the cross section with the maximum stenosis value for the selected target"));
  MaxStenosisTool->setText(tr("MaxStenosis"));
  connect(MaxStenosisTool, SIGNAL(clicked()), this, SLOT(MaxStenosis()));
  if (masterWithRespectToMenu) {
    MaxStenosisAction = new QAction(MaxStenosisTool->toolTip(), owner);
    QString objName = setName;
    MaxStenosisAction->setObjectName(objName.append(MaxStenosisTool->toolTip()));
    sisterMenu->addAction(MaxStenosisAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == MaxStenosisTool->toolTip())
        if (a->objectName().contains(setName))
          MaxStenosisAction = a;
  }

  MaxDilationTool = new QToolButton(controlsBox);
  MaxDilationTool->setObjectName(QStringLiteral("MaxDilationTool"));
  MaxDilationTool->setMinimumSize(QSize(48, 48));
  MaxDilationTool->setMaximumSize(QSize(48, 48));
  QIcon iconMaxDilation;
  iconMaxDilation.addFile(QStringLiteral(":/cap/toolPixMaps/MaxDilation.png"), QSize(), QIcon::Normal, QIcon::On);
  MaxDilationTool->setIcon(iconMaxDilation);
  MaxDilationTool->setIconSize(QSize(48, 48));
  MaxDilationTool->setCheckable(false);
  MaxDilationTool->setToolTip(tr("Snap to Max Dilation"));
  MaxDilationTool->setWhatsThis(tr("MaxDilation snaps to the cross section with the maximum dilation value for the selected target"));
  MaxDilationTool->setText(tr("MaxDilation"));
  connect(MaxDilationTool, SIGNAL(clicked()), this, SLOT(MaxDilation()));
  if (masterWithRespectToMenu) {
    MaxDilationAction = new QAction(MaxDilationTool->toolTip(), owner);
    QString objName = setName;
    MaxDilationAction->setObjectName(objName.append(MaxDilationTool->toolTip()));
    sisterMenu->addAction(MaxDilationAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == MaxDilationTool->toolTip())
        if (a->objectName().contains(setName))
          MaxDilationAction = a;
  }

  MaxRRTool = new QToolButton(controlsBox);
  MaxRRTool->setObjectName(QStringLiteral("MaxRRTool"));
  MaxRRTool->setMinimumSize(QSize(48, 48));
  MaxRRTool->setMaximumSize(QSize(48, 48));
  QIcon iconMaxRR;
  iconMaxRR.addFile(QStringLiteral(":/cap/toolPixMaps/MaxRR.png"), QSize(), QIcon::Normal, QIcon::On);
  MaxRRTool->setIcon(iconMaxRR);
  MaxRRTool->setIconSize(QSize(48, 48));
  MaxRRTool->setCheckable(false);
  MaxRRTool->setToolTip(tr("Snap to Max Remodeling Ratio"));
  MaxRRTool->setWhatsThis(tr("MaxRR snaps to the cross section with the maximum remodeling ratio value for the selected target"));
  MaxRRTool->setText(tr("MaxRR"));
  connect(MaxRRTool, SIGNAL(clicked()), this, SLOT(MaxRR()));
  if (masterWithRespectToMenu) {
    MaxRRAction = new QAction(MaxRRTool->toolTip(), owner);
    QString objName = setName;
    MaxRRAction->setObjectName(objName.append(MaxRRTool->toolTip()));
    sisterMenu->addAction(MaxRRAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == MaxRRTool->toolTip())
        if (a->objectName().contains(setName))
          MaxRRAction = a;
  }

  MaxWTTool = new QToolButton(controlsBox);
  MaxWTTool->setObjectName(QStringLiteral("MaxWTTool"));
  MaxWTTool->setMinimumSize(QSize(48, 48));
  MaxWTTool->setMaximumSize(QSize(48, 48));
  QIcon iconMaxWT;
  iconMaxWT.addFile(QStringLiteral(":/cap/toolPixMaps/MaxWT.png"), QSize(), QIcon::Normal, QIcon::On);
  MaxWTTool->setIcon(iconMaxWT);
  MaxWTTool->setIconSize(QSize(48, 48));
  MaxWTTool->setCheckable(false);
  MaxWTTool->setToolTip(tr("Snap to Max Wall Thickness"));
  MaxWTTool->setWhatsThis(tr("MaxWT snaps to the cross section with the maximum wall thickness value for the selected target"));
  MaxWTTool->setText(tr("MaxWT"));
  connect(MaxWTTool, SIGNAL(clicked()), this, SLOT(MaxWT()));
  if (masterWithRespectToMenu) {
    MaxWTAction = new QAction(MaxWTTool->toolTip(), owner);
    QString objName = setName;
    MaxWTAction->setObjectName(objName.append(MaxWTTool->toolTip()));
    sisterMenu->addAction(MaxWTAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == MaxWTTool->toolTip())
        if (a->objectName().contains(setName))
          MaxWTAction = a;
  }

  MaxCalcTool = new QToolButton(controlsBox);
  MaxCalcTool->setObjectName(QStringLiteral("MaxCalcTool"));
  MaxCalcTool->setMinimumSize(QSize(48, 48));
  MaxCalcTool->setMaximumSize(QSize(48, 48));
  QIcon iconMaxCalc;
  iconMaxCalc.addFile(QStringLiteral(":/cap/toolPixMaps/MaxCalc.png"), QSize(), QIcon::Normal, QIcon::On);
  MaxCalcTool->setIcon(iconMaxCalc);
  MaxCalcTool->setIconSize(QSize(48, 48));
  MaxCalcTool->setCheckable(false);
  MaxCalcTool->setToolTip(tr("Snap to Max Calcification"));
  MaxCalcTool->setWhatsThis(tr("MaxCalc snaps to the cross section with the maximum calcification for the selected target"));
  MaxCalcTool->setText(tr("MaxCalc"));
  connect(MaxCalcTool, SIGNAL(clicked()), this, SLOT(MaxCalc()));
  if (masterWithRespectToMenu) {
    MaxCalcAction = new QAction(MaxCalcTool->toolTip(), owner);
    QString objName = setName;
    MaxCalcAction->setObjectName(objName.append(MaxCalcTool->toolTip()));
    sisterMenu->addAction(MaxCalcAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == MaxCalcTool->toolTip())
        if (a->objectName().contains(setName))
          MaxCalcAction = a;
  }

  MaxLRNCTool = new QToolButton(controlsBox);
  MaxLRNCTool->setObjectName(QStringLiteral("MaxLRNCTool"));
  MaxLRNCTool->setMinimumSize(QSize(48, 48));
  MaxLRNCTool->setMaximumSize(QSize(48, 48));
  QIcon iconMaxLRNC;
  iconMaxLRNC.addFile(QStringLiteral(":/cap/toolPixMaps/MaxLRNC.png"), QSize(), QIcon::Normal, QIcon::On);
  MaxLRNCTool->setIcon(iconMaxLRNC);
  MaxLRNCTool->setIconSize(QSize(48, 48));
  MaxLRNCTool->setCheckable(false);
  MaxLRNCTool->setToolTip(tr("Snap to Max LRNC"));
  MaxLRNCTool->setWhatsThis(tr("MaxLRNC snaps to the cross section with the maximum lipid core for the selected target"));
  MaxLRNCTool->setText(tr("MaxLRNC"));
  connect(MaxLRNCTool, SIGNAL(clicked()), this, SLOT(MaxLRNC()));
  if (masterWithRespectToMenu) {
    MaxLRNCAction = new QAction(MaxLRNCTool->toolTip(), owner);
    QString objName = setName;
    MaxLRNCAction->setObjectName(objName.append(MaxLRNCTool->toolTip()));
    sisterMenu->addAction(MaxLRNCAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == MaxLRNCTool->toolTip())
        if (a->objectName().contains(setName))
          MaxLRNCAction = a;
  }

  MaxMATXTool = new QToolButton(controlsBox);
  MaxMATXTool->setObjectName(QStringLiteral("MaxMATXTool"));
  MaxMATXTool->setMinimumSize(QSize(48, 48));
  MaxMATXTool->setMaximumSize(QSize(48, 48));
  QIcon iconMaxMATX;
  iconMaxMATX.addFile(QStringLiteral(":/cap/toolPixMaps/MaxMATX.png"), QSize(), QIcon::Normal, QIcon::On);
  MaxMATXTool->setIcon(iconMaxMATX);
  MaxMATXTool->setIconSize(QSize(48, 48));
  MaxMATXTool->setCheckable(false);
  MaxMATXTool->setToolTip(tr("Snap to Max Matrix"));
  MaxMATXTool->setWhatsThis(tr("MaxMATX snaps to the cross section with the maximum matrix for the selected target"));
  MaxMATXTool->setText(tr("MaxMATX"));
  connect(MaxMATXTool, SIGNAL(clicked()), this, SLOT(MaxMATX()));
  if (masterWithRespectToMenu) {
    MaxMATXAction = new QAction(MaxMATXTool->toolTip(), owner);
    QString objName = setName;
    MaxMATXAction->setObjectName(objName.append(MaxMATXTool->toolTip()));
    sisterMenu->addAction(MaxMATXAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == MaxMATXTool->toolTip())
        if (a->objectName().contains(setName))
          MaxMATXAction = a;
  }

  MaxIPHTool = new QToolButton(controlsBox);
  MaxIPHTool->setObjectName(QStringLiteral("MaxIPHTool"));
  MaxIPHTool->setMinimumSize(QSize(48, 48));
  MaxIPHTool->setMaximumSize(QSize(48, 48));
  QIcon iconMaxIPH;
  iconMaxIPH.addFile(QStringLiteral(":/cap/toolPixMaps/MaxIPH.png"), QSize(), QIcon::Normal, QIcon::On);
  MaxIPHTool->setIcon(iconMaxIPH);
  MaxIPHTool->setIconSize(QSize(48, 48));
  MaxIPHTool->setCheckable(false);
  MaxIPHTool->setToolTip(tr("Snap to Max Intra-Plaque Hemorrhage"));
  MaxIPHTool->setWhatsThis(tr("MaxIPH snaps to the cross section with the maximum hemorrhage for the selected target"));
  MaxIPHTool->setText(tr("MaxIPH"));
  connect(MaxIPHTool, SIGNAL(clicked()), this, SLOT(MaxIPH()));
  if (masterWithRespectToMenu) {
    MaxIPHAction = new QAction(MaxIPHTool->toolTip(), owner);
    QString objName = setName;
    MaxIPHAction->setObjectName(objName.append(MaxIPHTool->toolTip()));
    sisterMenu->addAction(MaxIPHAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == MaxIPHTool->toolTip())
        if (a->objectName().contains(setName))
          MaxIPHAction = a;
  }

  MaxUlcTool = new QToolButton(controlsBox);
  MaxUlcTool->setObjectName(QStringLiteral("MaxUlcTool"));
  MaxUlcTool->setMinimumSize(QSize(48, 48));
  MaxUlcTool->setMaximumSize(QSize(48, 48));
  QIcon iconMaxUlc;
  iconMaxUlc.addFile(QStringLiteral(":/cap/toolPixMaps/MaxUlc.png"), QSize(), QIcon::Normal, QIcon::On);
  MaxUlcTool->setIcon(iconMaxUlc);
  MaxUlcTool->setIconSize(QSize(48, 48));
  MaxUlcTool->setCheckable(false);
  MaxUlcTool->setToolTip(tr("Snap to Max Ulceration"));
  MaxUlcTool->setWhatsThis(tr("MaxUlc snaps to the cross section with the maximum ulceration for the selected target"));
  MaxUlcTool->setText(tr("MaxUlc"));
  connect(MaxUlcTool, SIGNAL(clicked()), this, SLOT(MaxUlc()));
  if (masterWithRespectToMenu) {
    MaxUlcAction = new QAction(MaxUlcTool->toolTip(), owner);
    QString objName = setName;
    MaxUlcAction->setObjectName(objName.append(MaxUlcTool->toolTip()));
    sisterMenu->addAction(MaxUlcAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == MaxUlcTool->toolTip())
        if (a->objectName().contains(setName))
          MaxUlcAction = a;
  }

  MaxThrTool = new QToolButton(controlsBox);
  MaxThrTool->setObjectName(QStringLiteral("MaxThrTool"));
  MaxThrTool->setMinimumSize(QSize(48, 48));
  MaxThrTool->setMaximumSize(QSize(48, 48));
  QIcon iconMaxThr;
  iconMaxThr.addFile(QStringLiteral(":/cap/toolPixMaps/MaxThr.png"), QSize(), QIcon::Normal, QIcon::On);
  MaxThrTool->setIcon(iconMaxThr);
  MaxThrTool->setIconSize(QSize(48, 48));
  MaxThrTool->setCheckable(false);
  MaxThrTool->setToolTip(tr("Snap to Max Thrombosis"));
  MaxThrTool->setWhatsThis(tr("MaxThr snaps to the cross section with the maximum thrombus for the selected target"));
  MaxThrTool->setText(tr("MaxThr"));
  connect(MaxThrTool, SIGNAL(clicked()), this, SLOT(MaxThr()));
  if (masterWithRespectToMenu) {
    MaxThrAction = new QAction(MaxThrTool->toolTip(), owner);
    QString objName = setName;
    MaxThrAction->setObjectName(objName.append(MaxThrTool->toolTip()));
    sisterMenu->addAction(MaxThrAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == MaxThrTool->toolTip())
        if (a->objectName().contains(setName))
          MaxThrAction = a;
  }

  MeasureTool = new QToolButton(controlsBox);
  MeasureTool->setObjectName(QStringLiteral("MeasureTool"));
  MeasureTool->setMinimumSize(QSize(48, 48));
  MeasureTool->setMaximumSize(QSize(48, 48));
  QIcon iconMeasure;
  iconMeasure.addFile(QStringLiteral(":/cap/toolPixMaps/Measure.png"), QSize(), QIcon::Normal, QIcon::On);
  MeasureTool->setIcon(iconMeasure);
  MeasureTool->setIconSize(QSize(48, 48));
  MeasureTool->setCheckable(true);
  MeasureTool->setChecked(false);
  MeasureTool->setToolTip(tr("Make a manual caliper measurement"));
  MeasureTool->setWhatsThis(tr("Measure allows the user to make a manual caliper measurement"));
  MeasureTool->setText(tr("Measure"));
  connect(MeasureTool, SIGNAL(clicked(bool)), this, SLOT(Measure(bool)));
  if (masterWithRespectToMenu) {
    MeasureAction = new QAction(MeasureTool->toolTip(), owner);
    QString objName = setName;
    MeasureAction->setObjectName(objName.append(MeasureTool->toolTip()));
    sisterMenu->addAction(MeasureAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == MeasureTool->toolTip())
        if (a->objectName().contains(setName))
          MeasureAction = a;
  }
  //MeasureAction->setShortcut(Qt::Key_M|Qt::ShiftModifier);

  AnnotateTool = new QToolButton(controlsBox);
  AnnotateTool->setObjectName(QStringLiteral("AnnotateTool"));
  AnnotateTool->setMinimumSize(QSize(48, 48));
  AnnotateTool->setMaximumSize(QSize(48, 48));
  QIcon iconAnnotate;
  iconAnnotate.addFile(QStringLiteral(":/cap/toolPixMaps/Annotate.png"), QSize(), QIcon::Normal, QIcon::On);
  AnnotateTool->setIcon(iconAnnotate);
  AnnotateTool->setIconSize(QSize(48, 48));
  AnnotateTool->setCheckable(true);
  AnnotateTool->setChecked(false);
  if (setName.contains("SLICE"))
    AnnotateTool->setToolTip(tr("Annotate Slice Image"));
  else if (setName.contains("SUMMARY"))
    AnnotateTool->setToolTip(tr("Annotate 3D Image"));
  else
    AnnotateTool->setToolTip(tr("Annotate Image"));
  AnnotateTool->setWhatsThis(tr("Annotate allows the user to add markings to the view"));
  AnnotateTool->setText(tr("Place an Annotation"));
  connect(AnnotateTool, SIGNAL(clicked(bool)), this, SLOT(Annotate(bool)));
  if (masterWithRespectToMenu) {
    AnnotateAction = new QAction(AnnotateTool->toolTip(), owner);
    QString objName = setName;
    AnnotateAction->setObjectName(objName.append(AnnotateTool->toolTip()));
    sisterMenu->addAction(AnnotateAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == AnnotateTool->toolTip())
        if (a->objectName().contains(setName))
          AnnotateAction = a;
  }
  AnnotateTool->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(AnnotateTool, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(AnnotateContextMenu(QPoint)));

  ShowAsMovieTool = new QToolButton(controlsBox);
  ShowAsMovieTool->setObjectName(QStringLiteral("ShowAsMovieTool"));
  ShowAsMovieTool->setMinimumSize(QSize(48, 48));
  ShowAsMovieTool->setMaximumSize(QSize(48, 48));
  QIcon iconShowAsMovie;
  iconShowAsMovie.addFile(QStringLiteral(":/cap/toolPixMaps/ShowAsMovie.png"), QSize(), QIcon::Normal, QIcon::On);
  ShowAsMovieTool->setIcon(iconShowAsMovie);
  ShowAsMovieTool->setIconSize(QSize(48, 48));
  ShowAsMovieTool->setCheckable(false);
  if (setName.contains("SLICE"))
    ShowAsMovieTool->setToolTip(tr("Scroll Through Slices As a Movie"));
  else if (setName.contains("SUMMARY"))
    ShowAsMovieTool->setToolTip(tr("Rotate 3D As a Movie"));
  else
    ShowAsMovieTool->setToolTip(tr("Show As a Movie"));
  ShowAsMovieTool->setWhatsThis(tr("ShowAsMovie allows the user to view the scene in the viewport as a looping sequence"));
  ShowAsMovieTool->setText(tr("ShowAsMovie"));
  connect(ShowAsMovieTool, SIGNAL(clicked()), this, SLOT(ShowAsMovie()));
  if (masterWithRespectToMenu) {
    ShowAsMovieAction = new QAction(ShowAsMovieTool->toolTip(), owner);
    QString objName = setName;
    ShowAsMovieAction->setObjectName(objName.append(ShowAsMovieTool->toolTip()));
    sisterMenu->addAction(ShowAsMovieAction);
  }
  else {
    foreach (QAction *a, sisterMenu->actions())
      if (a->text() == ShowAsMovieTool->toolTip())
        if (a->objectName().contains(setName))
          ShowAsMovieAction = a;
  }
}

capTools::~capTools()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  delete ToggleLumenTool; delete ToggleLumenAction;
  delete ToggleWallTool; delete ToggleWallAction;
  delete MaxStenosisTool; delete MaxStenosisAction;
  delete MaxDilationTool; delete MaxDilationAction;
  delete MaxRRTool; delete MaxRRAction;
  delete MaxWTTool; delete MaxWTAction;
  delete ToggleCornerTool; delete ToggleCornerAction;
  delete OptimizeSubvolumeTool; delete OptimizeSubvolumeAction;
  delete ShapeIntensityTool; delete ShapeIntensityAction;
  delete SaveViewSettingsTool; delete SaveViewSettingsAction;
  delete RestoreViewSettingsTool; delete RestoreViewSettingsAction;
  delete AnnotateTool; delete AnnotateAction;
  delete MeasureTool; delete MeasureAction;
  delete MaxCalcTool; delete MaxCalcAction;
  delete MaxLRNCTool; delete MaxLRNCAction;
  delete MaxMATXTool; delete MaxMATXAction;
  delete MaxIPHTool; delete MaxIPHAction;
  delete MaxUlcTool; delete MaxUlcAction;
  delete MaxThrTool; delete MaxThrAction;
  delete TargetPathSwitchTool; delete TargetPathSwitchAction;
  delete TargetCreateTool; delete TargetCreateAction;
  delete TargetDeleteTool; delete TargetDeleteAction;
  delete TargetModifyTool; delete TargetModifyAction;
  delete ToggleObliqueTool; delete ToggleObliqueAction;
  delete VolumeRenderTool; delete VolumeRenderAction;
  delete ToggleCoronalTool; delete ToggleCoronalAction;
  delete ToggleSagittalTool; delete ToggleSagittalAction;
  delete ToggleAxialTool; delete ToggleAxialAction;
  delete ShowAsMovieTool; delete ShowAsMovieAction;
  delete ToggleColorBarTool; delete ToggleColorBarAction;
  delete CenterAtCursorTool; delete CenterAtCursorAction;
  delete MoveProximalTool; delete MoveProximalAction;
  delete MoveDistalTool; delete MoveDistalAction;
  delete SaveToReportTool; delete SaveToReportAction;
}

void capTools::disableToolButtons()
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  DISABLECONTROL(ToggleLumenTool, ToggleLumenAction, tr("ToggleLumen is disabled"));
  DISABLECONTROL(ToggleWallTool, ToggleWallAction, tr("ToggleWall is disabled"));
  DISABLECONTROL(MaxStenosisTool, MaxStenosisAction, tr("MaxStenosis is disabled (select a target to enable it)"));
  DISABLECONTROL(MaxDilationTool, MaxDilationAction, tr("MaxDilation is disabled (select a target to enable it)"));
  DISABLECONTROL(MaxRRTool, MaxRRAction, tr("MaxRR is disabled (select a target to enable it)"));
  DISABLECONTROL(MaxWTTool, MaxWTAction, tr("MaxWT is disabled (select a target to enable it)"));
  DISABLECONTROL(ToggleCornerTool, ToggleCornerAction, tr("ToggleCorner is disabled (select a target to enable it)"));
  DISABLECONTROL(OptimizeSubvolumeTool, OptimizeSubvolumeAction, tr("OptimizeSubvolume is disabled"));
  DISABLECONTROL(ShapeIntensityTool, ShapeIntensityAction, tr("ShapeIntensity is disabled"));
  DISABLECONTROL(SaveViewSettingsTool, SaveViewSettingsAction, tr("SaveViewSettings is disabled"));
  DISABLECONTROL(RestoreViewSettingsTool, RestoreViewSettingsAction, tr("RestoreViewSettings is disabled"));
  DISABLECONTROL(AnnotateTool, AnnotateAction, tr("Annotate is disabled"));
  DISABLECONTROL(MeasureTool, MeasureAction, tr("Measure is disabled"));
  DISABLECONTROL(MaxCalcTool, MaxCalcAction, tr("MaxCalc is disabled (select a target to enable it)"));
  DISABLECONTROL(MaxLRNCTool, MaxLRNCAction, tr("MaxLRNC is disabled (select a target to enable it)"));
  DISABLECONTROL(MaxMATXTool, MaxMATXAction, tr("MaxMATX is disabled (select a target to enable it)"));
  DISABLECONTROL(MaxIPHTool, MaxIPHAction, tr("MaxIPH is disabled (select a target to enable it)"));
  DISABLECONTROL(MaxUlcTool, MaxUlcAction, tr("MaxUlc is disabled (select a target to enable it)"));
  DISABLECONTROL(MaxThrTool, MaxThrAction, tr("MaxThr is disabled (select a target to enable it)"));
  DISABLECONTROL(TargetPathSwitchTool, TargetPathSwitchAction, tr("TargetPathSwitch is disabled (create a target to enable)"));
  DISABLECONTROL(TargetCreateTool, TargetCreateAction, tr("TargetCreate is disabled"));
  DISABLECONTROL(TargetDeleteTool, TargetDeleteAction, tr("TargetDelete is disabled (select or create a target to enable it)"));
  DISABLECONTROL(TargetModifyTool, TargetModifyAction, tr("TargetModify is disabled (select or create a target to enable it)"));
  DISABLECONTROL(ToggleObliqueTool, ToggleObliqueAction, tr("ToggleOblique is disabled"));
  DISABLECONTROL(VolumeRenderTool, VolumeRenderAction, tr("VolumeRender is disabled"));
  DISABLECONTROL(ToggleCoronalTool, ToggleCoronalAction, tr("ToggleCoronal is disabled"));
  DISABLECONTROL(ToggleSagittalTool, ToggleSagittalAction, tr("ToggleSagittal is disabled"));
  DISABLECONTROL(ToggleAxialTool, ToggleAxialAction, tr("ToggleAxial is disabled"));
  DISABLECONTROL(ShowAsMovieTool, ShowAsMovieAction, tr("ShowAsMovie is disabled"));
  DISABLECONTROL(ToggleColorBarTool, ToggleColorBarAction, tr("ToggleColorBar is disabled"));
  DISABLECONTROL(CenterAtCursorTool, CenterAtCursorAction, tr("CenterAtCursor is disabled"));
  DISABLECONTROL(MoveProximalTool, MoveProximalAction, tr("MoveProximal is disabled (select a target to enable it)"));
  DISABLECONTROL(MoveDistalTool, MoveDistalAction, tr("MoveDistal is disabled (select a target to enable it)"));
  DISABLECONTROL(SaveToReportTool, SaveToReportAction, tr("Save to Report is disabled (select a target to enable it)"));
}

void capTools::enableToolButtons()
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  ENABLECONTROL(ToggleLumenTool, ToggleLumenAction, tr("Press to toggle lumen display"));
  ENABLECONTROL(ToggleWallTool, ToggleWallAction, tr("Press to toggle wall display"));
  ENABLECONTROL(MaxStenosisTool, MaxStenosisAction, tr("Press to snap to cross section of greatest stenosis"));
  ENABLECONTROL(MaxDilationTool, MaxDilationAction, tr("Press to snap to cross section of greatest dilation"));
  ENABLECONTROL(MaxRRTool, MaxRRAction, tr("Press to snap to cross section of greatest remodeling ratio"));
  ENABLECONTROL(MaxWTTool, MaxWTAction, tr("Press to snap to cross section of greatest wall thickness"));
  ENABLECONTROL(ToggleCornerTool, ToggleCornerAction, tr("Press to toggle corner annotations"));
  ENABLECONTROL(OptimizeSubvolumeTool, OptimizeSubvolumeAction, tr("Press to toggle widgets allowing optimization of subvolume"));
  ENABLECONTROL(ShapeIntensityTool, ShapeIntensityAction, tr("Press to optimize the gray level settings depicted in the imagery"));
  ENABLECONTROL(SaveViewSettingsTool, SaveViewSettingsAction, tr("Press to save view settings"));
  if (viewSettingsSaved) {
    ENABLECONTROL(RestoreViewSettingsTool, RestoreViewSettingsAction, tr("Press to restore view settings"));
  }
  else {
    DISABLECONTROL(RestoreViewSettingsTool, RestoreViewSettingsAction, tr("RestoreViewSettings is disabled"));
  }
  ENABLECONTROL(AnnotateTool, AnnotateAction, tr("Press to annotate"));
  ENABLECONTROL(MeasureTool, MeasureAction, tr("Press to make a manual caliper measurement"));
  ENABLECONTROL(MaxCalcTool, MaxCalcAction, tr("Press to snap to cross section of maximum calcification"));
  ENABLECONTROL(MaxLRNCTool, MaxLRNCAction, tr("Press to snap to cross section of maximum lipid core"));
  ENABLECONTROL(MaxMATXTool, MaxMATXAction, tr("Press to snap to cross section of maximum matrix"));
  ENABLECONTROL(MaxIPHTool, MaxIPHAction, tr("Press to snap to cross section of maximum hemorrhage"));
  ENABLECONTROL(MaxUlcTool, MaxUlcAction, tr("Press to snap to cross section of maximum ulceration"));
  ENABLECONTROL(MaxThrTool, MaxThrAction, tr("Press to snap to cross section of maximum thrombosis"));
  ENABLECONTROL(TargetPathSwitchTool, TargetPathSwitchAction, tr("Press to switch target path"));
  ENABLECONTROL(TargetCreateTool, TargetCreateAction, tr("Press to create a new target"));
  ENABLECONTROL(TargetDeleteTool, TargetDeleteAction, tr("Press to delete current target"));
  ENABLECONTROL(TargetModifyTool, TargetModifyAction, tr("Press to modify current target"));
  ENABLECONTROL(ToggleObliqueTool, ToggleObliqueAction, tr("Press to toggle slice viewer alignment"));
  ENABLECONTROL(VolumeRenderTool, VolumeRenderAction, tr("Press to toggle volume rendering"));
  ENABLECONTROL(ToggleCoronalTool, ToggleCoronalAction, tr("Press to toggle coronal plane"));
  ENABLECONTROL(ToggleSagittalTool, ToggleSagittalAction, tr("Press to toggle sagittal plane"));
  ENABLECONTROL(ToggleAxialTool, ToggleAxialAction, tr("Press to toggle axial plane"));
  ENABLECONTROL(ShowAsMovieTool, ShowAsMovieAction, tr("Press to show as movie"));
  ENABLECONTROL(ToggleColorBarTool, ToggleColorBarAction, tr("Press to toggle presentation of tissue characteristics"));
  ENABLECONTROL(CenterAtCursorTool, CenterAtCursorAction, tr("Press to center at cursor"));
  ENABLECONTROL(MoveProximalTool, MoveProximalAction, tr("Press to move proximally"));
  ENABLECONTROL(MoveDistalTool, MoveDistalAction, tr("Press to move distally"));
  ENABLECONTROL(SaveToReportTool, SaveToReportAction, tr("Press to save view as a key image"));
}

void capTools::disconnectMenuActions()
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  disconnect(ToggleLumenAction, SIGNAL(triggered()), this, SLOT(ToggleLumenNoArg()));
  disconnect(ToggleWallAction, SIGNAL(triggered()), this, SLOT(ToggleWallNoArg()));
  disconnect(MaxStenosisAction, SIGNAL(triggered()), this, SLOT(MaxStenosis()));
  disconnect(MaxDilationAction, SIGNAL(triggered()), this, SLOT(MaxDilation()));
  disconnect(MaxRRAction, SIGNAL(triggered()), this, SLOT(MaxRR()));
  disconnect(MaxWTAction, SIGNAL(triggered()), this, SLOT(MaxWT()));
  disconnect(ToggleCornerAction, SIGNAL(triggered()), this, SLOT(ToggleCornerNoArg()));
  disconnect(OptimizeSubvolumeAction, SIGNAL(triggered()), this, SLOT(OptimizeSubvolumeNoArg()));
  disconnect(ShapeIntensityAction, SIGNAL(triggered()), this, SLOT(ShapeIntensity()));
  disconnect(SaveViewSettingsAction, SIGNAL(triggered()), this, SLOT(SaveViewSettings()));
  disconnect(RestoreViewSettingsAction, SIGNAL(triggered()), this, SLOT(RestoreViewSettings()));
  disconnect(AnnotateAction, SIGNAL(triggered()), this, SLOT(AnnotateNoArg()));
  disconnect(MeasureAction, SIGNAL(triggered()), this, SLOT(MeasureNoArg()));
  disconnect(MaxCalcAction, SIGNAL(triggered()), this, SLOT(MaxCalc()));
  disconnect(MaxLRNCAction, SIGNAL(triggered()), this, SLOT(MaxLRNC()));
  disconnect(MaxMATXAction, SIGNAL(triggered()), this, SLOT(MaxMATX()));
  disconnect(MaxIPHAction, SIGNAL(triggered()), this, SLOT(MaxIPH()));
  disconnect(MaxUlcAction, SIGNAL(triggered()), this, SLOT(MaxUlc()));
  disconnect(MaxThrAction, SIGNAL(triggered()), this, SLOT(MaxThr()));
  disconnect(TargetPathSwitchAction, SIGNAL(triggered()), this, SLOT(TargetPathSwitch()));
  disconnect(TargetCreateAction, SIGNAL(triggered()), this, SLOT(TargetCreateNoArg()));
  disconnect(TargetDeleteAction, SIGNAL(triggered()), this, SLOT(TargetDelete()));
  disconnect(TargetModifyAction, SIGNAL(triggered()), this, SLOT(TargetModify()));
  disconnect(ToggleObliqueAction, SIGNAL(triggered()), this, SLOT(ToggleObliqueNoArg()));
  disconnect(VolumeRenderAction, SIGNAL(triggered()), this, SLOT(VolumeRenderNoArg()));
  disconnect(ToggleCoronalAction, SIGNAL(triggered()), this, SLOT(ToggleCoronalNoArg()));
  disconnect(ToggleSagittalAction, SIGNAL(triggered()), this, SLOT(ToggleSagittalNoArg()));
  disconnect(ToggleAxialAction, SIGNAL(triggered()), this, SLOT(ToggleAxialNoArg()));
  disconnect(ShowAsMovieAction, SIGNAL(triggered()), this, SLOT(ShowAsMovie()));
  disconnect(ToggleColorBarAction, SIGNAL(triggered()), this, SLOT(ToggleColorBarNoArg()));
  disconnect(CenterAtCursorAction, SIGNAL(triggered()), this, SLOT(CenterAtCursor()));
  disconnect(MoveProximalAction, SIGNAL(triggered()), this, SLOT(MoveProximal()));
  disconnect(MoveDistalAction, SIGNAL(triggered()), this, SLOT(MoveDistal()));
  disconnect(SaveToReportAction, SIGNAL(triggered()), this, SLOT(SaveToReport()));
}

void capTools::connectMenuActions()
{
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  connect(ToggleLumenAction, SIGNAL(triggered()), this, SLOT(ToggleLumenNoArg()));
  connect(ToggleWallAction, SIGNAL(triggered()), this, SLOT(ToggleWallNoArg()));
  connect(MaxStenosisAction, SIGNAL(triggered()), this, SLOT(MaxStenosis()));
  connect(MaxDilationAction, SIGNAL(triggered()), this, SLOT(MaxDilation()));
  connect(MaxRRAction, SIGNAL(triggered()), this, SLOT(MaxRR()));
  connect(MaxWTAction, SIGNAL(triggered()), this, SLOT(MaxWT()));
  connect(ToggleCornerAction, SIGNAL(triggered()), this, SLOT(ToggleCornerNoArg()));
  connect(OptimizeSubvolumeAction, SIGNAL(triggered()), this, SLOT(OptimizeSubvolumeNoArg()));
  connect(ShapeIntensityAction, SIGNAL(triggered()), this, SLOT(ShapeIntensity()));
  connect(SaveViewSettingsAction, SIGNAL(triggered()), this, SLOT(SaveViewSettings()));
  connect(RestoreViewSettingsAction, SIGNAL(triggered()), this, SLOT(RestoreViewSettings()));
  connect(AnnotateAction, SIGNAL(triggered()), this, SLOT(AnnotateNoArg()));
  connect(MeasureAction, SIGNAL(triggered()), this, SLOT(MeasureNoArg()));
  connect(MaxCalcAction, SIGNAL(triggered()), this, SLOT(MaxCalc()));
  connect(MaxLRNCAction, SIGNAL(triggered()), this, SLOT(MaxLRNC()));
  connect(MaxMATXAction, SIGNAL(triggered()), this, SLOT(MaxMATX()));
  connect(MaxIPHAction, SIGNAL(triggered()), this, SLOT(MaxIPH()));
  connect(MaxUlcAction, SIGNAL(triggered()), this, SLOT(MaxUlc()));
  connect(MaxThrAction, SIGNAL(triggered()), this, SLOT(MaxThr()));
  connect(TargetPathSwitchAction, SIGNAL(triggered()), this, SLOT(TargetPathSwitch()));
  connect(TargetCreateAction, SIGNAL(triggered()), this, SLOT(TargetCreateNoArg()));
  connect(TargetDeleteAction, SIGNAL(triggered()), this, SLOT(TargetDelete()));
  connect(TargetModifyAction, SIGNAL(triggered()), this, SLOT(TargetModify()));
  connect(ToggleObliqueAction, SIGNAL(triggered()), this, SLOT(ToggleObliqueNoArg()));
  connect(VolumeRenderAction, SIGNAL(triggered()), this, SLOT(VolumeRenderNoArg()));
  connect(ToggleCoronalAction, SIGNAL(triggered()), this, SLOT(ToggleCoronalNoArg()));
  connect(ToggleSagittalAction, SIGNAL(triggered()), this, SLOT(ToggleSagittalNoArg()));
  connect(ToggleAxialAction, SIGNAL(triggered()), this, SLOT(ToggleAxialNoArg()));
  connect(ShowAsMovieAction, SIGNAL(triggered()), this, SLOT(ShowAsMovie()));
  connect(ToggleColorBarAction, SIGNAL(triggered()), this, SLOT(ToggleColorBarNoArg()));
  connect(CenterAtCursorAction, SIGNAL(triggered()), this, SLOT(CenterAtCursor()));
  connect(MoveProximalAction, SIGNAL(triggered()), this, SLOT(MoveProximal()));
  connect(MoveDistalAction, SIGNAL(triggered()), this, SLOT(MoveDistal()));
  connect(SaveToReportAction, SIGNAL(triggered()), this, SLOT(SaveToReport()));
}

void capTools::ToggleLumen(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->getTargetDefine(sessionItemIndex)->toggleSegmentation("lumen", checked);
  owner->getPatientAnalyze(sessionItemIndex)->toggleSegmentation("lumen", checked);
}

void capTools::ToggleWall(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->getTargetDefine(sessionItemIndex)->toggleSegmentation("wall", checked);
  owner->getPatientAnalyze(sessionItemIndex)->toggleSegmentation("wall", checked);
}

void capTools::MaxStenosis()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  //if (viewers->GetScene()->GetSelectedVesselTarget()->GetVesselTarget()->HasTag("VesselTargetMaxStenosisByArea"))
  bool foundtag = owner->getPatientAnalyze(sessionItemIndex)->jumpTo("VesselTargetMaxStenosisByArea"); 
  if (!foundtag) {
    QMessageBox msg;
    msg.setText(tr("MaxStenosis tag not present."));
    msg.exec();
  }
}

void capTools::MaxDilation()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  //if (viewers->GetScene()->GetSelectedVesselTarget()->GetVesselTarget()->HasTag("VesselTargetMaxDilationByArea"))
  bool foundtag = owner->getPatientAnalyze(sessionItemIndex)->jumpTo("VesselTargetMaxDilationByArea"); 
  if (!foundtag) {
    QMessageBox msg;
    msg.setText(tr("MaxDilation tag not present."));
    msg.exec();
  }
}

void capTools::MaxRR()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  //if (viewers->GetScene()->GetSelectedVesselTarget()->GetVesselTarget()->HasTag("VesselTargetMaxRemodelingRatio"))
  bool foundtag = owner->getPatientAnalyze(sessionItemIndex)->jumpTo("VesselTargetMaxRemodelingRatio"); 
  if (!foundtag) {
    QMessageBox msg;
    msg.setText(tr("MaxRemodelingRatio tag not present."));
    msg.exec();
  }
}

void capTools::MaxWT()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  //if (viewers->GetScene()->GetSelectedVesselTarget()->GetVesselTarget()->HasTag("VesselTargetMaxMaxWallThickness"))
  bool foundtag = owner->getPatientAnalyze(sessionItemIndex)->jumpTo("VesselTargetMaxMaxWallThickness"); 
  if (!foundtag) {
    QMessageBox msg;
    msg.setText(tr("MaxWallThickness tag not present."));
    msg.exec();
  }
}

void capTools::ToggleCorner(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->getPatientAnalyze(sessionItemIndex)->toggleCorner(checked);
}

void capTools::OptimizeSubvolume(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (viewerType == ebvViewer::THREED) {
    viewers->SetSubVolumeWidgetsEnabled(checked);
    viewers->SyncViewersToScene();
    viewers->Render();
  }
}

void capTools::ShapeIntensity()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  viewers->GetScene()->AutoWindowLevel();
  viewers->SyncViewersToScene();
  viewers->Render();
}

void capTools::SaveViewSettings()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  cursorPosition = viewers->GetScene()->GetCursorPosition();
  cameraParams = viewers->GetViewer(viewerId)->GetCameraParams();
  viewers->GetScene()->GetSelectedImage4()->GetWindowLevel(window, level);

  // now that we have saved settings, can enable the restore
  viewSettingsSaved = true;
  ENABLECONTROL(RestoreViewSettingsTool, RestoreViewSettingsAction, tr("Press to restore view settings"));
}

void capTools::RestoreViewSettings()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  viewers->GetScene()->SetCursorPosition(cursorPosition);
  viewers->UpdateCameras();
  viewers->GetViewer(viewerId)->SetCameraParams(cameraParams);
  viewers->GetScene()->GetSelectedImage4()->SetWindowLevel(window, level);
  viewers->SyncViewersToScene();
  viewers->Render();
}

void capTools::Annotate(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QMessageBox msg;
  msg.setText(tr("This feature is staged for implementation in a subsequent release."));
  msg.exec();
  /*qDebug() << "capTools::_AnnotateTool, checked is" << checked;
  if (checked) {

    //  NOTE: probably better to use an event filter rather than this, as while disableds the events are queued 
    // up which then all run when the enable takes place later
    viewers->GetViewer(id3d)->Disable();

    // here is where the annotation mode is enabled
  }
  else {
    viewers->GetViewer(id3d)->Enable();

    // turn off anything associated with the annotation mode
  }*/
}

void capTools::AnnotateContextMenu(QPoint pos)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QAction *anAct = new QAction(tr("an action"), this);
  QAction *anotherAct = new QAction(tr("another action"), this);
  QMenu menu(this);
  menu.addAction(anAct);
  connect(anAct, SIGNAL(triggered()), this, SLOT(anAction()));
  menu.addAction(anotherAct);
  connect(anotherAct, SIGNAL(triggered()), this, SLOT(anotherAction()));
  menu.exec(AnnotateTool->mapToGlobal(pos));
}

void capTools::anAction()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QMessageBox msg;
  msg.setText(tr("This feature is staged for implementation in a subsequent release."));
  msg.exec();
}

void capTools::anotherAction()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QMessageBox msg;
  msg.setText(tr("This feature is staged for implementation in a subsequent release."));
  msg.exec();
}

void capTools::Measure(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (checked)
    viewers->GetViewer2D(viewerId)->StartDistanceWidget();
  else
    viewers->GetViewer2D(viewerId)->StopDistanceWidget();
}

void capTools::MaxCalc()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  //if (viewers->GetScene()->GetSelectedVesselTarget()->GetVesselTarget()->HasTag("VesselTargetMaxCALCArea"))
  bool foundtag = owner->getPatientAnalyze(sessionItemIndex)->jumpTo("VesselTargetMaxCALCArea"); 
  if (!foundtag) {
    QMessageBox msg;
    msg.setText(tr("MaxCALCArea tag not present."));
    msg.exec();
  }
}

void capTools::MaxLRNC()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  //if (viewers->GetScene()->GetSelectedVesselTarget()->GetVesselTarget()->HasTag("VesselTargetMaxLRNCArea"))
  bool foundtag = owner->getPatientAnalyze(sessionItemIndex)->jumpTo("VesselTargetMaxLRNCArea"); 
  if (!foundtag) {
    QMessageBox msg;
    msg.setText(tr("MaxLRNCArea tag not present."));
    msg.exec();
  }
}

void capTools::MaxMATX()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  //if (viewers->GetScene()->GetSelectedVesselTarget()->GetVesselTarget()->HasTag("VesselTargetMaxMATXArea"))
  bool foundtag = owner->getPatientAnalyze(sessionItemIndex)->jumpTo("VesselTargetMaxMATXArea"); 
  if (!foundtag) {
    QMessageBox msg;
    msg.setText(tr("MaxMATXArea tag not present."));
    msg.exec();
  }
}

void capTools::MaxIPH()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  //if (viewers->GetScene()->GetSelectedVesselTarget()->GetVesselTarget()->HasTag("VesselTargetMaxIPHArea"))
  bool foundtag = owner->getPatientAnalyze(sessionItemIndex)->jumpTo("VesselTargetMaxIPHArea"); 
  if (!foundtag) {
    QMessageBox msg;
    msg.setText(tr("MaxIPHArea tag not present."));
    msg.exec();
  }
}

void capTools::MaxUlc()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QMessageBox msg;
  msg.setText(tr("Development of this feature is staged for a subsequent release."));
  msg.exec();
}

void capTools::MaxThr()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QMessageBox msg;
  msg.setText(tr("Development of this feature is staged for a subsequent release."));
  msg.exec();
}

void capTools::TargetPathSwitch()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // update buttons buttons now that there is a specific target in focus
  ENABLECONTROL(TargetDeleteTool, TargetDeleteAction, tr("Press to delete current target"));
  
  bool ret = true;
  if (owner->getWorkItemProcess(sessionItemIndex)->currentIndex() == PATIENTANALYZE) {
    ret = owner->getPatientAnalyze(sessionItemIndex)->switchTargetPath();
  }
  else if (owner->getWorkItemProcess(sessionItemIndex)->currentIndex() == TARGETDEFINE) {
    ret = owner->getTargetDefine(sessionItemIndex)->switchTargetPath();
  }
  if (ret) {
    ENABLECONTROL(TargetModifyTool, TargetModifyAction, tr("Press to modify current target"));
    ENABLECONTROL(MoveProximalTool, MoveProximalAction, tr("Press to move proximally"));
    ENABLECONTROL(MoveDistalTool, MoveDistalAction, tr("Press to move distally"));

    // move up and jog back, for two purposes: one, it has the effect of moving the cursors to the selected path; also, it provides a user visible action 
    // which indicates that the switch has been processed even if there is no other path to go to, i.e., where the switch has no net effect
    viewers->GetScene()->IncrCurrentCrossSectionSetCursor(1); 
    viewers->GetScene()->IncrCurrentCrossSectionSetCursor(-1);
    viewers->UpdateCameras();

    // finally, center at the new cursor
    viewers->PanCamerasToCursor(true, false);
    viewers->SyncViewersToScene();
    viewers->Render();
  }
  else
    owner->getTargetDefine(sessionItemIndex)->setDefiningRootFlag(true); 
}

void capTools::TargetCreate(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (owner->getPatientAnalyze(sessionItemIndex)->markingLesion()) {
    TargetCreateTool->setChecked(true); // don't consider it unchecked until user follows through all the way in naming the lesion; they might back out
    ENABLECONTROL(TargetCreateTool, TargetCreateAction, tr("Press to end point collection"));
    owner->getPatientAnalyze(sessionItemIndex)->labelMarkedLesion();
    owner->getPatientAnalyze(sessionItemIndex)->setMarkingLesionFlag(false);
  }
  else if (checked) {
    owner->getTargetDefine(sessionItemIndex)->setDefiningRootFlag(true); // TEMPORARY, UNTIL CAN DEFINE LONG PATHS IN CAPLIB
    TargetCreateTool->setChecked(false); // don't consider it checked until user follows through all the way to adding a vessel; they might back out
    ENABLECONTROL(TargetCreateTool, TargetCreateAction, tr("Press to create a new target"));

    QMenu menu(this);
    QList<QString> allowableBodySites = QList<QString>()
             << tr("Aorta")
             << tr("LeftCarotid")
             << tr("RightCarotid")
             << tr("LeftCoronary")
             << tr("RightCoronary")
             << tr("LeftFemoral")
             << tr("RightFemoral")
             << tr("LeftVertebral")
             << tr("RightVertebral")
             << tr("NotSpecified")
            ;
            
    foreach (QString bodySite, allowableBodySites) {
      QVariant bodySiteParam;
      QString actionString;
      bodySiteParam.setValue(bodySite);
      actionString = tr("New ");
      actionString.append(bodySite);
      QAction *siteAction = new QAction(actionString, this);
      siteAction->setData(bodySiteParam);
      menu.addAction(siteAction);
      connect(siteAction, SIGNAL(triggered()), this, SLOT(TargetCreateWithSpec()));
    }
    if (menu.exec(QCursor::pos()) == 0) {
      // if the menu returned 0, the user wants to back out.  it is not too late for them to do so; just reset flag in this case
      owner->getTargetDefine(sessionItemIndex)->setDefiningRootFlag(false); // TEMPORARY, UNTIL CAN DEFINE LONG PATHS IN CAPLIB
    }
  }
  else if (owner->getTargetDefine(sessionItemIndex)->definingRoot()) {
    // TEMPORARY, UNTIL CAN DEFINE LONG PATHS IN CAPLIB
    owner->getTargetDefine(sessionItemIndex)->labelDistalVessel("");
    owner->getTargetDefine(sessionItemIndex)->setDefiningRootFlag(false);
    VolumeRender(false);
  }
  else {
    owner->getTargetDefine(sessionItemIndex)->completeDistalVessel();
    VolumeRender(false);
  }
}

void capTools::TargetCreateWithSpec()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QAction *act = qobject_cast<QAction *>(sender());
  if (act != 0) {
    owner->getTargetDefine(sessionItemIndex)->establishNewTarget(act->data().toString());
    owner->getTargetDefine(sessionItemIndex)->createTargetPre(act->data().toString());
  }
  else 
    qWarning() << "capTools::TargetCreateWithSpec invoked by act=0";
}

void capTools::TargetDelete()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QMessageBox msgBox(this);
  msgBox.setText(tr("Deleting a target will remove its analysis from the session, and cannot be undone."));
  msgBox.setInformativeText(tr("Do you want to proceed to delete?"));
  msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
  int ret = msgBox.exec();
  if (ret == QMessageBox::No)
    return;
  else {
    owner->getTargetDefine(sessionItemIndex)->deleteTarget(); // removes displays, frees memory, and removes from lists
  }
}

void capTools::TargetModify()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QMenu menu(this);

  QAction *addDistalVessel = new QAction(tr("Add Distal Vessel"), this);
  menu.addAction(addDistalVessel);
  connect(addDistalVessel, SIGNAL(triggered()), this, SLOT(AddDistalVessel()));
  addDistalVessel->setEnabled(!viewers->SegmentationEditorActive());

  QAction *evaluateLumen = new QAction(tr("Evaluate Lumen"), this);
  menu.addAction(evaluateLumen);
  connect(evaluateLumen, SIGNAL(triggered()), this, SLOT(EvaluateLumen()));
  evaluateLumen->setEnabled(!viewers->SegmentationEditorActive());

  QAction *evaluateWall = new QAction(tr("Evaluate Wall"), this);
  menu.addAction(evaluateWall);
  connect(evaluateWall, SIGNAL(triggered()), this, SLOT(EvaluateWall()));
  evaluateWall->setEnabled(!viewers->SegmentationEditorActive());

  QAction *markLesion = new QAction(markLesionActionLabel, this);
  menu.addAction(markLesion);
  connect(markLesion, SIGNAL(triggered()), this, SLOT(MarkLesion()));
  markLesion->setEnabled(markLesionEnableState);

  menu.addSeparator(); // set off the edit tools with a separator
  QAction *editSegmentation = new QAction(editSegmentationActionLabel, this); // unlike the others, this one needs its pointer stored so that the owner can change its state and labelling based on context
  menu.addAction(editSegmentation);
  connect(editSegmentation, SIGNAL(triggered()), this, SLOT(EditSegmentation()));
  editSegmentation->setEnabled(editSegmentationEnableState);

  QAction *resetWall = new QAction(tr("Reset Wall"), this);
  menu.addAction(resetWall);
  connect(resetWall, SIGNAL(triggered()), this, SLOT(ResetWall()));
  resetWall->setEnabled(!viewers->SegmentationEditorActive());

  QAction *clearLesions = new QAction(clearLesionsActionLabel, this);
  menu.addAction(clearLesions);
  connect(clearLesions, SIGNAL(triggered()), this, SLOT(ClearLesions()));
  clearLesions->setEnabled(clearLesionsEnableState);

  QAction *resetTarget = new QAction(tr("Reset Target"), this);
  menu.addAction(resetTarget);
  connect(resetTarget, SIGNAL(triggered()), this, SLOT(ResetTarget()));
  resetTarget->setEnabled(!viewers->SegmentationEditorActive());

  menu.exec(QCursor::pos());
}

void capTools::AddDistalVessel()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
  owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
  owner->getTargetDefine(sessionItemIndex)->addDistalVessel();
}

void capTools::MarkLesion()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->getTargetDefine(sessionItemIndex)->giveToAnalyzeIfNotAlreadyThere();
  owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(PATIENTANALYZE);
  owner->getPatientAnalyze(sessionItemIndex)->setMarkingLesionFlag(true);
  owner->getPatientAnalyze(sessionItemIndex)->markLesion();
}

void capTools::ResetWall()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
  owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
  owner->getTargetDefine(sessionItemIndex)->resetWall();
}
 
void capTools::ClearLesions()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->getTargetDefine(sessionItemIndex)->giveToAnalyzeIfNotAlreadyThere();
  owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(PATIENTANALYZE);
  owner->getPatientAnalyze(sessionItemIndex)->clearLesions();
}

void capTools::ResetTarget()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QMessageBox msgBox(this);
  msgBox.setText(tr("Resetting a target will remove its analysis from the session and require a new initializer."));
  msgBox.setInformativeText(tr("Do you want to proceed to reset?"));
  msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
  msgBox.setDefaultButton(QMessageBox::No);
  msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
  int ret = msgBox.exec();
  if (ret == QMessageBox::No) {
    return;
  }
  else {
    owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
    owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
    owner->getTargetDefine(sessionItemIndex)->resetTarget();
    owner->getTargetDefine(sessionItemIndex)->setDefiningRootFlag(true);
  }
}
 
void capTools::EvaluateLumen()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
  owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
  owner->getTargetDefine(sessionItemIndex)->evaluateLumen();
}
  
void capTools::EvaluateWall()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
  owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
  owner->getTargetDefine(sessionItemIndex)->evaluateWall();
}
  
void capTools::EditSegmentation()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  bool alreadyOn = viewers->SegmentationEditorActive();
  owner->getPatientAnalyze(sessionItemIndex)->giveToDefineIfNotAlreadyThere();
  owner->getWorkItemProcess(sessionItemIndex)->setCurrentIndex(TARGETDEFINE);
  if (!alreadyOn) {
    QMessageBox msgBox(this);
    msgBox.setText(tr("Editing segmentation(s) will remove downstream computations."));
    msgBox.setInformativeText(tr("Do you want to proceed to edit?"));
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    msgBox.setWindowFlags(msgBox.windowFlags()&~Qt::WindowContextHelpButtonHint|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
    int ret = msgBox.exec();
    if (ret == QMessageBox::No) {
      return;
    }
    owner->getTargetDefine(sessionItemIndex)->editSegmentation(true);
  }
  else {
    owner->getTargetDefine(sessionItemIndex)->editSegmentation(false);
  }
}

void capTools::ToggleOblique(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (owner->getWorkItemProcess(sessionItemIndex)->currentIndex() == PATIENTANALYZE) {
    owner->getPatientAnalyze(sessionItemIndex)->toggleOblique(checked);
  }
  else if (owner->getWorkItemProcess(sessionItemIndex)->currentIndex() == TARGETDEFINE) {
    owner->getTargetDefine(sessionItemIndex)->toggleOblique(checked);
  }
}

void capTools::VolumeRender(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // no-op if not a 3D viewer 
  if (viewerType == ebvViewer::THREED) {
    if (!checked) {
      ToggleCoronal(true); ToggleCoronalTool->setChecked(true);
      ToggleSagittal(true); ToggleSagittalTool->setChecked(true);
      ToggleAxial(true); ToggleAxialTool->setChecked(true);
    }
    viewers->GetViewer3D(viewerId)->SetShowVolume(checked);
    if (ToggleObliqueTool->isChecked()) {
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::OBLIQUE, !checked);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::OBLIQUEPERP1, !checked);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::CPR, !checked);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::AXIAL, false);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::CORONAL, false);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::SAGITTAL, false);
    }
    else {
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::AXIAL, !checked);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::CORONAL, !checked);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::SAGITTAL, !checked);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::OBLIQUE, false);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::CPR, false);
    }

    viewers->SyncViewersToScene();
    viewers->Render();
  }
}

void capTools::ToggleCoronal(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // no-op if not a 3D viewer 
  if (viewerType == ebvViewer::THREED) {
    if (ToggleObliqueTool->isChecked()) {
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::OBLIQUEPERP1, checked);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::CORONAL, false);
    }
    else {
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::CORONAL, checked);
    }

    viewers->SyncViewersToScene();
    viewers->Render();
  }
}

void capTools::ToggleSagittal(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // no-op if not a 3D viewer 
  if (viewerType == ebvViewer::THREED) {
    if (ToggleObliqueTool->isChecked()) {
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::CPR, checked);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::SAGITTAL, false);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::OBLIQUEPERP2, false);
    }
    else {
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::CPR, false);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::SAGITTAL, checked);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::OBLIQUEPERP2, false);
    }
    
    viewers->SyncViewersToScene();
    viewers->Render();
  }
}

void capTools::ToggleAxial(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  // no-op if not a 3D viewer 
  if (viewerType == ebvViewer::THREED) {
    if (ToggleObliqueTool->isChecked()) {
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::OBLIQUE, checked);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::AXIAL, false);
    }
    else {
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::OBLIQUE, false);
      viewers->GetViewer3D(viewerId)->SetShowSlice(ebvViewer::AXIAL, checked);
    }
    
    viewers->SyncViewersToScene();
    viewers->Render();
  }
}

void capTools::ShowAsMovie()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  QMessageBox msg;
  msg.setText(tr("This feature is staged for implementation in a subsequent release."));
  msg.exec();
}

void capTools::ToggleColorBar(bool checked)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->getPatientAnalyze(sessionItemIndex)->toggleColorBar(checked);
}

void capTools::CenterAtCursor()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  viewers->PanCamerasToCursor(true, true);
  viewers->Render();
}

void capTools::MoveProximal()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  viewers->GetScene()->IncrCurrentCrossSectionSetCursor(-1);
  viewers->UpdateCameras();
  viewers->SyncViewersToScene();
  viewers->PanCamerasToCursor(true, false);
  viewers->Render();
}

void capTools::MoveDistal()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  viewers->GetScene()->IncrCurrentCrossSectionSetCursor(1);
  viewers->UpdateCameras();
  viewers->SyncViewersToScene();
  viewers->PanCamerasToCursor(true, false);
  viewers->Render();
}
 
void capTools::SaveToReport()
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  owner->getPatientAnalyze(sessionItemIndex)->saveToReport(viewerType, /*forceSliceViewerForTarget*/false, /*includeReportUpdate*/true);
}

/** @} */
