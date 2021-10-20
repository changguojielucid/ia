// Copyright (c) Elucid Bioimaging
#include "StatusWatcher.h"
#include "ebLog.h"
#include <QApplication>

/**
 * \ingroup cap
 * @{
 *
 * See cap.h for description of the CAP package purpose and contents.  This file has member functions for classes in the package.
 */

/**
 * @page StatusWatcher member functions
 */
StatusWatcher::StatusWatcher(QObject *owner, QString outLog, QString errLog)
{
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
  if (outLog != "") {
    outLogPath = outLog;
    outFilePtr = fopen(outLogPath.toStdString().c_str(), "r");
    outStream = new QTextStream(outFilePtr, QIODevice::ReadOnly);
    outNotifier = new QSocketNotifier(fileno(outFilePtr), QSocketNotifier::Write);
    //COMMENTED OUT FOR NOW  connect(outNotifier, SIGNAL(activated(int)), this, SLOT(readChangedLog(int)));
  }
  else {
    outLogPath = "";
    emit statusMessage(tr("output log could not be monitored in status bar"));
  }
  if (errLog != "") {
    errLogPath = errLog;
    errFilePtr = fopen(errLogPath.toStdString().c_str(), "r");
    errStream = new QTextStream(errFilePtr, QIODevice::ReadOnly);
    errNotifier = new QSocketNotifier(fileno(errFilePtr), QSocketNotifier::Write);
    //COMMENTED OUT FOR NOW  connect(errNotifier, SIGNAL(activated(int)), this, SLOT(readChangedLog(int)));
  }
  else {
    errLogPath = "";
    emit statusMessage(tr("error log could not be monitored in status bar"));
  }
  prefixedMessage.push("CAP Processing: ");
}

void StatusWatcher::readChangedLog(int socket)
{
  //CAREFUL NOT TO DO THE NORMAL LOG HERE, IT WOULD CREATE AN INFINITE LOOP
  if (socket == fileno(outFilePtr)) {
    //emit toggleNotifier(false, outNotifier);
    fflush(stdout);
    fflush(outFilePtr);
    outStream->flush();
    qApp->processEvents();
    while (!outStream->atEnd()) {
      QString line = outStream->readLine();
      emit statusMessage(line);
    }

    //COMMENTED OUT FOR NOW  emit toggleNotifier(true, outNotifier);
  }
  else if (socket == fileno(errFilePtr)) {
    //emit toggleNotifier(false, errNotifier);
    fflush(stderr);
    fflush(errFilePtr);
    errStream->flush();
    while (!errStream->atEnd()) {
      QString line = errStream->readLine();
      QRegExp dateExp("[0-9][0-9]:[0-9][0-9]:[0-9][0-9] [0-9][0-9][0-9][0-9]");
      if ((line != "") && (!line.contains("elapsed seconds")) && (!line.contains(dateExp))) {
        line.remove("void ");
        line.remove("int ");
        line.remove("QString ");
              line.remove("__cdecl ");
        if (line.contains("LOGEND")) {
          prefixedMessage.pop();
          if (prefixedMessage.size() > 1)
            emit statusMessage(prefixedMessage.top());
          else 
            emit statusMessage("CAP Ready");
        }
        else {
          QString prefixedLine = "";
          if (prefixedMessage.size() > 1)
            prefixedLine = prefixedMessage.top();
          if (prefixedLine != "")
            if (prefixedMessage.size() > 1)
              prefixedLine.append(", ");
          if (line.contains("LOGBEGIN")) { // parse lines that are formatted similar to LOGBEGIN workItem::workItem
            line.remove("LOGBEGIN "); // now LOGBEGIN and space is gone
            if (line.contains("(")) { // the Qt function infor is in it
              QStringList lineParts = line.split('(');
              if (lineParts.size() > 0)
                line = lineParts.at(0); // now the ownerhesis and everythign after it is gone
            }
            prefixedLine.append(line);
            prefixedMessage.push(prefixedLine);
            emit statusMessage(prefixedMessage.top());
          }
          else if ((!line.contains("vtk")) && (!line.contains("itk"))) {
            if (prefixedMessage.size() > 1)
              prefixedLine.append(line);
            else {
              prefixedLine = "CAP Ready (";
              prefixedLine.append(line);
              prefixedLine.append(")");
            }
            emit statusMessage(prefixedLine);
          }
        }
      }
    }
    //COMMENTED OUT FOR NOW  emit toggleNotifier(true, errNotifier);
  }
}
/** @} */
