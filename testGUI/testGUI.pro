include(../common.pri)

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = testGUI


SOURCES += main.cpp mainwindow.cpp PaintWidget.cpp \
    RemoveDup.cpp
HEADERS  += mainwindow.h PaintWidget.h \
    Globals.h \
    RemoveDup.h

FORMS    += mainwindow.ui PaintWidget.ui \
    RemoveDup.ui

# Library
LIBS += -L$$PWD/../bowlib/$$CFG/lib -lbow
INCLUDEPATH += ../bowlib
