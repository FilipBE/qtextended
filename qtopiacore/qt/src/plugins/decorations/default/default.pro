TARGET	 = qdecorationdefault
include(../../qpluginbase.pri)

HEADERS	= $$QT_SOURCE_TREE/src/gui/embedded/qdecorationdefault_qws.h
SOURCES	= main.cpp \
	  $$QT_SOURCE_TREE/src/gui/embedded/qdecorationdefault_qws.cpp

target.path += $$[QT_INSTALL_PLUGINS]/decorations
INSTALLS += target
