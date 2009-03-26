TEMPLATE=plugin
TARGET=threegpp

PLUGIN_FOR=qtopia
PLUGIN_TYPE=content

CONFIG+=qtopia singleexec

pkg [
    name=3gpp-content
    desc="3GPP content plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    threegppcontentplugin.h

SOURCES=\
    threegppcontentplugin.cpp

