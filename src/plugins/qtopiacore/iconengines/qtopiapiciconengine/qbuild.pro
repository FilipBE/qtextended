TEMPLATE=plugin
TARGET=qtopiapiciconengine

PLUGIN_FOR=qt
PLUGIN_TYPE=iconengines

CONFIG+=qtopia singleexec
QTOPIA=base

pkg [
    name=pic-iconengine
    desc="PIC icon engine plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    piciconengine.h

SOURCES=\
    piciconengine.cpp

