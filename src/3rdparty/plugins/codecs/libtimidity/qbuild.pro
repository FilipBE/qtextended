TEMPLATE=plugin
TARGET=timidityplugin

PLUGIN_FOR=qtopia
PLUGIN_TYPE=codecs

CONFIG+=qtopia singleexec
QTOPIA+=media
MODULES*=libtimidity

HEADERS	= midiplugin.h mididecoder.h
SOURCES	= midiplugin.cpp mididecoder.cpp

