include(../common.pri)

# Configuration
TEMPLATE = lib
CONFIG += staticlib

TARGET = bow
DESTDIR = $$PWD/$$CFG/lib

SOURCES +=  galif.cpp \
            vocabulary.cpp \
            BofSearchManager.cpp \
            inverted_index.cpp \
            tf_idf.cpp

HEADERS +=  galif.h \
            types.h \
            image_sampler.h \
            utilities.h \
            vocabulary.h \
            kmeans.h \
            BofSearchManager.h \
            inverted_index.h \
            tf_idf.h \
            quantizer.h \
			histvw.h \
            iofiles.h
