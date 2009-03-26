TEMPLATE=app
TARGET=appservices

CONFIG+=qtopia singleexec quicklaunch

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=appservices
    desc="Allows you to choose which application provides a service."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    appservices.h\
    applist.h\
    appdetails.h\
    mimetypes.h\
    itemfactory.h

SOURCES=\
    appservices.cpp\
    applist.cpp\
    appdetails.cpp\
    mimetypes.cpp\
    itemfactory.cpp\
    main.cpp

# Install rules

target [
    hint=sxe
    domain=trusted
]

pics [
    hint=pics
    files=pics/*
    path=/pics/appservices
]

desktop [
    hint=desktop
    files=appservices.desktop
    path=/apps/Settings
]

help [
    hint=help
    source=help
    files=*.html
]

