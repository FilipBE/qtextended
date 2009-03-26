TEMPLATE=lib
CONFIG+=qt embedded singleexec
TARGET=md5

MODULE_NAME=md5
LICENSE=BSD GPL_COMPATIBLE
QT=core

SOURCEPATH+=/qtopiacore/qt/src/3rdparty/md5

SOURCES+=md5.cpp md5hash.cpp
HEADERS+=md5.h md5hash.h md5hashglobal.h

