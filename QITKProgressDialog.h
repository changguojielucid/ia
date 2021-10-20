// Copyright (c) Elucid Bioimaging
#ifndef QITKPROGRESSDIALOG_H_
#define QITKPROGRESSDIALOG_H_

#include "ebiCompositeProgressUpdate.h"
#include <QProgressDialog>
#include <itkProcessObject.h>

/** \brief A Qt progress dialog that is an observer of ITK filters
 *
 *  This progress dialog becomes an observer of ITK filters looking
 *  for progress events (which vary from 0 to 1).  It then refreshes
 *  the GUI at each progress event so that progress dialog reflects
 *  the current progress of the filter.
 *
 *  \copyright Elucid Bioimaging
 *  \ingroup cap
 **/

class QITKProgressDialog : public QProgressDialog {
  Q_OBJECT
public:
  //! constructor (adds itself as ITK observer; removal of observer on destruction)
  QITKProgressDialog(QWidget *parent=0, Qt::WindowFlags f=0);

  virtual ~QITKProgressDialog();
  
  //! add a sub-filter to observe, weight should be roughly proportional to compute time
  void AddFilter(double stage, itk::ProcessObject *filter, double weight);
  
protected:
  class QITKProgressUpdate : public ebiCompositeProgressUpdate {
  public:
    typedef itk::SmartPointer<QITKProgressUpdate> Pointer;
    itkNewMacro(QITKProgressUpdate);
    virtual void Execute(itk::Object *caller, const itk::EventObject &event) override;
    virtual void Execute(const itk::Object *object, const itk::EventObject &event) override;
    QITKProgressDialog *progressDialog;
  };
  
  QITKProgressUpdate::Pointer progressUpdate;
};


#endif  // QITKPROGRESSDIALOG_H_
