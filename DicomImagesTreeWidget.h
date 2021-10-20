// Copyright (c) Elucid Bioimaging
#ifndef DICOMIMAGESTREEWIDGET_H
#define DICOMIMAGESTREEWIDGET_H

#include "ebLog.h"

#include <QTreeWidget>
#include <QDragEnterEvent>

namespace Ui {
  class DicomImagesTreeWidget;
}

class DicomImagesTreeWidget : public QTreeWidget {
  Q_OBJECT
public:
  explicit DicomImagesTreeWidget(QWidget *parent=0);
  void dragEnterEvent(QDragEnterEvent *e);
signals:
public slots:
};

#endif  // DICOMIMAGESTREEWIDGET_H
