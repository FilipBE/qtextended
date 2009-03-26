TEMPLATE=app
TARGET=logging

CONFIG+=qtopia singleexec quicklaunch

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=logging
    desc="Logging settings for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    loggingedit.h\
    loggingview.h

SOURCES=\
    loggingedit.cpp\
    loggingview.cpp\
    main.cpp

# Install rules

target [
    hint=sxe
    domain=trusted
]

desktop [
    hint=desktop
    files=logging.desktop
    path=/apps/Settings
]

pics [
    hint=pics
    files=pics/*
    path=/pics/logging
]

