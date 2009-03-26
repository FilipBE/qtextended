TARGET = qgfxvnc
include(../../qpluginbase.pri)

DEFINES	+= QT_QWS_VNC

QTDIR_build:DESTDIR = $$QT_BUILD_TREE/plugins/gfxdrivers

HEADERS = \
	$$QT_SOURCE_TREE/src/gui/embedded/qscreenvnc_qws.h \
	$$QT_SOURCE_TREE/src/gui/embedded/qscreenvnc_p.h

SOURCES = main.cpp \
	$$QT_SOURCE_TREE/src/gui/embedded/qscreenvnc_qws.cpp


target.path += $$[QT_INSTALL_PLUGINS]/gfxdrivers
INSTALLS += target
