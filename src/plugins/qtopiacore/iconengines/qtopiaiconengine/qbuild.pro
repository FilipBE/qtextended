TEMPLATE=plugin
TARGET=qtopiaiconengine

PLUGIN_FOR=qt
PLUGIN_TYPE=iconengines

CONFIG+=qt embedded
CONFIG+=singleexec

pkg [
    name=qtextended-iconengine
    desc="Qt Extended icon engine plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

SOURCES=\
    qtopiaiconengine.cpp

