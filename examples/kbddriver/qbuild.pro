requires(qws)
TEMPLATE=plugin
PLUGIN_FOR=qt
PLUGIN_TYPE=kbddrivers

TARGET=examplekbdhandler

CONFIG+=qtopia
QTOPIA=base

HEADERS=\
    examplekbddriverplugin.h\
    examplekbdhandler.h

SOURCES=\
    examplekbddriverplugin.cpp\
    examplekbdhandler.cpp

