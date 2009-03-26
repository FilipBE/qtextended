TEMPLATE=plugin
TARGET=tremorplugin

PLUGIN_FOR=qtopia
PLUGIN_TYPE=codecs

CONFIG+=qtopia singleexec
QTOPIA+=media
MODULES*=tremor

HEADERS = \
        oggplugin.h \
        oggdecoder.h

SOURCES = \
        oggplugin.cpp \
        oggdecoder.cpp

