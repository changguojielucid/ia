// Copyright (c) Elucid Bioimaging

#include <signal.h>

#include <QApplication>
#include <qstylefactory.h>
#include <QStandardPaths>
#include <QMessageBox>
#include <QTimer>
#include <QPixmap>
#include <QSplashScreen>
#include <QThread>

#include <string.h>
#include <iostream>
#include <cstdio>
#include <vtkVersion.h>


#include "cap.h"
#include "ebSystem.h"
#include "ebException.h"


#include <QCoreApplication>
#include "EVSocketServer.h"


/**
 * \ingroup end-user-app
 * @{
 *
 * @brief main entry point for application, implemented with level of indirection to allow automated unit test hook.
 */
typedef int(*AppRunFunc)();

void term(int signum) { std::cout << "trapped sig " << signum << '\n'; exit(0); }

class Application final : public QApplication 
{
public:
  Application(int& argc, char** argv) : QApplication(argc, argv) {}
  virtual bool notify(QObject *receiver, QEvent *event) override {
    try {
      return QApplication::notify(receiver, event);
    } catch (std::exception &e) {
      ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
      eblog << "EXCEPTION CAUGHT: " << e.what() << std::endl;
      eblog << "event=" << typeid(*event).name() << std::endl;
      eblog << "object=" << qPrintable(receiver->objectName()) << std::endl;
      eblog << "receiver=" << typeid(*receiver).name() << std::endl;
      qCritical("EXCEPTION: %s", e.what());
      QMessageBox error;
      QString errorMessage = tr("We were not able to complete the requested operation. Please email log files to Elucid.");
      error.setText(errorMessage);
      error.exec();
    } catch (...) {
      ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
      eblog << "EXCEPTION CAUGHT: non-standard exception" << std::endl;
      eblog << "event=" << typeid(*event).name() << std::endl;
      eblog << "object=" << qPrintable(receiver->objectName()) << std::endl;
      eblog << "receiver=" << typeid(*receiver).name() << std::endl;
      qCritical("EXCEPTION: non-standard exception");
      QMessageBox error;
      QString errorMessage = tr("We were not able to complete the requested operation. Please email log files to Elucid.");
      error.setText(errorMessage);
      error.exec();
    }
    return true;
  }
};

int cap_main_impl(int argc, char *argv[], AppRunFunc app_run)
{
  // launch GUI
  Application a(argc, argv);

  QPixmap pixmap(":/cap/splash screen 2021.png");
  QSplashScreen splash(pixmap);
  splash.show();
  unsigned long waitFactor = 1;
  if (ebSystem::NoWait())
    waitFactor = 0;
  QThread::msleep(1000*waitFactor);
  splash.showMessage(QSplashScreen::tr("\n\n\n\n\n\n   ...confirming registration and setting up local storage,"), Qt::AlignLeft, Qt::lightGray);
  splash.repaint(); // ensure progress is shown
  qApp->processEvents();
  
  // position at users CAP Exam Data folder, creating it if necessary

  QDir::setCurrent(cap::getDocumentsDir());

  QString examDataPath = cap::getCAPExamDataDir();
  QDir currDir(QDir::current());
  if (!currDir.mkpath(examDataPath)) {
    splash.showMessage(QSplashScreen::tr("\n\n\n\n\n\n   Cannot access exam data folder."), Qt::AlignLeft, Qt::lightGray);
    splash.repaint(); // ensure progress is shown
    qApp->processEvents();
    std::cerr << "Cannot access exam data folder, exiting. ";
    QThread::msleep(10000);
    splash.close();
    return 90;
  }
  QDir::setCurrent(examDataPath);
  std::cout << "CAP Exam Data directory is: " << QDir::currentPath().toStdString() << "\n";

  // make sure CAP Exam Data/Configuration folder exists, creating it if necessary
  QString configurationPath = examDataPath+CONFIGURATION_FOLDER;
  if (!currDir.mkpath(configurationPath)) {
    splash.showMessage(QSplashScreen::tr("\n\n\n\n\n\n   Cannot access configuration folder."), Qt::AlignLeft, Qt::lightGray);
    splash.repaint(); // ensure progress is shown
    qApp->processEvents();
    std::cerr << "Cannot access configuration folder, exiting. ";
    QThread::msleep(10000);
    splash.close();
    return 91;
  }
  
  // set up the logs
  QString outLog = "";
  FILE *outFilePtr = NULL;
  QString errLog = "";
  FILE *errFilePtr = NULL;
  QString logFolder = examDataPath+ACTIVITY_LOGS_FOLDER+"/";
  QDir logDir(QDir::current());
  if (logDir.mkpath(logFolder)) {
    QString prefix = logFolder;
    prefix.append( getLocalUserName() );
    prefix.append("_");
    QDateTime dt = QDateTime::currentDateTime();
    QString dtStr = dt.toString("yyyyMMdd-hhmm");
    prefix.append(dtStr);
    prefix.append("_");
    outLog = prefix;
    outLog.append("out.txt");
    outFilePtr = freopen(outLog.toLocal8Bit().data(), "w", stdout); 
  	
    errLog = prefix;
    errLog.append("err.txt");
    errFilePtr = freopen(errLog.toLocal8Bit().data(), "w", stderr);
  }
  // now delete all but the last ones
  QDir logFolderDir(logFolder);
  QFileInfoList logFileList = logFolderDir.entryInfoList(QDir::Files| QDir::NoDotAndDotDot, QDir::Time);
  for (int i=0; i < logFileList.size(); i++) {
    if (i < 20)
      continue;
    QFile::remove(logFileList.at(i).absoluteFilePath());
  }

  // set up the backup storage
  QString bakFolder = examDataPath+SESSION_BACKUPS_FOLDER;
  QDir bakDir(QDir::current());
  if (!bakDir.mkpath(bakFolder)) {
    splash.showMessage(QSplashScreen::tr("\n\n\n\n\n\n   Cannot access session backup folder."), Qt::AlignLeft, Qt::lightGray);
    splash.repaint(); // ensure progress is shown
    qApp->processEvents();
    std::cerr << "Cannot access session backup folder, exiting. ";
    QThread::msleep(10000);
    splash.close();
    return 92;
  }
  // now delete all but the last ones
  QDir bakFolderDir(bakFolder);
  QFileInfoList bakFileList = bakFolderDir.entryInfoList(QDir::Files| QDir::NoDotAndDotDot, QDir::Time);
  for (int i=0; i < bakFileList.size(); i++) {
    if (i < 100)
      continue;
    QFile::remove(bakFileList.at(i).absoluteFilePath());
  }

  // make sure CAP Exam Data/Working Storage folder exists, creating it if necessary
  QString workingStoragePath = examDataPath+WORKING_STORAGE_FOLDER;
  if (!currDir.mkpath(workingStoragePath)) {
    splash.showMessage(QSplashScreen::tr("\n\n\n\n\n\n   Cannot access working storage folder."), Qt::AlignLeft, Qt::lightGray);
    splash.repaint(); // ensure progress is shown
    qApp->processEvents();
    std::cerr << "Cannot access working storage folder, exiting. ";
    QThread::msleep(10000);
    splash.close();
    return 93;
  }

  // and now establish the GUI
  QThread::msleep(2000*waitFactor);
  splash.showMessage(QSplashScreen::tr("\n\n\n\n\n\n   ...setting display attributes,"), Qt::AlignLeft, Qt::lightGray);
  splash.repaint(); // ensure progress is shown
  qApp->processEvents();
  QApplication::setStyle(QStyleFactory::create("windows"));
  QPalette palette;
  palette.setColor(QPalette::Window, QColor(53,53,53));
  palette.setColor(QPalette::WindowText, Qt::white);
  palette.setColor(QPalette::Base, QColor(15,15,15));
  palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
  //palette.setColor(QPalette::ToolTipBase, Qt::white);
  //palette.setColor(QPalette::ToolTipText, Qt::white);
  palette.setColor(QPalette::Text, Qt::white);
  palette.setColor(QPalette::Button, QColor(53,53,53));
  palette.setColor(QPalette::ButtonText, Qt::white);
  palette.setColor(QPalette::BrightText, Qt::red);
  palette.setColor(QPalette::Highlight, QColor(142,45,197).lighter());
  palette.setColor(QPalette::HighlightedText, Qt::black);
  palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::darkGray); // this one for buttons
  palette.setColor(QPalette::Disabled, QPalette::Text, Qt::darkGray);  // this one for menus
  palette.setColor(QPalette::Disabled, QPalette::Light, Qt::gray); // this for both (the "shadow")
  a.setPalette(palette);
  std::string pathStdString(argv[0]);
  QFileInfo pathFileInfo(QString::fromStdString(pathStdString));
  std::cout << "Product is: " << pathFileInfo.baseName().toStdString() << std::endl;
  cap screen(0, pathFileInfo.baseName());
  QThread::msleep(2000*waitFactor);
  screen.setCAPgraphConfig();
  QString splashProductMessage = QSplashScreen::tr("\n\n\n\n\n\n   ...customizing for ");
  splashProductMessage.append(screen.thisProduct);
  splashProductMessage.append(QSplashScreen::tr("-specific behavior,"));
  splash.showMessage(splashProductMessage, Qt::AlignLeft, Qt::lightGray);
  splash.repaint(); // ensure progress is shown
  qApp->processEvents();
  QIcon applicationIcon;
  if (screen.thisProduct.contains("vascuCAP"))
    applicationIcon.addFile(QStringLiteral(":/cap/VascuCap_D1.ico"), QSize(), QIcon::Normal, QIcon::On);
  else if (screen.thisProduct.contains("lungCAP"))
    applicationIcon.addFile(QStringLiteral(":/cap/LungCap_D1.ico"), QSize(), QIcon::Normal, QIcon::On);
  screen.setWindowIcon(applicationIcon);
  screen.setLogs(outLog, errLog);
  qApp->setStyleSheet("QToolTip {color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
  qApp->setStyleSheet("QStatusBar {padding-left:8px; border: none; border-color: #444444; background-color: #444444; color:gray; font-weight:normal; }");
  QThread::msleep(2000*waitFactor);
  splash.showMessage(QSplashScreen::tr("\n\n\n\n\n\n   ...and establishing connection to CAPgraph."), Qt::AlignLeft, Qt::lightGray);
  splash.repaint(); // ensure progress is shown
  qApp->processEvents();
  screen.connectToCAPgraph();
  //QThread::msleep(2000);
  screen.showMaximized();
  splash.finish(&screen);
  //FIRST ATTEMPT   screen.resize(QApplication::desktop()->availableGeometry().width(),QApplication::desktop()->availableGeometry().height());
  //FIRST ATTEMPT   screen.setGeometry(QApplication::desktop()->availableGeometry());
  //SECOND ATTEMPT   int difference = screen.frameGeometry().height() - screen.geometry().height();
  //SECOND ATTEMPT   int height = QApplication::desktop()->availableGeometry().height() - difference;
  screen.setFixedHeight(screen.geometry().height());
  screen.setFixedWidth(screen.geometry().width());
  screen.setUnifiedTitleAndToolBarOnMac(false);

  // last but not least, cd to working storage and run the app
  QDir::setCurrent(workingStoragePath);
  std::cout << "Working storage directory is: " << QDir::currentPath().toStdString() << "\n";

  // Get the vtk version
 
  std::cerr << "VTK Major Version = " << vtkVersion::GetVTKMajorVersion() << std::endl;
  std::cerr << "VTK Minor Version = " << vtkVersion::GetVTKMinorVersion() << std::endl;
  std::cerr << "VTK Build Version = " << vtkVersion::GetVTKBuildVersion() << std::endl;
  std::cout << "VTK Major Version = " << vtkVersion::GetVTKMajorVersion() << std::endl;
  std::cout << "VTK Minor Version = " << vtkVersion::GetVTKMinorVersion() << std::endl;
  std::cout << "VTK Build Version = " << vtkVersion::GetVTKBuildVersion() << std::endl;


 // EVSocketServer server;
 //  server.startServer();



  return app_run();
}

int qapp_exec()
{
  return qApp->exec();
}

int cap_main(int argc, char *argv[])
{
  return cap_main_impl(argc, argv, qapp_exec);
}

/** @} */
