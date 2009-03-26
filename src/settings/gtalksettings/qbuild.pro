requires(enable_voip)
TEMPLATE=app
TARGET=gtalksettings

CONFIG+=qtopia quicklaunch singleexec
QTOPIA*=phone

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=gtalksettings
    desc="Google talk settings for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

FORMS=\
    gtalksettingsbase.ui

HEADERS=\
    gtalksettings.h

SOURCES=\
    gtalksettings.cpp\
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

desktop [
    hint=desktop
    files=gtalksettings.desktop
    path=/apps/Settings
]

pics [
    hint=pics
    files=pics/*
    path=/pics/gtalksettings
]

