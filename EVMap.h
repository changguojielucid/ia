#pragma once

#include "EVFirst.h"


// QMap<QString, QString> map;
//#include <QMap>
//QList<EVRegion> regions;

/*
typedef QMap<QString, QString> EVMap
EVMap map;
map.insert("composition", compositionFileName)
*/

class EVProbabilityMap
{
public:
  EVProbabilityMap(const QString& name, const QString& fileName)
  {
    probabilityName = name;
    probabilityMapFileName = fileName;
  }

  QString getProbabilityMapFileName() const
  {
    ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
    return probabilityMapFileName;
  }

  void setProbabilityMapFileName(const QString& name)
  {
    ebLog eblog(Q_FUNC_INFO);
    eblog << "" << std::endl;
    probabilityMapFileName = name;
  }

  QString probabilityName;
  QString probabilityMapFileName;
};

class EVRegion
{
public:
  EVRegion(QString name, QString fileName)
  {
    regName = name;
    regFileName = fileName;
  }

  QString getRegFileName() const
  {
    ebLog eblog(Q_FUNC_INFO);
    eblog << "" << std::endl; return regFileName;
  }

  void setRegFileName(QString name)
  {
    ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
    regFileName = name;
  }

  QString regName;
  QString regFileName;
};


class EVValueMap
{
public:
  EVValueMap(QString name, QString fileName)
  {
    valueName = name;
    valueMapFileName = fileName;
  }

  QString getValueMapFileName() const
  {
    ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl;
    return valueMapFileName;
  }

  void setValueMapFileName(QString name)
  {
    ebLog eblog(Q_FUNC_INFO);
    eblog << "" << std::endl;
    valueMapFileName = name;
  }
  QString valueName;
  QString valueMapFileName;
};

