TEMPLATE=plugin
TARGET=wavecomvendor

PLUGIN_FOR=qtopia
PLUGIN_TYPE=phonevendors

CONFIG+=qtopia singleexec
QTOPIA+=phonemodem

pkg [
    name=wavecom-phonevendor
    desc="Wavecom phonevendor plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    vendor_wavecom_p.h\
    wavecomplugin.h

SOURCES=\
    vendor_wavecom.cpp\
    wavecomplugin.cpp

