TEMPLATE=plugin
TARGET=perftestqvfbscreen

PLUGIN_FOR=qt
PLUGIN_TYPE=gfxdrivers

CONFIG+=qtopia singleexec

pkg [
    name=perftestqvfb-gfxdriver
    desc="QVFb performance test gfx plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    perftestqvfbscreendriverplugin.h\
    perftestqvfbscreen.h

SOURCES=\
    perftestqvfbscreendriverplugin.cpp\
    perftestqvfbscreen.cpp

