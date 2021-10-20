// Copyright (c) Elucid Bioimaging

#include "DicomImagesTreeWidget.h"
#include <iostream>

/*explicit*/ DicomImagesTreeWidget::DicomImagesTreeWidget(QWidget *parent/*=0*/) : QTreeWidget(parent) {
  ebLog eblog(Q_FUNC_INFO);
}

void DicomImagesTreeWidget::dragEnterEvent(QDragEnterEvent *e) {
  //ebLog eblog(Q_FUNC_INFO);
  e->acceptProposedAction();
}

