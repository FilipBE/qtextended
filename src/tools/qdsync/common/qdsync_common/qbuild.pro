MODULE_NAME=qdsync_common
TEMPLATE=lib
TARGET=qdsync_common

CONFIG+=qtopia singleexec

SOURCEPATH=.. ../qws

include(../common.pri)
HEADERS=$$QTOPIADESKTOP_HEADERS
PRIVATE_HEADERS=$$QTOPIADESKTOP_PRIVATE_HEADERS
HEADERS=$$QTOPIADESKTOP_HEADERS
SOURCES=$$QTOPIADESKTOP_SOURCES

HEADERS+=\
    qtopia4sync.h\
    qdglobal.h

SOURCES+=\
    qtopia4sync.cpp

