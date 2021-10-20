
/*
*             dP                   oo       dP          oo
*             88                            88
*    .d8888b. 88 dP    dP .d8888b. dP .d888b88 dP   .dP dP dP   .dP .d8888b.
*    88ooood8 88 88    88 88'  `"" 88 88'  `88 88   d8' 88 88   d8' 88'  `88
*    88.  ... 88 88.  .88 88.  ... 88 88.  .88 88 .88'  88 88 .88'  88.  .88
*    `88888P' dP `88888P' `88888P' dP `88888P8 8888P'   dP 8888P'   `88888P'
*    oooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo
*

*
*    Created:
*
*    Modified:
*
*
*
*
*
*                              Copyright (c) Elucid Bioimaging
*/


#include "EVFirst.h"
#include "EVLinkedViewers.h"
#include "EVTargetDefine.h"
#include "EVWorkItem.h"
#include "EVMacros.h"
#include "EVWorkItemThread.h"

EVWorkItem::EVWorkItem()
{
  m_pPipeline        = new EVVesselPipeline(this);
  m_pEvTargetDefine  = new EVTargetDefine(this);
  m_pEvLinkedViewers = new EVLinkedViewers(this);
  


  m_pEvTargetDefine->preloadDefinePre("vascuCAP");
  m_pEvTargetDefine->preloadDefinePost();
}



EVWorkItem::~EVWorkItem()
{
  SAFE_REMOVAL(m_pEvLinkedViewers)
  SAFE_REMOVAL(m_pEvTargetDefine)
  SAFE_REMOVAL(m_pPipeline)
}


ebiMultiImageReader::Pointer EVWorkItem::GetMultiImageReader()
{ 
  ebAssert(m_pPipeline);
  return  m_pPipeline->GetMultiImageReader();
}

void EVWorkItem::RenderImage()
{
  EVLinkedViewers* pEvLinkedViewers = GetEVLinkerViewers();
  ebAssert(pEvLinkedViewers);

  pEvLinkedViewers->RenderImage();
}

bool EVWorkItem::LoadImageFromJsonObject(const QJsonObject &json)
{
  EVLinkedViewers* pEvLinkedViewers = GetEVLinkerViewers();
  ebAssert(pEvLinkedViewers);

  ebiMultiImageReader::Pointer pMultiReader = GetMultiImageReader();
  ebAssert(pMultiReader);

  EVVesselPipeline* pPipeline = GetVesselPipeline();
  ebAssert(pPipeline);

  m_ImageSeries.readImageSeries(json);
  
  qDebug() << m_ImageSeries.seriesFolder;
  qDebug() << QDir(m_ImageSeries.seriesFolder).exists();

  //deal with workitem folder with seriesFolder later
  m_ImageSeries.imageID = pMultiReader->OpenImage<float, 3>(m_ImageSeries.seriesFolder.toStdString());

  pEvLinkedViewers->AddImageFromMultiReader(pMultiReader, m_ImageSeries.imageID, m_ImageSeries.seriesType.toStdString());

 
  EVTargetDefine* pEvTargetDefine = GetTargetDefine();
  ebAssert(pEvTargetDefine);
  pEvTargetDefine->resetAllTargetsDueToDifferentImages();
  pPipeline->SetPrimaryImageID(m_ImageSeries.imageID);

  return true;
}

bool EVWorkItem::LoadImageFromSeriesPath(const std::string& seriesPath)
{
  /*
  if (QDir::exists(seriesPath))
  {
    ebAssert(false);
    return false;
  }
  */

  // all image loading should be centralized by function reading a json obj
  QJsonObject json;

  EVImageSeries series; // tmp obj, more inits if necessary
  series.seriesFolder = QString::fromStdString(seriesPath);

  series.writeImageSeries(json);
  return LoadImageFromJsonObject(json);
}

bool EVWorkItem::LoadImageFromWorkItemListFile(const QString& strJsnWorklistDesktopAppFileName, int iWorkItemIdx /*= 0*/, int iSeriesIdx /*= 0*/)
{
  QFile loadFile(strJsnWorklistDesktopAppFileName);
  if (loadFile.exists())
  {
    if (!loadFile.open(QIODevice::ReadOnly)) {
      return false;
    }

    QJsonArray wiArray = QJsonDocument::fromJson(loadFile.readAll()).object()[workItemList_token].toArray();
    qDebug() << wiArray.size();
    ebAssert(wiArray.size() > iWorkItemIdx);

    QJsonArray seriesArray = wiArray[iWorkItemIdx].toObject()[imageSeriesSet_token].toArray();
    qDebug() << seriesArray.size();
    assert(seriesArray.size() > iSeriesIdx);

    QJsonObject seriesObject = seriesArray[iSeriesIdx].toObject();
    return LoadImageFromJsonObject(seriesObject);
  }

  return false;
}

