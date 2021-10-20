// Copyright (c) Elucid Bioimaging
#ifndef STATUSWATCHER_H
#define STATUSWATCHER_H

#include <QObject>
#include <QString>
#include <QSocketNotifier>
#include <QTextStream>
#include <QStack>
#include <cstdio>

/**
 * @{ 
 ** 
 * @brief class StatusWatcher to echo stdout and a processed version of stderr to the status bar (in addition to being written to the log files).
 *
 *  \copyright Elucid Bioimaging
 *  \ingroup cap
 */
class StatusWatcher : public QObject
{
  Q_OBJECT
public:
  StatusWatcher(QObject *owner, QString outLog, QString errLog);

signals:
  void statusMessage(QString);
  void toggleNotifier(bool, QSocketNotifier*);
  
private:
  QSocketNotifier *outNotifier, *errNotifier;
  QString outLogPath, errLogPath;
  FILE *outFilePtr, *errFilePtr;
  QTextStream *outStream, *errStream;
  QStack<QString> prefixedMessage;

private slots:
  void readChangedLog(int socket);
};
/** @} */

#endif  // STATUSWATCHER_H
