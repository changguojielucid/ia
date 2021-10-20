// Copyright (c) Elucid Bioimaging

#include "QITKProgressDialog.h"
#include "ebiHelper.h"
#include "ebAssert.h"
#include <QApplication>

/*virtual*/ QITKProgressDialog::~QITKProgressDialog() {
  progressUpdate->ClearFilters();
}

/*virtual*/ void QITKProgressDialog::QITKProgressUpdate::
Execute(itk::Object *caller, const itk::EventObject &event) /*override*/ {
  Execute((const itk::Object *)caller,event);
}

/*virtual*/ void QITKProgressDialog::QITKProgressUpdate::
Execute(const itk::Object *object, const itk::EventObject &event) /*override*/ {
  ebAssert(progressDialog);
  ebAssert(progressDialog->progressUpdate);
  // call superclass method
  ebiCompositeProgressUpdate::Execute(object,event);
  // update progress value
  progressDialog->setRange(0,1000);
  progressDialog->setValue(1000*progressDialog->progressUpdate->GetProgress());
  // update description string
  progressDialog->setLabelText(progressDialog->progressUpdate->GetProgressDescription().c_str());
  // refresh GUI
  progressDialog->repaint();
  qApp->processEvents();
}

QITKProgressDialog::QITKProgressDialog(QWidget *parent/*=0*/, Qt::WindowFlags f/*=0*/)
  : QProgressDialog(parent,f) {
  // set up observer
  progressUpdate = QITKProgressUpdate::New();
  progressUpdate->progressDialog = this;
}

void QITKProgressDialog::AddFilter(double stage, itk::ProcessObject *filter, double weight) {
  progressUpdate->AddFilter(stage,filter,weight);
}
