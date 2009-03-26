TEMPLATE=app
TARGET=pimdatagui

CONFIG+=qtopia singleexec quicklaunch

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

pkg [
    name=pimdatagui
    desc="PIM data generator for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
    multi=tools/pimdatagui/pimdata
]

FORMS=\
    pimgen.ui

HEADERS=\
    pimgen.h

SOURCES=\
    main.cpp\
    pimgen.cpp

# Install rules

target [
    hint=sxe
    domain=trusted
]

desktop [
    hint=desktop nct
    trtarget=pimdatagui-nct
    files=pimdatagui.desktop
    path=/apps/Applications
]

