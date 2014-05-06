include(../common.pri)

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = testGUI
TEMPLATE = app

SOURCES += main.cpp mainwindow.cpp PaintWidget.cpp
HEADERS  += mainwindow.h PaintWidget.h

FORMS    += mainwindow.ui PaintWidget.ui

# Library
LIBS += -L$$PWD/../bowlib/$$CFG/lib -lbow
INCLUDEPATH += ../bowlib
