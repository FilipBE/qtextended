TEMPLATE=lib
TARGET=qtopiasmil
MODULE_NAME=qtopiasmil
VERSION=4.0.0

CONFIG+=qtopia hide_symbols singleexec
enable_qtopiamedia:QTOPIA*=media

pkg [
    name=smillib
    desc="SMIL library for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    smil.h\
    system.h\
    transfer.h\
    module.h\
    element.h\
    structure.h\
    content.h\
    layout.h\
    timing.h\
    media.h 

SOURCES=\
    smil.cpp\
    system.cpp\
    transfer.cpp\
    module.cpp\
    element.cpp\
    structure.cpp\
    content.cpp\
    layout.cpp\
    timing.cpp\
    media.cpp 

