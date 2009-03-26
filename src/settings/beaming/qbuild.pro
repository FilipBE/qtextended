TEMPLATE=app
TARGET=beaming

CONFIG+=qtopia singleexec quicklaunch
QTOPIA*=comm

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=beaming
    desc="Beaming settings for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    ircontroller.h\
    beaming.h

SOURCES=\
    ircontroller.cpp\
    beaming.cpp\
    main.cpp

# Install rules

target [
    hint=sxe
    domain=trusted
]

help [
    hint=help
    source=help
    files=*.html
]

pics [
    hint=pics
    files=pics/*
    path=/pics/beaming
]

desktop [
    hint=desktop
    files=beaming.desktop
    path=/apps/Settings
]

beam [
    hint=image
    files=etc/beam
    path=/etc
]

