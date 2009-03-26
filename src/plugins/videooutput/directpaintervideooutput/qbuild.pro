TEMPLATE=plugin
TARGET=directpaintervideooutput

PLUGIN_TYPE=videooutput
PLUGIN_FOR=qtopia

CONFIG+=qtopia singleexec
QTOPIA*=media gfx video

pkg [
    name=directpainter-videooutput
    desc="Direct painter video output plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    qdirectpaintervideooutput.h

SOURCES=\
    qdirectpaintervideooutput.cpp

