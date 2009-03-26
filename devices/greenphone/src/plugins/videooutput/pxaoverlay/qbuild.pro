TEMPLATE=plugin
CONFIG+=qtopia
TARGET=pxaoverlay

PLUGIN_TYPE=videooutput
PLUGIN_FOR=qtopia

QTOPIA*=media gfx video

HEADERS=\
    qpxavideooutput.h \
    pxaoverlay.h \

SOURCES=\
    qpxavideooutput.cpp \
    pxaoverlay.cpp \

