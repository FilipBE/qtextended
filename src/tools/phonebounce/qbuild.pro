TEMPLATE=app
TARGET=phonebounce

CONFIG+=embedded

pkg [
    name=phonebounce
    desc="Phone bounce utility for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

SOURCES=\
    phonebounce.cpp

