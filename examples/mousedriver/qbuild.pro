requires(qws)
TEMPLATE=plugin
PLUGIN_FOR=qt
PLUGIN_TYPE=mousedrivers

TARGET=examplemousehandler

CONFIG+=qtopia
QTOPIA=base

HEADERS=\
    examplemousedriverplugin.h\
    examplemousehandler.h

SOURCES=\
    examplemousedriverplugin.cpp\
    examplemousehandler.cpp

