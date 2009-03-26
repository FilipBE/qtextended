TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=v4lwebcams

QTOPIA*=video

PLUGIN_FOR=qtopia
PLUGIN_TYPE=cameras

HEADERS = \
        webcams.h\
        plugin.h

SOURCES = \
        webcams.cpp \
        plugin.cpp 

