TARGET = qpcmousedriver
include(../../qpluginbase.pri)

target.path = $$[QT_INSTALL_PLUGINS]/mousedrivers
INSTALLS += target

DEFINES += QT_QWS_MOUSE_PC

HEADERS	= $$QT_SOURCE_TREE/src/gui/embedded/qmousepc_qws.h

SOURCES	= main.cpp \
	$$QT_SOURCE_TREE/src/gui/embedded/qmousepc_qws.cpp

