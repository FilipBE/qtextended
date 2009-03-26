TEMPLATE=plugin
TARGET=bscidrmagent

PLUGIN_FOR=qtopia
PLUGIN_TYPE=drmagent

CONFIG+=qtopia singleexec
MODULES*=drmagent

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

pkg [
    name=bsci-drm
    desc="BSCI drm plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    bscidrmcontentplugin.h\
    bscidrm.h\
    bscirightsmanager.h\
    bscidrmagentservice.h\
    bscidrmagentplugin.h\
    bscifileengine.h\
    bsciprompts.h

SOURCES=\
    bscidrmcontentplugin.cpp\
    bscidrm.cpp\
    bscirightsmanager.cpp\
    bscidrmagentservice.cpp\
    bscidrmagentplugin.cpp\
    bscifileengine.cpp\
    bsciprompts.cpp

# Install rules

pki [
    hint=image
    files=etc/bscidrm/*
    path=/etc/bscidrm/
]

