TEMPLATE=plugin
TARGET=exif

PLUGIN_FOR=qtopia
PLUGIN_TYPE=content

CONFIG+=qtopia singleexec

pkg [
    name=exif-content
    desc="EXIF content plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    exifcontentplugin.h\
    exifcontentproperties.h

SOURCES=\
    exifcontentplugin.cpp\
    exifcontentproperties.cpp

