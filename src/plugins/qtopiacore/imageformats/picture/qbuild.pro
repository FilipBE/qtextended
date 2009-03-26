TEMPLATE=plugin
TARGET=pictureimageformat

PLUGIN_FOR=qt
PLUGIN_TYPE=imageformats

CONFIG+=qtopia singleexec
QTOPIA=base

pkg [
    name=pic-imageformat
    desc="PIM image format plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    pictureiohandler.h

SOURCES=\
    pictureiohandler.cpp

