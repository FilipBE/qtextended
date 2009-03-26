TEMPLATE=app
TARGET=hwsettings

CONFIG+=qtopia quicklaunch singleexec
MODULES*=handwriting

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=hwsettings
    desc="Handwriting settings for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

FORMS=\
    gprefbase.ui\
    charseteditbase.ui

HEADERS=\
    pensettingswidget.h\
    mainwindow.h\
    charsetedit.h\
    uniselect.h

SOURCES=\
    pensettingswidget.cpp\
    main.cpp\
    mainwindow.cpp\
    charsetedit.cpp\
    uniselect.cpp

# Install rules

target [
    hint=sxe
    domain=trusted
]

desktop [
    hint=desktop
    files=hwsettings.desktop
    path=/apps/Settings
]

pics [
    hint=pics
    files=pics/*
    path=/pics/hwsettings
]

help [
    hint=help
    source=help
    files=*.html
]

EXTRA_TS_FILES=QtopiaHandwriting

