TEMPLATE=app
TARGET=drmbrowser

CONFIG+=qtopia singleexec quicklaunch

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=drmbrowser
    desc="DRM application for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    drmbrowser.h

SOURCES=\
    main.cpp\
    drmbrowser.cpp

# Install rules

target [
    hint=sxe
    domain=trusted
]

desktop [
    hint=desktop
    files=drmbrowser.desktop
    path=/apps/Settings
]

help [
    hint=help
    source=help
    files=*.html
]

omadrmagentservice [
    hint=image
    files=services/OmaDrmAgent/drmbrowser
    path=/services/OmaDrmAgent
]

qdsservice [
    hint=image
    files=etc/qds/OmaDrmAgent
    path=/etc/qds
]

