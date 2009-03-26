TEMPLATE=plugin
TARGET=phonon_qtopiamedia

PLUGIN_FOR=qt
PLUGIN_TYPE=phonon_backend

CONFIG+=qtopia singleexec
QTOPIA*=media
QT+=phonon

pkg [
    name=qtextendedmedia-phonon
    desc="Qt Extended media for phonon plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    backend.h\
    mediaobject.h\
    audiooutput.h\
    videowidget.h

SOURCES=\
    backend.cpp\
    mediaobject.cpp\
    audiooutput.cpp\
    videowidget.cpp

