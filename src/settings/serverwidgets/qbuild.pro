TEMPLATE=app
TARGET=serverwidgets

CONFIG+=qtopia singleexec quicklaunch

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=serverwidgets
    desc="Server widget settings for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    serverwidgets.h

SOURCES=\
    serverwidgets.cpp\
    main.cpp

# Install rules

target [
    hint=sxe
    domain=trusted
]

desktop [
    hint=desktop
    files=serverwidgets.desktop
    path=/apps/Settings
]

pics [
    hint=pics
    files=pics/*
    path=/pics/serverwidgets
]

