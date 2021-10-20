#pragma once

//c++ general
#include <iostream>
#include <vector>

//caplib related
#include "ebiHelper.h"
#include "ebiImageReader.h"
#include "ebiThumbnailImageFilter.h"
#include "ebiVesselPipeline.h"
#include "ebiVesselTarget.h"
#include "ebiVesselTargetPipeline.h"
#include "ebvViewer2D.h"
#include "ebAssert.h"
#include "ebiMultiImageReader.h"
#include "ebiImageMetaData.h"
#include "ebiMultiImageRegistrationFilter.h"
#include "ebiLocalImageRegistrationFilter.h"
#include "ebiAnalyte.h"
#include "ebiBitPlaneImageToColorImageFilter.h"
#include "ebvLinkedViewers2.h"

#include "ebvViewerChart.h"
#include "ebvRecursiveViewportGrid.h"
#include "ebvEvent.h"
#include "ebIDMap.h"
#include "ebLog.h"
#include <itkImage.h>
#include <itkRGBAPixel.h>
#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <vector>



//Qt utilities
#include <QDebug>
#include <QFileInfo>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QList>
#include <QHttpMultiPart>
#include <QFileDialog>  //QDir
#include <QUuid>
#include <QThread>

#include "EVConfig.h"
