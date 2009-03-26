TEMPLATE=plugin
TARGET=locationplugin

PLUGIN_FOR=qtopia
PLUGIN_TYPE=whereabouts

CONFIG+=qtopia singleexec no_tr
QTOPIA+=whereabouts

HEADERS = locationplugin.h
SOURCES = locationplugin.cpp
