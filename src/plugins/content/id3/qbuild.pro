TEMPLATE=plugin
TARGET=id3

PLUGIN_FOR=qtopia
PLUGIN_TYPE=content

CONFIG+=qtopia singleexec

pkg [
    name=id3-content
    desc="ID3 content plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    id3contentplugin.h\
    id3tag.h

SOURCES=\
    id3contentplugin.cpp\
    id3tag.cpp

