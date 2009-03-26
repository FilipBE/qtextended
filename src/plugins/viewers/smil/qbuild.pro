requires(enable_cell)
TEMPLATE=plugin
TARGET=smilviewer

PLUGIN_FOR=qtopia
PLUGIN_TYPE=viewers

CONFIG+=qtopia singleexec
QTOPIA*=mail smil

pkg [
    name=smil-viewer
    desc="SMIL viewer plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    smilviewer.h

SOURCES=\
    smilviewer.cpp

