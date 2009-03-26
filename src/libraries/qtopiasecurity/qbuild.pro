requires(enable_sxe)
TEMPLATE=lib
TARGET=qtopiasecurity
MODULE_NAME=qtopiasecurity
VERSION=4.0.0

CONFIG+=qtopia hide_symbols singleexec
QTOPIA=base

pkg [
    name=securitylib
    desc="Security library for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    qsxepolicy.h\
    qpackageregistry.h

PRIVATE_HEADERS=\
    keyfiler_p.h

SOURCES=\
    qsxepolicy.cpp\
    keyfiler.cpp\
    qpackageregistry.cpp

