// Copyright (c) Elucid Bioimaging

#ifndef PROCESSINGPARAMETERS_H
#define PROCESSINGPARAMETERS_H

#include "workItemListFileToken.h"
#include "ebiVesselPipeline.h"
#include "systemPreferences.h"
#include "ebLog.h"

#include <QWidget>
#include <QString>
#include <QPushButton>
#include <QInputDialog>
#include <QList>
#include <QLabel>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QErrorMessage>
#include <QSlider>
#include <QStyle>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QStylePainter>
#include <QStyleOptionSlider>
#include <QApplication>
#include <QHeaderView>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QJsonObject>
#include <QJsonDocument>

#include <string>
#include <vector>
//#include <string.h>
#include <iostream>

/**
 * @{ 
 ** 
 * @brief class stageParameters provides functionality for user review and selection of processing parameters asociated with analysis of targets 
 *
 *  \copyright Elucid Bioimaging
 *  \ingroup targetDefine
 */
class stageParameters : public QWidget
{
  Q_OBJECT

public:
  explicit stageParameters() {
    stageName = "";
    parameterString = "";
    versionString = "";
    version = "";
    parameterKeys.clear();
    parameterPrompts.clear();
    settings.clear();
    settingEntries.clear();
  }
  ~stageParameters() { }
  QString stageName;
  std::string parameterString, versionString;
  QGroupBox *stageBox;
  QStringList versionSubString;
  QString version;
  QVBoxLayout *parameters;
  QByteArray byteArray;
  QJsonParseError err;
  QJsonDocument doc;
  QJsonObject list;
  QList<QString> parameterKeys;
  QList<QLabel *> parameterPrompts;
  QList<double> settings;
  QFormLayout *formLayout_5;
  QList<QLineEdit *> settingEntries;

  void readStageParameters(const QJsonObject &json) { 
    //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
    version = json[stageVersion_token].toString(); 
    parameterKeys.clear();
    settings.clear();
    list = json[stageSettings_token].toObject();
    foreach (const QString key, list.keys()) {
      parameterKeys << key;
      settings << list[key].toDouble(); 
    }
  }

  void writeStageParameters(QJsonObject &json) const { 
    //ebLog eblog(Q_FUNC_INFO); eblog << stageName.toStdString() << std::endl;
    json[stageVersion_token] = version; 
    QJsonObject settingsObject;
    for (int i=0; i < settings.size(); i++)
      settingsObject[parameterKeys.at(i)] = settings.at(i);
    json[stageSettings_token] = settingsObject;
  }

  void update(stageParameters *ostage)
  {
    ostage->stageName = stageName;
    ostage->parameterString = parameterString;
    ostage->versionString = versionString;
    ostage->version = version;
    for (int i = 0; i < parameterKeys.size(); i++) {
        if (i < ostage->parameterKeys.size())
            ostage->parameterKeys[i] = parameterKeys[i];
        else
            ostage->parameterKeys.append( parameterKeys[i] );
    }
  }
};
/** @} */

/**
 * @{ 
 ** 
 * @brief class CompositionControlSlider provides a specialized multiple-handle slider with spans 
 *  \copyright Elucid Bioimaging
 *  \ingroup targetDefine
 */
class CompositionControlSlider : public QSlider {
  Q_OBJECT

public:
  explicit CompositionControlSlider(QWidget* parent = 0);
  QLabel *LRNCLowerLimitLabel;
  QSpinBox *LRNCLowerLimitSpinBox;
  QLabel *LRNCUpperLimitLabel;
  QSpinBox *LRNCUpperLimitSpinBox;
  QLabel *CALCLowerLimitLabel;
  QSpinBox *CALCLowerLimitSpinBox;
  QLabel *CALCUpperLimitLabel;
  QSpinBox *CALCUpperLimitSpinBox;
  QList<int> HUvalueLUT;

  enum SpanHandle
  {
    NoHandle,

    LRNC_lowerHandle,
    LRNC_upperHandle,

    CALC_lowerHandle,
    CALC_upperHandle
  };

Q_SIGNALS:
  void valueChanged();

public Q_SLOTS:
  void movePressedHandle(); 
  void setLRNC_lowerValue(int value);
  void setLRNC_upperValue(int value);
  void setCALC_lowerValue(int value);
  void setCALC_upperValue(int value);
  
protected:
  virtual void mousePressEvent(QMouseEvent* event);
  virtual void mouseMoveEvent(QMouseEvent* event);
  virtual void mouseReleaseEvent(QMouseEvent* event);
  virtual void paintEvent(QPaintEvent* event);

private:
  void initStyleOption(QStyleOptionSlider* option, CompositionControlSlider::SpanHandle handle = CompositionControlSlider::LRNC_upperHandle) const;
  int pick(const QPoint& pt) const { return pt.x(); }
  int pixelPosToRangeValue(int pos) const;
  void drawHandle(QStylePainter* painter, CompositionControlSlider::SpanHandle handle) const;
  void drawSpan(QStylePainter* painter, const QRect& rect, QColor color) const;
  void triggerAction(QAbstractSlider::SliderAction action);
  void setLRNC_lowerPos(int pos, bool alsoChangeValue);
  void setLRNC_upperPos(int pos, bool alsoChangeValue);
  void setCALC_lowerPos(int pos, bool alsoChangeValue);
  void setCALC_upperPos(int pos, bool alsoChangeValue);

  int LRNC_lowerPos, LRNC_upperPos;
  int CALC_lowerPos, CALC_upperPos;
  int offset;
  int position;
  CompositionControlSlider::SpanHandle lastPressed;
  QStyle::SubControl LRNC_lowerPressed;
  QStyle::SubControl LRNC_upperPressed;
  QStyle::SubControl CALC_lowerPressed;
  QStyle::SubControl CALC_upperPressed;
  bool blockTracking;
};

/**
 * @{ 
 ** 
 * @brief class CompositionControl provides functionality for user review and selection of processing parameters asociated with analysis of targets 
 *  \copyright Elucid Bioimaging
 *  \ingroup targetDefine
 */
class CompositionControl : public QWidget
{
  Q_OBJECT

public:
  explicit CompositionControl(QWidget *p, QString bodySite);
  CompositionControlSlider *compositionControlSlider;
  QDialog *Dialog;
  QDialogButtonBox *buttonBox;

public slots:
  void acceptCompositionSettings();
  void cancelCompositionSetting();

private slots:
  void indicateThatChangeHasBeenMadeButNotYetProcessed();

private:
  bool eventFilter(QObject *obj, QEvent *event);
  QWidget *owner;
  QErrorMessage *message;
  bool oneOrMoreValuesChanged;

  QVBoxLayout *verticalLayout;
  QFormLayout *formLayout;
};
/** @} */

/**
 * \ingroup targetDefine
 * @{ 
 ** 
 * @brief class processingParameters provides functionality for user review and selection of processing parameters asociated with analysis of targets 
 */
class processingParameters : public QWidget
{
  Q_OBJECT

public:
  explicit processingParameters(QWidget *p, systemPreferences *prefObj, QString juris, ebiVesselPipeline::Pointer pline, QString b, ebID tgtPid);
  ~processingParameters() { ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; delete message; delete Dialog; }
  void presentDialog();
  void presentCompositionControl();
  void emitProcessCompositionSettingsChange();
  QWidget *owner;
  QList<stageParameters *> parametersByStages;
  CompositionControl *compositionControl;
  bool compositionControlCurrentlyInUse;

public slots:
  void acceptParameterSettings();
  void cancelParameterSetting();

signals:
  void processingParametersSettingsChanged();

private:
  QString bodySite;
  ebiVesselPipeline::Pointer pipeline;
  QDialog *Dialog;
  QGridLayout *gridLayout_3;
  QDialogButtonBox *buttonBox;
  ebID targetPipelineID;
  QErrorMessage *message;
  systemPreferences *systemPreferencesObject;
  QString clinicalJurisdiction;
  bool alreadyReceivedConfirmation;
  bool confirmParameterSettingChange();
  bool eventFilter(QObject *obj, QEvent *event);
};
/** @} */

#endif // PROCESSINGPARAMETERS_H
