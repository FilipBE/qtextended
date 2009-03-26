TEMPLATE=app
TARGET=light-and-power

CONFIG+=qtopia quicklaunch singleexec

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=light-and-power
    desc="Light and power settings for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

FORMS=\
    lightsettingsbase.ui 

HEADERS=\
    light.h\
    minsecspinbox.h

SOURCES=\
    light.cpp\
    main.cpp\
    minsecspinbox.cpp

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
    files=light-and-power.desktop
    path=/apps/Settings
]

pics [
    hint=pics
    files=pics/*
    path=/pics/light-and-power
]

captureService [
    hint=image
    files=services/Settings/light-and-power
    path=/services/Settings
]

