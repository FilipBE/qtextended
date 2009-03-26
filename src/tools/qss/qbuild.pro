TEMPLATE=app
TARGET=mediaserver

CONFIG+=qtopia singleexec

pkg [
    name=mediaserver
    desc="Media server for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

SOURCES=\
    main.cpp

# Install rules

target [
    hint=sxe
    domain=trusted
]

