#-------------------------------------------------
#
# Project created by QtCreator 2015-11-03T17:27:05
#
#-------------------------------------------------

QT       += core gui
QT       += webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = cap
TEMPLATE = app


SOURCES += main.cpp\
        cap.cpp \
    patientAnalyze.cpp \
    patientReport.cpp \
    workItem.cpp \
    targetDefine.cpp \
    seriesSurvey.cpp \
    dicomQueryRetrieve.cpp \
    systemPreferences.cpp \
    DicomTreeWidget.cpp \
    WorkItemTableView.cpp \
    SegmentationEditor.cpp

HEADERS  += cap.h \
    patientAnalyze.h \
    patientReport.h \
    workItem.h \
    targetDefine.h \
    seriesSurvey.h \
    dicomQueryRetrieve.h \
    systemPreferences.h \
    DicomTreeWidget.h \
    WorkItemTableView.h \
    SegmentationEditor.h

FORMS    += \
    patientAnalyze.ui \
    patientReport.ui \
    workItem.ui \
    seriesSurvey.ui \
    targetDefine.ui \
    mainWindow.ui \
    aboutPage.ui \
    dialog.ui \
    systemPreferences.ui \
    dicomQueryRetrieve.ui \
    compositioncontrol.ui \
    SegmentationEditor.ui

TRANSLATIONS += \
    cap_fr.ts \
    cap_it.ts \
    cap_de.ts \
    cap_sp.ts

RESOURCES += \
    cap.qrc

RC_ICONS = ElucidLogotransp-for-dark.ico
