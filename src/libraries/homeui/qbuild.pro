TEMPLATE=lib
TARGET=homeui
MODULE_NAME=homeui
VERSION=4.0.0

CONFIG+=qtopia hide_symbols singleexec
QTOPIA*=pim

pkg [
    name=libhomeui
    desc="Home UI library for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    qribbonselector.h

SEMI_PRIVATE_HEADERS=\
    homewidgets_p.h

SOURCES=\
    homewidgets_p.cpp\
    qribbonselector.cpp

# Install rules

pics [
    hint=pics
    files=pics/*
    path=/pics/home
]

