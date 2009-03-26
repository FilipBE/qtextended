TEMPLATE=plugin
TARGET=qtopiasvgiconengine

PLUGIN_FOR=qt
PLUGIN_TYPE=iconengines

CONFIG+=qtopia singleexec
QTOPIA=base
QT+=svg

pkg [
    name=svg-iconengine
    desc="SVG icon engine plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    svgiconengine.h

SOURCES=\
    svgiconengine.cpp

