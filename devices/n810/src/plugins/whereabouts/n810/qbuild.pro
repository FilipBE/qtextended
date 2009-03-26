requires(enable_qtopiawhereabouts)
TEMPLATE=plugin
TARGET=n810gpsplugin

PLUGIN_FOR=qtopia
PLUGIN_TYPE=whereabouts

CONFIG+=qtopia singleexec
QTOPIA+=whereabouts

HEADERS	= n810gpsplugin.h
SOURCES = n810gpsplugin.cpp

