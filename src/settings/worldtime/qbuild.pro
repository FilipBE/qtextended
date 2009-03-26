TEMPLATE=app
TARGET=worldtime

CONFIG+=qtopia singleexec quicklaunch

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=worldtime
    desc="Time-zone settings for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    worldtime.h\
    cityinfo.h

SOURCES=\
    worldtime.cpp\
    main.cpp\
    cityinfo.cpp

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
    path=/pics/worldtime
]

desktop [
    hint=desktop
    files=worldtime.desktop
    path=/apps/Applications
]

