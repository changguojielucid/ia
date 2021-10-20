// Copyright (c) Elucid Bioimaging
#ifndef SEGMENTATIONEDITOR_H
#define SEGMENTATIONEDITOR_H

#include "ebLog.h"
#include <QDialog>

class targetDefine;

/** @brief class SegmentationEditor tool palette during segmentation editing
 *
 *  \copyright Elucid Bioimaging
 *  \ingroup cap
 */

namespace Ui {
class SegmentationEditor;
}

class SegmentationEditor : public QDialog
{
  Q_OBJECT

public:
  explicit SegmentationEditor(QWidget *owner = 0);
  ~SegmentationEditor();

public slots:
  virtual void reject() override { }  // prevent user hitting 'ESC' to reject dialog
  void on_doneButton_clicked();
  
protected:
  virtual void closeEvent(QCloseEvent *event) override;
  virtual void showEvent(QShowEvent *event) override;
  void selectButton(QPushButton *button);  // set button checked, others unchecked
  void fillParameters(); // fill GUI parameters tab widgets from linked viewers widget parameter values
  
private:
  Ui::SegmentationEditor *ui;
  targetDefine *owner;

private slots:
  void on_paintBrushButton_clicked();
  void on_cullButton_clicked();
  void on_imageFillButton_clicked();
  void on_segmentationFillButton_clicked();
  void on_smoothEdgeButton_clicked();
  void on_sharpenEdgeButton_clicked();
  void on_findEdgeButton_clicked();
  void on_drawEdgeButton_clicked();
  void on_undoButton_clicked();
  void on_redoButton_clicked();
  void parameter_changed();
};

#endif // SEGMENTATIONEDITOR_H
