TEMPLATE=plugin
TARGET=videomailcomposer

PLUGIN_FOR=qtopia
PLUGIN_TYPE=composers

CONFIG+=qtopia singleexec
QTOPIA*=mail media
MODULES*=homeui

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

pkg [
    name=videomail-composer
    desc="Videomail composer plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    videomailcomposer.h

SOURCES=\
    videomailcomposer.cpp

