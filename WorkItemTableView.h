// Copyright (c) Elucid Bioimaging
#ifndef WORKITEMTABLEVIEW_H
#define WORKITEMTABLEVIEW_H

#include "ebLog.h"

#include <QTableView>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>

/**
 * @{ 
 ** 
 * @brief class WorkItemTableView
 *
 *  \copyright Elucid Bioimaging
 *  \ingroup workItem
 */

namespace Ui {
  class WorkItemTableView;
}

class WorkItemTableView : public QTableView {
  Q_OBJECT
public:
  QWidget *parent;
  explicit WorkItemTableView(QWidget *parent=0);
  void dragEnterEvent(QDragEnterEvent *e);
  void dragMoveEvent(QDragMoveEvent *e);
  void dropEvent(QDropEvent *e);
  
signals:
  void seriesDropped();
  
public slots:
};

/** @} */

#endif  // WORKITEMTABLEVIEW_H
