TEMPLATE=plugin
TARGET=cruxus

PLUGIN_FOR=qtopia
PLUGIN_TYPE=mediaengines

CONFIG+=qtopia singleexec
QTOPIA*=media audio

pkg [
    name=cruxus-mediaengine
    desc="Cruxus media engine plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

equals(QTOPIA_SOUND_SYSTEM,oss):DEFINES+=HAVE_OSS

HEADERS=\
    contentdevice.h\
    cruxusengine.h\
    cruxusenginefactory.h\
    cruxusurisessionbuilder.h\
    cruxussimplesession.h\
    cruxusurihandlers.h\
    cruxusoutputdevices.h\
    cruxusoutputthread.h\
    audioresampler.h

SOURCES=\
    contentdevice.cpp\
    cruxusengine.cpp\
    cruxusenginefactory.cpp\
    cruxusurisessionbuilder.cpp\
    cruxussimplesession.cpp\
    cruxusurihandlers.cpp\
    cruxusoutputdevices.cpp\
    cruxusoutputthread.cpp\
    audioresampler.cpp

