requires(qws)
TEMPLATE=plugin
PLUGIN_FOR=qt
PLUGIN_TYPE=gfxdrivers

TARGET=examplescreen

CONFIG+=qtopia
QTOPIA=base

HEADERS=\
    examplescreendriverplugin.h\
    examplescreen.h

SOURCES=\
    examplescreendriverplugin.cpp\
    examplescreen.cpp

