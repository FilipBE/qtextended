TARGET = qyopykbddriver
include(../../qpluginbase.pri)

target.path = $$[QT_INSTALL_PLUGINS]/kbddrivers
INSTALLS += target

DEFINES += QT_QWS_KBD_YOPY

HEADERS	= $$QT_SOURCE_TREE/src/gui/embedded/qkbdyopy_qws.h

SOURCES	= main.cpp \
	$$QT_SOURCE_TREE/src/gui/embedded/qkbdyopy_qws.cpp

