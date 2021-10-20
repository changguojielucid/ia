// Copyright (c) Elucid Bioimaging

#include "WorkItemTableView.h"

#include <QFrame>

#include <iostream>

/*explicit*/ WorkItemTableView::WorkItemTableView(QWidget *p/*=0*/) : QTableView(p) {
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; 
  parent = p;
}

void WorkItemTableView::dragEnterEvent(QDragEnterEvent *e) {
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; 
  e->acceptProposedAction();
  setFrameStyle(QFrame::Panel | QFrame::Sunken);
  setLineWidth(5); 
}

void WorkItemTableView::dragMoveEvent(QDragMoveEvent *e) {
  //ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; 
  e->acceptProposedAction(); 
}

void WorkItemTableView::dropEvent(QDropEvent *e) {
  ebLog eblog(Q_FUNC_INFO); eblog << "" << std::endl; 
  e->acceptProposedAction();
  emit seriesDropped();
  setFrameStyle(QFrame::NoFrame);
  setLineWidth(1);
}
