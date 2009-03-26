TEMPLATE=app
TARGET=security

CONFIG+=qtopia singleexec quicklaunch
enable_cell:QTOPIA*=phone

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=security
    desc="Security settings for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    security.h

SOURCES=\
    security.cpp\
    main.cpp

CELL [
    TYPE=CONDITIONAL_SOURCES
    CONDITION=enable_cell
    FORMS=securityphone.ui
    HEADERS=phonesecurity.h
    SOURCES=phonesecurity.cpp
]

NONCELL [
    TYPE=CONDITIONAL_SOURCES
    CONDITION=!enable_cell
    FORMS=securitybase.ui
]

# Install rules

target [
    hint=sxe
    domain=trusted
]

pics [
    hint=pics
    files=pics/*
    path=/pics/security
]

desktop [
    hint=desktop
    files=security.desktop
    path=/apps/Settings
]

help [
    hint=help
    source=help
    files=*.html
]

