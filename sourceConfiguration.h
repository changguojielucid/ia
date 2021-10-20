// Copyright (c) Elucid Bioimaging
#ifndef SOURCECONFIGURATION_H
#define SOURCECONFIGURATION_H

#include "ebLog.h"

#include <QDialog>
#include <QStandardPaths>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFileInfo>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QMetaType>

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#define startingFolder_token "startingFolder"
#define serverAETitle_token "serverAETitle"
#define serverHost_token "serverHost"
#define serverPort_token "serverPort"
#define localAETitle_token "localAETitle"
#define localPort_token "localPort"
#define secure_token "secure"

#define DEFAULT_FILESYSTEM_SOURCE "File System"
#define DEFAULT_PACS_SOURCE "PACS 1"
#define TYPE_FILESYSTEM_SOURCE QListWidgetItem::UserType
#define TYPE_PACS_SOURCE QListWidgetItem::UserType+1
  
/**
 * @{ 
 ** 
 * @brief class sourceConfiguration
 *
 *  \copyright Elucid Bioimaging
 *  \ingroup workItem
 */
class sourceConfiguration : public QDialog
{
  Q_OBJECT

public:
  explicit sourceConfiguration(QString n, QWidget *p = 0, int t=TYPE_FILESYSTEM_SOURCE);
  ~sourceConfiguration();
  QString name;
  int type;
  QString startingFolder; // if file system type
  QString serverAETitle, serverHost, localAETitle; // if PACS type
  unsigned short serverPort,localPort; // if PACS type
  bool secure;
                                     
public slots:
  void accept();
  void reject();
  void displayConfiguration();

private:
  QWidget *owner;
  //QString configFileName;
  QDialog *Dialog;
  QGridLayout *gridLayout;
  QDialogButtonBox *buttonBox;
  QGroupBox *groupBox;
  QFormLayout *formLayout;
  QLabel *serverAETitleLabel,*serverHostLabel,*serverPortLabel;
  QLineEdit *serverAETitleEdit,*serverHostEdit,*serverPortEdit;
  QLabel *localAETitleLabel,*localPortLabel;
  QLineEdit *localAETitleEdit,*localPortEdit;
  QLabel *secureLabel;
  QCheckBox *secureCheckBox;
  void assignValuesFromSaved();
};

/** @} */

#endif // SOURCECONFIGURATION_H
