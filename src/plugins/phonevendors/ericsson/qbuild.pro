TEMPLATE=plugin
TARGET=ericssonvendor

PLUGIN_FOR=qtopia
PLUGIN_TYPE=phonevendors

CONFIG+=qtopia singleexec
QTOPIA+=phonemodem

pkg [
    name=ericsson-phonevendor
    desc="Ericsson phonevendor plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    vendor_ericsson_p.h\
    ericssonplugin.h

SOURCES=\
    vendor_ericsson.cpp\
    ericssonplugin.cpp

