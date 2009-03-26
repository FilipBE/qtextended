TEMPLATE=plugin
TARGET=inputmethodsitem

PLUGIN_FOR=qtopia
PLUGIN_TYPE=themeitems

CONFIG+=qtopia singleexec
QTOPIA+=theming
# This uses symbols from the server!
SOURCEPATH+=/src/server/core_server
CONFIG-=link_test

pkg [
    name=inputmethods-themeitem
    desc="Inputmethods theme item plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    inputmethodsitem.h

SOURCES=\
    inputmethodsitem.cpp

