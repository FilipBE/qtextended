TEMPLATE=app
TARGET=language

CONFIG+=qtopia quicklaunch singleexec

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=language
    desc="Language settings for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

FORMS=\
    languagesettingsbase.ui

HEADERS=\
    languagesettings.h\
    langmodel.h

SOURCES=\
    language.cpp\
    main.cpp\
    langmodel.cpp

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

desktop [
    hint=desktop
    files=language.desktop
    path=/apps/Settings
]

pics [
    hint=pics
    files=pics/*
    path=/pics/language
]

