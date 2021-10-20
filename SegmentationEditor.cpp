// Copyright (c) Elucid Bioimaging

#include "SegmentationEditor.h"
#include "ui_SegmentationEditor.h"
#include "cap.h"
#include "targetDefine.h"
#include "ebvSegmentationEditorRepresentation.h"
#include "ebAssert.h"

// ------------------------------------------------------------------------------------
// public methods
// ------------------------------------------------------------------------------------

SegmentationEditor::SegmentationEditor(QWidget *p) :
    QDialog(p),
    ui(new Ui::SegmentationEditor)
{
  ebLog eblog(Q_FUNC_INFO);
  owner = dynamic_cast<targetDefine*>(p);

  // setup the form
  ui->setupUi(this);
  // tool buttons
  ui->paintBrushButton->setCheckable(true);
  ui->cullButton->setCheckable(true);
  ui->imageFillButton->setCheckable(true);
  ui->segmentationFillButton->setCheckable(true);
  ui->smoothEdgeButton->setCheckable(true);
  ui->sharpenEdgeButton->setCheckable(true);
  ui->findEdgeButton->setCheckable(true);
  ui->drawEdgeButton->setCheckable(true);
  selectButton(ui->paintBrushButton);  // default first tool 
  // parameter value validators
  ui->imageFillIntensityWindowLineEdit->setValidator(new QDoubleValidator(0,999999,10));
  ui->imageFillEdgeSoftnessLineEdit->setValidator(new QDoubleValidator(0,100,10));
  ui->smoothEdgeIterationsLineEdit->setValidator(new QIntValidator(1,50));
  ui->sharpenEdgeIterationsLineEdit->setValidator(new QIntValidator(1,50));
  ui->sharpenEdgeRadiusCutoffLineEdit->setValidator(new QDoubleValidator(0,100,10));
  ui->findEdgeIterationsLineEdit->setValidator(new QIntValidator(1,50));
  ui->findEdgeGradientSigmaLineEdit->setValidator(new QDoubleValidator(0,100,10));
  ui->findEdgeContrastLineEdit->setValidator(new QDoubleValidator(0,10000,10));
  ui->findEdgeSmoothingWeightLineEdit->setValidator(new QDoubleValidator(0,100,10));
  ui->drawEdgeIterationsLineEdit->setValidator(new QIntValidator(1,50));
  ui->drawEdgeSmoothingWeightLineEdit->setValidator(new QDoubleValidator(0,100,10));
  connect(ui->imageFillIntensityWindowLineEdit,SIGNAL(textEdited(const QString &)),this,SLOT(parameter_changed()));
  connect(ui->imageFillEdgeSoftnessLineEdit,SIGNAL(textEdited(const QString &)),this,SLOT(parameter_changed()));
  connect(ui->imageFillIntensityRangeComboBox,SIGNAL(activated(const QString &)),this,SLOT(parameter_changed()));
  connect(ui->smoothEdgeIterationsLineEdit,SIGNAL(textEdited(const QString &)),this,SLOT(parameter_changed()));
  connect(ui->sharpenEdgeIterationsLineEdit,SIGNAL(textEdited(const QString &)),this,SLOT(parameter_changed()));
  connect(ui->sharpenEdgeRadiusCutoffLineEdit,SIGNAL(textEdited(const QString &)),this,SLOT(parameter_changed()));
  connect(ui->findEdgeIterationsLineEdit,SIGNAL(textEdited(const QString &)),this,SLOT(parameter_changed()));
  connect(ui->findEdgeGradientSigmaLineEdit,SIGNAL(textEdited(const QString &)),this,SLOT(parameter_changed()));
  connect(ui->findEdgeContrastLineEdit,SIGNAL(textEdited(const QString &)),this,SLOT(parameter_changed()));
  connect(ui->findEdgeSmoothingWeightLineEdit,SIGNAL(textEdited(const QString &)),this,SLOT(parameter_changed()));
  connect(ui->drawEdgeIterationsLineEdit,SIGNAL(textEdited(const QString &)),this,SLOT(parameter_changed()));
  connect(ui->drawEdgeSmoothingWeightLineEdit,SIGNAL(textEdited(const QString &)),this,SLOT(parameter_changed()));

  // window
  setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
  move(50,50);
}

SegmentationEditor::~SegmentationEditor()
{
  hide();
  delete ui;
}

// ------------------------------------------------------------------------------------
// protected methods
// ------------------------------------------------------------------------------------

/*virtual*/ void SegmentationEditor::closeEvent(QCloseEvent *event) /*override*/ {
  ebLog eblog(Q_FUNC_INFO);
  if (owner)
    owner->editSegmentation(false);
  event->accept();
}

/*virtual*/ void SegmentationEditor::showEvent(QShowEvent *event) /*override*/ {
  ebLog eblog(Q_FUNC_INFO);
  ebAssert(event);
  ebAssert(ui);  
  try {
    selectButton(ui->paintBrushButton);  // default first tool
    fillParameters();
    eblog << "accepting event" << std::endl;
    event->accept();
  } catch (std::exception &e) {
    eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
  }
}

void SegmentationEditor::selectButton(QPushButton *button) {
  ebLog eblog(Q_FUNC_INFO);
  ebAssert(button);
  ebAssert(ui);
  ui->paintBrushButton->setChecked(false);
  ui->cullButton->setChecked(false);
  ui->imageFillButton->setChecked(false);
  ui->segmentationFillButton->setChecked(false);
  ui->smoothEdgeButton->setChecked(false);
  ui->sharpenEdgeButton->setChecked(false);
  ui->findEdgeButton->setChecked(false);
  ui->drawEdgeButton->setChecked(false);
  button->setChecked(true);
}

void SegmentationEditor::fillParameters() {
  ebLog eblog(Q_FUNC_INFO);
  ebAssert(owner);
  auto lv = owner->viewers;
  ebAssert(lv);
  ebAssert(ui);

  eblog << lv->GetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::ImageFillIntensityWindowParameter) << std::endl;

  ui->imageFillIntensityWindowLineEdit->setText(QString::number(lv->GetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::ImageFillIntensityWindowParameter)));
  ui->imageFillEdgeSoftnessLineEdit->setText(QString::number(lv->GetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::ImageFillEdgeSoftnessParameter)));

  eblog << (int)lv->GetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::ImageFillRangeTypeParameter) << std::endl;

  ui->imageFillIntensityRangeComboBox->setCurrentIndex((int)lv->GetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::ImageFillRangeTypeParameter));
  ui->smoothEdgeIterationsLineEdit->setText(QString::number(lv->GetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::SmoothEdgeIterationsParameter)));
  ui->sharpenEdgeIterationsLineEdit->setText(QString::number(lv->GetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::SharpenEdgeIterationsParameter)));
  ui->sharpenEdgeRadiusCutoffLineEdit->setText(QString::number(lv->GetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::SharpenEdgeRadiusOfCurvatureCutoffParameter)));
  ui->findEdgeIterationsLineEdit->setText(QString::number(lv->GetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::FindEdgeIterationsParameter)));
  ui->findEdgeGradientSigmaLineEdit->setText(QString::number(lv->GetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::FindEdgeGradientSigmaParameter)));
  ui->findEdgeContrastLineEdit->setText(QString::number(lv->GetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::FindEdgeContrastParameter)));
  ui->findEdgeSmoothingWeightLineEdit->setText(QString::number(lv->GetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::FindEdgeSmoothingWeightParameter)));
  ui->drawEdgeIterationsLineEdit->setText(QString::number(lv->GetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::DrawEdgeIterationsParameter)));
  ui->drawEdgeSmoothingWeightLineEdit->setText(QString::number(lv->GetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::DrawEdgeSmoothingWeightParameter)));  
}

// ------------------------------------------------------------------------------------
// private slots
// ------------------------------------------------------------------------------------

void SegmentationEditor::on_paintBrushButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO);
  if (!owner || !owner->viewers)  return;
  owner->viewers->SetSegmentationEditorTool(ebvSegmentationEditorRepresentation::PaintBrush);
  selectButton(ui->paintBrushButton);
}

void SegmentationEditor::on_cullButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO);
  if (!owner || !owner->viewers)  return;
  owner->viewers->SetSegmentationEditorTool(ebvSegmentationEditorRepresentation::Cull);
  selectButton(ui->cullButton);
}

void SegmentationEditor::on_imageFillButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO);
  if (!owner || !owner->viewers)  return;
  owner->viewers->SetSegmentationEditorTool(ebvSegmentationEditorRepresentation::ImageFill);
  selectButton(ui->imageFillButton);
}

/*
SegmentationMode::GetMessage(const& String)
{

  CEVLinkedViewer* pViewer = GetLinkerViewer()

  swithc()
  {
    case SEGMODE_MSG_FILLBTN_CLICK;
      GetLinkerViewer()->SetSegmentationEditorTool(ebvSegmentationEditorRepresentation::SegmentationFill);

  }
  
}
*/
void SegmentationEditor::on_segmentationFillButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO);
  if (!owner || !owner->viewers)  return;
  owner->viewers->SetSegmentationEditorTool(ebvSegmentationEditorRepresentation::SegmentationFill);
  selectButton(ui->segmentationFillButton);
}

void SegmentationEditor::on_smoothEdgeButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO);
  if (!owner || !owner->viewers)  return;
  owner->viewers->SetSegmentationEditorTool(ebvSegmentationEditorRepresentation::SmoothEdge);
  selectButton(ui->smoothEdgeButton);
}

void SegmentationEditor::on_sharpenEdgeButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO);
  if (!owner || !owner->viewers)  return;
  owner->viewers->SetSegmentationEditorTool(ebvSegmentationEditorRepresentation::SharpenEdge);
  selectButton(ui->sharpenEdgeButton);
}

void SegmentationEditor::on_findEdgeButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO);
  if (!owner || !owner->viewers)  return;
  owner->viewers->SetSegmentationEditorTool(ebvSegmentationEditorRepresentation::FindEdge);
  selectButton(ui->findEdgeButton);
}

void SegmentationEditor::on_drawEdgeButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO);
  if (!owner || !owner->viewers)  return;
  owner->viewers->SetSegmentationEditorTool(ebvSegmentationEditorRepresentation::DrawEdge);
  selectButton(ui->drawEdgeButton);
}

void SegmentationEditor::on_undoButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO);
  if (!owner || !owner->viewers)  return;
  owner->viewers->UndoSegmentationEditor(-1);
}

void SegmentationEditor::on_redoButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO);
  if (!owner || !owner->viewers)  return;
  owner->viewers->UndoSegmentationEditor(1);
}

void SegmentationEditor::on_doneButton_clicked()
{
  ebLog eblog(Q_FUNC_INFO);
  if (owner)
    owner->editSegmentation(false);
}

void SegmentationEditor::parameter_changed() {
  ebAssert(owner);
  auto lv = owner->viewers;
  ebAssert(lv);
  lv->SetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::ImageFillIntensityWindowParameter,ui->imageFillIntensityWindowLineEdit->text().toDouble());
  lv->SetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::ImageFillEdgeSoftnessParameter,ui->imageFillEdgeSoftnessLineEdit->text().toDouble());
  lv->SetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::ImageFillRangeTypeParameter,ui->imageFillIntensityRangeComboBox->currentIndex());
  lv->SetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::SmoothEdgeIterationsParameter,ui->smoothEdgeIterationsLineEdit->text().toDouble());
  lv->SetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::SharpenEdgeIterationsParameter,ui->sharpenEdgeIterationsLineEdit->text().toDouble());
  lv->SetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::SharpenEdgeRadiusOfCurvatureCutoffParameter,ui->sharpenEdgeRadiusCutoffLineEdit->text().toDouble());
  lv->SetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::FindEdgeIterationsParameter,ui->findEdgeIterationsLineEdit->text().toDouble());
  lv->SetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::FindEdgeGradientSigmaParameter,ui->findEdgeGradientSigmaLineEdit->text().toDouble());
  lv->SetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::FindEdgeContrastParameter,ui->findEdgeContrastLineEdit->text().toDouble());
  lv->SetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::FindEdgeSmoothingWeightParameter,ui->findEdgeSmoothingWeightLineEdit->text().toDouble());
  lv->SetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::DrawEdgeIterationsParameter,ui->drawEdgeIterationsLineEdit->text().toDouble());
  lv->SetSegmentationEditorParameter(ebvSegmentationEditorRepresentation::DrawEdgeSmoothingWeightParameter,ui->drawEdgeSmoothingWeightLineEdit->text().toDouble());
}
