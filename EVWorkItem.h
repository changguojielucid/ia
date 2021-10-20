#pragma once

#include "qobject.h"
#include "EVFirst.h"
#include "EVTargetDef.h"
#include "EVLinkedViewers.h"
#include "EVTargetDefine.h"
#include "EVVesselPipeline.h"
#include "EVImageSeries.h"
#include "EVTargetDef.h"


class EVTargetDefine;
class EVLinkedViewers;
class EVVesselPipeline;

class EVWorkItem : public QObject
{
  Q_OBJECT

public:
  EVWorkItem();
  ~EVWorkItem();

public:
  inline QList<EVTargetDef>*  GetTargetDefs() { return &m_lstTargets; }
  inline ebiMultiImageReader::Pointer GetMultiImageReader();
  inline EVLinkedViewers* GetEVLinkerViewers() { return m_pEvLinkedViewers; }
  inline EVTargetDefine* GetTargetDefine() { return m_pEvTargetDefine;  }
  inline EVVesselPipeline* GetVesselPipeline() { return m_pPipeline; }

  EVImageSeries*  GetImageSeries() { return &m_ImageSeries; }

  bool LoadImageFromJsonObject(const QJsonObject &json);
  bool LoadImageFromSeriesPath(const std::string& seriesPath);
  bool LoadImageFromWorkItemListFile(const QString& strJsnWorklistDesktopAppFileName, int iWorkItemIdx = 0, int iSeriesIdx = 0);
  void RenderImage();

public slots:
  void doWork(const QString &parameter)
  {
    QString result;
    /* ... here is the expensive or blocking operation ... */
    emit resultReady(result);
  }

signals:
  void resultReady(const QString &result);

private:
  EVTargetDefine* m_pEvTargetDefine;
  EVLinkedViewers* m_pEvLinkedViewers;
  EVVesselPipeline* m_pPipeline;

  EVImageSeries      m_ImageSeries;
  QList<EVTargetDef> m_lstTargets;

};

