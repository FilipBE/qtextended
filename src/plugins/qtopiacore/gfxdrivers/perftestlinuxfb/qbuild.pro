TEMPLATE=plugin
TARGET=perftestlinuxfbscreen

PLUGIN_FOR=qt
PLUGIN_TYPE=gfxdrivers

CONFIG+=qtopia singleexec

pkg [
    name=perftestfb-gfxdriver
    desc="Linux FB performance test gfx plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    perftestlinuxfbscreendriverplugin.h\
    perftestlinuxfbscreen.h

SOURCES=\
    perftestlinuxfbscreendriverplugin.cpp\
    perftestlinuxfbscreen.cpp

