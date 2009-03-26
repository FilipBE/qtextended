TEMPLATE=app
CONFIG+=qtopia
TARGET=musicplayer

QTOPIA*=media
CONFIG+=quicklaunch singleexec
MODULES*=homeui
MODULES*=pictureflow

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=musicplayer
    desc="Music player application for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    musicplayer.h\
    albumview.h\
    sidebar.h

SOURCES=\
    main.cpp\
    musicplayer.cpp\
    albumview.cpp\
    sidebar.cpp
 
# Install rules

target [
    hint=sxe
    domain=trusted
]

desktop [
    hint=desktop
    files=musicplayer.desktop
    path=/apps/Applications
]

pics [
    hint=pics
    files=pics/*
    path=/pics/musicplayer
]

service [
    hint=image
    files=services/MusicPlayer/musicplayer
    path=/services/MusicPlayer
]

