TEMPLATE=app
TARGET=profileedit

CONFIG+=qtopia quicklaunch singleexec

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=profileedit
    desc="Profile settings for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    ringprofile.h\
    ringtoneeditor.h

SOURCES=\
    main.cpp\
    ringprofile.cpp\
    ringtoneeditor.cpp

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
    files=profileedit.desktop
    path=/apps/Settings
]

pics [
    hint=pics
    files=pics/*
    path=/pics/profileedit
]

service [
    hint=image
    files=services/Profiles/profileedit
    path=/services/Profiles
]

serviceb [
    hint=image
    files=services/SettingsManager/profileedit
    path=/services/SettingsManager
]

