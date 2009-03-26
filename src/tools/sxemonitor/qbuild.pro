TEMPLATE=app
TARGET=sxemonitor

CONFIG+=qtopia singleexec

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

pkg [
    name=sxemonitor
    desc="SXE monitor for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    sxemonitor.h\
    sxemonqlog.h

SOURCES=\
    main.cpp\
    sxemonitor.cpp\
    sxemonqlog.cpp

# Install rules

target [
    hint=sxe
    domain=trusted
]

