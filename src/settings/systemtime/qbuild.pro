TEMPLATE=app
TARGET=systemtime

CONFIG+=qtopia singleexec quicklaunch

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=systemtime
    desc="Date/time settings for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    settime.h

SOURCES=\
    settime.cpp\
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

timeservice [
    hint=image
    files=services/Time/systemtime
    path=/services/Time
]

dateservice [
    hint=image
    files=services/Date/systemtime
    path=/services/Date
]

desktop [
    hint=desktop
    files=systemtime.desktop
    path=/apps/Settings
]

pics [
    hint=pics
    files=pics/*
    path=/pics/systemtime
]

