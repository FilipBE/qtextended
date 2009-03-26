TEMPLATE=app
TARGET=phonesettings

CONFIG+=qtopia singleexec quicklaunch
QTOPIA*=pim phone

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=phonesettings
    desc="Phone settings for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

FORMS=\
    channeledit.ui

HEADERS=\
    phonesettings.h

SOURCES=\
    phonesettings.cpp\
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
    files=phonesettings.desktop
    path=/apps/Settings
]

pics [
    hint=pics
    files=pics/*
    path=/pics/phonesettings
]

service [
    hint=image
    files=services/VoiceMail/phonesettings
    path=/services/VoiceMail
]

