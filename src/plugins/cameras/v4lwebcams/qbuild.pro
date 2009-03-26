TEMPLATE=plugin
TARGET=v4lwebcams

PLUGIN_FOR=qtopia
PLUGIN_TYPE=cameras

CONFIG+=qtopia singleexec
QTOPIA*=video

pkg [
    name=v4lwebcams-camera
    desc="Video4Linux webcam camera plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    webcams.h\
    plugin.h

SOURCES=\
    webcams.cpp\
    plugin.cpp

