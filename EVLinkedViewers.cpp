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



#include "EVLinkedViewers.h"
#include "EVTargetDefine.h"

EVLinkedViewers::EVLinkedViewers(EVWorkItem* pWorkItem)
{
  m_pWorkItem = pWorkItem;
  m_pRenwin = nullptr;
  
  SetOffscreenRendering(false); // main switch! 

  SetupLinkedViewers("default");
}

EVLinkedViewers::~EVLinkedViewers()
{
}

EVTargetDefine* EVLinkedViewers::GetTargetDefine()
{
  EVWorkItem* pWorkItem = GetWorkItem();
  assert(pWorkItem);

  return pWorkItem->GetTargetDefine();
}


/*
code on desktop version
void EVLinkedViewers::SetupEVLinkedViewers()
{

  ebvIdVolume = viewers->AddViewer(ebvViewer::THREED,ui->volumeRender->GetRenderWindow());
  viewers->GetViewer(ebvIdVolume)->SetShowLogo(true);
  viewers->GetViewer3D(ebvIdVolume)->SetShowVolume(false);
  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::AXIAL, true);
  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CORONAL, true);
  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::SAGITTAL, true);
  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUE, false);
  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP1, false);
  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::CPR, false);
  viewers->GetViewer3D(ebvIdVolume)->SetShowSlice(ebvViewer::OBLIQUEPERP2, false);
  ebvIdCoronal = viewers->AddViewer(ebvViewer::CORONAL,ui->coronalRender->GetRenderWindow());
  coronalType = ebvViewer::CORONAL;
  viewers->GetViewer(ebvIdCoronal)->SetShowLogo(true);
  //viewers->GetViewer2D(ebvIdCoronal)->SetSlabLineVisible(0,false);
  ebvIdSagittal = viewers->AddViewer(ebvViewer::SAGITTAL,ui->sagittalRender->GetRenderWindow());
  sagittalType = ebvViewer::SAGITTAL;
  viewers->GetViewer(ebvIdSagittal)->SetShowLogo(true);
  //viewers->GetViewer2D(ebvIdSagittal)->SetSlabLineVisible(0,false);
  ebvIdAxial = viewers->AddViewer(ebvViewer::AXIAL,ui->axialRender->GetRenderWindow());
  axialType = ebvViewer::AXIAL;
  viewers->GetViewer(ebvIdAxial)->SetShowLogo(true);
  //viewers->GetViewer2D(ebvIdAxial)->SetSlabLineVisible(0,false);
  viewers->GetScene()->SetImageSlabType(VTK_IMAGE_SLAB_MAX);
  viewers->GetScene()->SetSlabThickness(0);
  viewers->SyncViewersToScene();
  viewers->UpdateCameras();
  viewers->SetVesselTargetInitializationIntensityWindow(owner->systemPreferencesObject->getTargetDefineIntensityWindow());
  ui->coronalLabel->setText(tr("MIP enabled (set slab thickness as desired)"));
  ui->sagittalLabel->setText(tr("MIP enabled (set slab thickness as desired)"));
  ui->axialLabel->setText(tr("MIP enabled (set slab thickness as desired)"));
  vtkOutputWindow::SetInstance(ebvOutputWindow::New());
  viewers->Render();
}
*/

void EVLinkedViewers::SetupLinkedViewers(const std::string& layout) 
{
  ebAssert(!m_pRenwin);
  m_pRenwin = vtkSmartPointer<vtkRenderWindow>::New();
  m_pRenwin->SetSize(800, 600); // later
  m_pRenwin->SetPosition(0, 0);
 
  ebvRecursiveViewportGrid grid({ 1,1 }, { 1,1 });
  AddViewer(ebvViewer::AXIAL, m_pRenwin, grid.GetViewport(0, 1));
  AddViewer(ebvViewer::CORONAL, m_pRenwin, grid.GetViewport(1, 1));
  AddViewer(ebvViewer::SAGITTAL, m_pRenwin, grid.GetViewport(1, 0));
  ebID id3D = AddViewer(ebvViewer::THREED, m_pRenwin, grid.GetViewport(0, 0));

  //Init a nice 3D viewer
  //ebvViewer3D *pViewer3D = GetViewer3D(id3D);
  //assert(pViewer3D);
}


bool EVLinkedViewers::AddImageFromMultiReader(ebiMultiImageReader::Pointer pMultiReader, ebID imageID, const std::string& seriesType)
{
  Clear();

  EVWorkItem* pWorkItem = GetWorkItem();
  ebAssert(pWorkItem);

  EVImageSeries* pImageSeries = pWorkItem->GetImageSeries();
  ebAssert(pImageSeries);

  ebiVesselPipeline* pPipeine = pWorkItem->GetVesselPipeline();
  ebAssert(pPipeine);

  pImageSeries->image4IDdefine = AddImage4(pMultiReader, imageID, seriesType);
  GetScene()->AutoWindowLevel();

  return true;
}

void EVLinkedViewers::RenderImage()
{
  GetScene()->SetCursorPositionToSceneCenter();
  SetScreenScaleFitFactor2D(1.0);
  OptimizeScreenScale2D();
  SyncViewersToScene();
  InitializeCameras();
  Render();
}





