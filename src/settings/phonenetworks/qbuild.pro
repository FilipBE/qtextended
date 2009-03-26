TEMPLATE=app
TARGET=phonenetworks

CONFIG+=qtopia singleexec quicklaunch
QTOPIA*=pim phone collective

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=phonenetworks
    desc="Phone network settings for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    phonenetworks.h\
    modemnetwork.h\
    voipnetwork.h

SOURCES=\
    phonenetworks.cpp\
    main.cpp\
    modemnetwork.cpp\
    voipnetwork.cpp

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

service [
    hint=image
    files=services/CallNetworks/phonenetworks
    path=/services/CallNetworks
]

desktop [
    hint=desktop
    files=phonenetworks.desktop
    path=/apps/Settings
]

pics [
    hint=pics
    files=pics/*
    path=/pics/phonenetworks
]

settings [
    hint=image
    files=etc/default/Trolltech/GsmOperatorCountry.conf
    path=/etc/default/Trolltech
]

