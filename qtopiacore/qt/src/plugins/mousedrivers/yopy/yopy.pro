TARGET = qyopymousedriver
include(../../qpluginbase.pri)

target.path = $$[QT_INSTALL_PLUGINS]/mousedrivers
INSTALLS += target

DEFINES += QT_QWS_MOUSE_YOPY

HEADERS	= $$QT_SOURCE_TREE/src/gui/embedded/qmouseyopy_qws.h

SOURCES	= main.cpp \
	$$QT_SOURCE_TREE/src/gui/embedded/qmouseyopy_qws.cpp

