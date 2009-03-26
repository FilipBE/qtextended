requires(enable_qtopiamedia)
TEMPLATE=plugin
TARGET=voicemailviewer

PLUGIN_FOR=qtopia
PLUGIN_TYPE=viewers

CONFIG+=qtopia singleexec
QTOPIA*=mail pim media
equals(QTOPIA_UI,home):MODULES*=homeui

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

pkg [
    name=voicemail-viewer
    desc="Voicemail viewer plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    voicemailviewer.h

SOURCES=\
    voicemailviewer.cpp

