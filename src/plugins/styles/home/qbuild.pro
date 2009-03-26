TEMPLATE=plugin
TARGET=homestyle

PLUGIN_TYPE=styles
PLUGIN_FOR=qt

CONFIG+=qtopia singleexec
QTOPIA+=gfx
MODULES*=homeui

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

pkg [
    name=home-style
    desc="Home style plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    homestyle.h

SOURCES=\
    homestyle.cpp

