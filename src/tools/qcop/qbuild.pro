TEMPLATE=app
TARGET=qcop

CONFIG+=qtopia singleexec

pkg [
    name=qcop
    desc="QCop query and injection tool for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    qcopimpl.h

SOURCES=\
    main.cpp\
    qcopimpl.cpp 

# Install rules

target [
    hint=sxe
    domain=trusted
]

