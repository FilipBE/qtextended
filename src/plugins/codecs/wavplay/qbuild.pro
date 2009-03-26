TEMPLATE=plugin
TARGET=wavplay

PLUGIN_FOR=qtopia
PLUGIN_TYPE=codecs

CONFIG+=qtopia singleexec
QTOPIA*=media
contains(PROJECTS,3rdparty/libraries/gsm) {
    MODULES*=gsm
    DEFINES+=WAVGSM_SUPPORTED
}

pkg [
    name=wavplay-codec
    desc="Wavplay codec plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

DEFINES+=WAV49 FAST SASR

HEADERS=\
    wavplugin.h\
    wavdecoder.h

SOURCES=\
    wavplugin.cpp\
    wavdecoder.cpp

