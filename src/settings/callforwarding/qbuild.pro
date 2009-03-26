TEMPLATE=app
TARGET=callforwarding

CONFIG+=qtopia singleexec quicklaunch
QTOPIA*=pim

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=callforwarding
    desc="Call forwarding settings for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    callforwarding.h

SOURCES=\
    callforwarding.cpp\
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

service [
    hint=image
    files=services/CallForwarding/callforwarding
    path=/services/CallForwarding
]

desktop [
    hint=desktop
    files=callforwarding.desktop
    path=/apps/Settings
]

pics [
    hint=pics
    files=pics/*
    path=/pics/callforwarding
]

captureService [
    hint=image
    files=services/Settings/callforwarding
    path=/services/Settings
]

