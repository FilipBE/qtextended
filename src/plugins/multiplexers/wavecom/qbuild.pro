TEMPLATE=plugin
TARGET=wavecommultiplex

PLUGIN_FOR=qtopia
PLUGIN_TYPE=multiplexers

CONFIG+=qtopia singleexec
QTOPIA*=comm

pkg [
    name=wavecom-multiplexer
    desc="Wavecom multiplexer plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    wavecommultiplexer.h

SOURCES=\
    wavecommultiplexer.cpp

