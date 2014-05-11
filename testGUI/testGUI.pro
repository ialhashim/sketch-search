include(../common.pri)

QT       += core gui opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE    = app
TARGET      = testGUI

INCLUDEPATH *= .

SOURCES +=  main.cpp \
            mainwindow.cpp \
            PaintWidget.cpp \
            RemoveDup.cpp \
            Render3D.cpp \
            Viewer.cpp

HEADERS  += mainwindow.h \
            PaintWidget.h \
            Globals.h \
            RemoveDup.h \
            Render3D.h \
            Viewer.h

FORMS    += mainwindow.ui  \
            PaintWidget.ui \
            RemoveDup.ui \
            Render3D.ui

# Library
LIBS += -L$$PWD/../bowlib/$$CFG/lib -lbow
INCLUDEPATH += ../bowlib
