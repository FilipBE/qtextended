TEMPLATE=plugin
TARGET=dialerlineedit

PLUGIN_FOR=qtopia
PLUGIN_TYPE=themeitems

CONFIG+=qtopia singleexec
QTOPIA+=theming

pkg [
    name=dialerlineedit-themeitem
    desc="Dialer lineedit theme item plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    dialerlineedit.h

SOURCES=\
    dialerlineedit.cpp

