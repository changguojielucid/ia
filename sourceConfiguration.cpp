// Copyright (c) Elucid Bioimaging

#include "cap.h"
#include "sourceConfiguration.h"
#include <QFileDialog>
#include <QStandardPaths>
#include <QDebug>
#ifdef _MSC_VER
#include <winsock.h>  // for gethostname()
#endif

/**
 * \ingroup workItem
 * @{
 *
 * See workItem.h for description of the package purpose and contents.  This file has the member functions for classes in the package.
 */

/**
 * @page sourceConfiguration member functions
 */
void sourceConfiguration::assignValuesFromSaved()
{
  ebLog eblog(Q_FUNC_INFO); eblog << std::endl;
  serverAETitleEdit->setText(serverAETitle);
  serverHostEdit->setText(serverHost);
  serverPortEdit->setText(QString::number(serverPort));
  localAETitleEdit->setText(localAETitle);
  localPortEdit->setText(QString::number(localPort));
  secureCheckBox->setCheckState(secure ? Qt::Checked : Qt::Unchecked);
}

sourceConfiguration::sourceConfiguration(QString n, QWidget *p, int t) : QDialog(p)
{
  ebLog eblog(Q_FUNC_INFO); eblog << std::endl;
  name = n;
  owner = p;
  type = t;

  // default values (if not overridden by values persisted in config file)
  startingFolder = cap::getDocumentsDir();
  serverAETitle = "ACME1";
  serverHost = "bekku";
  serverPort = 5678;
  char hostname[1024];
  gethostname(hostname,sizeof(hostname)-1);
  localAETitle = hostname;
  localPort = 11112;
  secure = true;
  
  // if the config file is present, use it to override the default values (as means to save across sessions)
  QString configFileName;
  if (type == TYPE_FILESYSTEM_SOURCE) {
    configFileName = cap::getConfigurationFileSystemSourcesDir() + "/" + name + ".json";
  }
  if (type == TYPE_PACS_SOURCE) {
    configFileName = cap::getConfigurationPACSSourcesDir() + "/" + name + ".json";
  }
  QFileInfo configFileInfo(configFileName);
  if (configFileInfo.exists() && configFileInfo.isFile() && configFileInfo.isReadable()) {
    QFile configFile(configFileName);

    if (!configFile.open(QIODevice::ReadOnly)) {
      QMessageBox::warning(owner, tr("Warning: Cannot open source configuration file for reading."), QString(tr("Warning: Cannot open source configuration file for reading (should not effect normal CAP operation but please contact Elucid)")));
    }
    else {
      QByteArray saveData = configFile.readAll();
      QJsonDocument configDoc = QJsonDocument::fromJson(saveData);
      QJsonObject json = configDoc.object();
      if (type == TYPE_PACS_SOURCE) {
        serverAETitle = json[serverAETitle_token].toString(); 
        serverHost = json[serverHost_token].toString(); 
        serverPort = json[serverPort_token].toInt();
              localAETitle = json[localAETitle_token].toString(); 
        localPort = json[localPort_token].toInt(); 
              secure = json[secure_token].toBool();
      }
      else {
        startingFolder = json[startingFolder_token].toString();
      }
    }
  }
  
  if (type == TYPE_PACS_SOURCE) {
    // setup the form
    Dialog = new QDialog(owner);
    Dialog->setObjectName(QStringLiteral("sourceConfiguration"));
    Dialog->resize(638, 383);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(Dialog->sizePolicy().hasHeightForWidth());
    Dialog->setSizePolicy(sizePolicy);
    gridLayout = new QGridLayout(Dialog);
    gridLayout->setObjectName(QStringLiteral("gridLayout"));
    buttonBox = new QDialogButtonBox(Dialog);
    buttonBox->setObjectName(QStringLiteral("buttonBox"));
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    gridLayout->addWidget(buttonBox, 1, 0, 1, 1);
    groupBox = new QGroupBox(Dialog);
    groupBox->setObjectName(QStringLiteral("groupBox"));
    formLayout = new QFormLayout(groupBox);
    formLayout->setObjectName(QStringLiteral("formLayout"));
    // server
    serverAETitleLabel = new QLabel(groupBox);
    serverAETitleLabel->setObjectName(QStringLiteral("serverAETitleLabel"));
    formLayout->setWidget(0, QFormLayout::LabelRole, serverAETitleLabel);
    serverAETitleEdit = new QLineEdit(groupBox);
    serverAETitleEdit->setObjectName(QStringLiteral("Server AE Title"));
    formLayout->setWidget(0, QFormLayout::FieldRole, serverAETitleEdit);
    serverHostEdit = new QLineEdit(groupBox);
    serverHostEdit->setObjectName(QStringLiteral("Server Host"));
    QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
    sizePolicy1.setHorizontalStretch(0);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(serverHostEdit->sizePolicy().hasHeightForWidth());
    serverHostEdit->setSizePolicy(sizePolicy1);
    formLayout->setWidget(1, QFormLayout::FieldRole, serverHostEdit);
    serverHostLabel = new QLabel(groupBox);
    serverHostLabel->setObjectName(QStringLiteral("serverHostLabel"));
    formLayout->setWidget(1, QFormLayout::LabelRole, serverHostLabel);
    serverPortEdit = new QLineEdit(groupBox);
    serverPortEdit->setObjectName(QStringLiteral("Server Port"));
    QValidator *serverPortValidator = new QIntValidator(0,65535,this);
    serverPortEdit->setValidator(serverPortValidator);  
    formLayout->setWidget(2, QFormLayout::FieldRole, serverPortEdit);
    serverPortLabel = new QLabel(groupBox);
    serverPortLabel->setObjectName(QStringLiteral("serverPortLabel"));
    formLayout->setWidget(2, QFormLayout::LabelRole, serverPortLabel);
    // local
    localAETitleLabel = new QLabel(groupBox);
    localAETitleLabel->setObjectName(QStringLiteral("localAETitleLabel"));
    formLayout->setWidget(3, QFormLayout::LabelRole, localAETitleLabel);
    localAETitleEdit = new QLineEdit(groupBox);
    localAETitleEdit->setObjectName(QStringLiteral("Local AE Title"));
    formLayout->setWidget(3, QFormLayout::FieldRole, localAETitleEdit);
    localPortEdit = new QLineEdit(groupBox);
    localPortEdit->setObjectName(QStringLiteral("Local Port"));
    QValidator *localPortValidator = new QIntValidator(0,65535,this);
    localPortEdit->setValidator(localPortValidator);  
    formLayout->setWidget(4, QFormLayout::FieldRole, localPortEdit);
    localPortLabel = new QLabel(groupBox);
    localPortLabel->setObjectName(QStringLiteral("localPortLabel"));
    formLayout->setWidget(4, QFormLayout::LabelRole, localPortLabel);
    // secure
    secureCheckBox = new QCheckBox(groupBox);
    secureCheckBox->setCheckState(secure ? Qt::Checked : Qt::Unchecked);
    secureCheckBox->setObjectName(QStringLiteral("Secure Connection"));
    formLayout->setWidget(5, QFormLayout::FieldRole, secureCheckBox);
    secureLabel = new QLabel(groupBox);
    secureLabel->setObjectName(QStringLiteral("secureLabel"));
    formLayout->setWidget(5, QFormLayout::LabelRole, secureLabel);
    
    gridLayout->addWidget(groupBox, 0, 0, 1, 1);
    QObject::connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    QObject::connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    QMetaObject::connectSlotsByName(Dialog);
    Dialog->setFocus();
    Dialog->setWindowTitle(QApplication::translate("sourceConfiguration", "Source Configuration", 0));
    groupBox->setTitle(QApplication::translate("sourceConfiguration", "PACS Connection", 0));
    serverAETitleLabel->setText(QApplication::translate("sourceConfiguration", "Server AE Title:", 0));
    serverHostLabel->setText(QApplication::translate("sourceConfiguration", "Server Host:", 0));
    serverPortLabel->setText(QApplication::translate("sourceConfiguration", "Server Port:", 0));
    localAETitleLabel->setText(QApplication::translate("sourceConfiguration", "Local AE Title:", 0));
    localPortLabel->setText(QApplication::translate("sourceConfiguration", "Local Port:", 0));
    secureLabel->setText(QApplication::translate("sourceConfiguration", "Secure:", 0));

    // now use the established values
    assignValuesFromSaved();
  }
  else 
    Dialog = NULL;
}

sourceConfiguration::~sourceConfiguration()
{
  if (Dialog != NULL) {
    Dialog->hide();
    delete Dialog;
  }
}

void sourceConfiguration::displayConfiguration()
{
  ebLog eblog(Q_FUNC_INFO); eblog << std::endl;
  // if it is a file system type, put up a dialog to select a folder, if PACS, a dialog to get the setup paramters
  if (type == TYPE_FILESYSTEM_SOURCE) {
    QString importFolderAbsolute = QFileDialog::getExistingDirectory(this, tr("Import Folder"), startingFolder, QFileDialog::ShowDirsOnly);
    QDir    importFolderDir = QDir::current();
    QString importFolder = importFolderDir.relativeFilePath(importFolderAbsolute);
    if (importFolder.isNull())
      QMessageBox::warning(owner, tr("didn't select one"), QString(tr("didn't select one")));
    else {
      startingFolder = importFolderAbsolute;
      QString configFileName = cap::getConfigurationFileSystemSourcesDir() + "/" + name + ".json";
      QFileInfo configFileInfo(configFileName);
      QFile saveFile(configFileName);
      if (!saveFile.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(owner, tr("Warning: Cannot open source configuration file for writing."), QString(tr("Warning: Cannot open source configuration file for writing (should not effect normal CAP operation but please contact Elucid)")));
      }
      else {
        QJsonObject json;
        json[startingFolder_token] = startingFolder; 

        QJsonDocument saveDoc = QJsonDocument(json);
        saveFile.write(saveDoc.toJson());
        saveFile.flush();
        saveFile.close();
      }
    }
  }
  else {
    Dialog->hide(); //first hide before show, which has the effect of forcing to top
    Dialog->show();
  }
}

void sourceConfiguration::accept()
{
  ebLog eblog(Q_FUNC_INFO); eblog << std::endl;
  // take the necessary steps to act on the values
  serverAETitle = serverAETitleEdit->text();
  serverHost = serverHostEdit->text();
  serverPort = serverPortEdit->text().toUInt();
  localAETitle = localAETitleEdit->text();
  localPort = localPortEdit->text().toUInt();
  secure = (secureCheckBox->checkState() == Qt::Checked);

  // also write to config file for next time
  QString configFileName;
  if (type == TYPE_FILESYSTEM_SOURCE)
    configFileName = cap::getConfigurationFileSystemSourcesDir() + "/" + name + ".json";
  if (type == TYPE_PACS_SOURCE)
    configFileName = cap::getConfigurationPACSSourcesDir() + "/" + name + ".json";
  QFileInfo configFileInfo(configFileName);
  QFile saveFile(configFileName);
  if (!saveFile.open(QIODevice::WriteOnly)) {
    QMessageBox::warning(owner, tr("Warning: Cannot open source configuration file for writing."), QString(tr("Warning: Cannot open source configuration file for writing (should not effect normal CAP operation but please contact Elucid)")));
  }
  else {
    QJsonObject json;
    json[serverAETitle_token] = serverAETitle; 
    json[serverHost_token] = serverHost; 
    json[serverPort_token] = serverPort;
    json[localAETitle_token] = localAETitle; 
    json[localPort_token] = localPort; 
    json[secure_token] = secure;
    
    QJsonDocument saveDoc = QJsonDocument(json);
    saveFile.write(saveDoc.toJson());
    saveFile.flush();
    saveFile.close();
  }

  Dialog->hide();
}

void sourceConfiguration::reject()
{
  ebLog eblog(Q_FUNC_INFO); eblog << std::endl;
  // the user rejected any changes they may have made, so restore prior values
  assignValuesFromSaved();
  Dialog->hide();
}

/** @} */
