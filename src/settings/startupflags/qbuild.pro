TEMPLATE=app
TARGET=startupflags

CONFIG+=qtopia quicklaunch singleexec

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=startupflags
    desc="Startup settings for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    startupflags.h

SOURCES=\
    startupflags.cpp\
    main.cpp

# Install rules

target [
    hint=sxe
    domain=trusted
]

desktop [
    hint=desktop
    files=startupflags.desktop
    path=/apps/Settings
]

conf [
    hint=image
    files=etc/default/Trolltech/StartupFlags.conf
    path=/etc/default/Trolltech
]

script [
    hint=script
    files=startupflags.sh
    path=/bin
]

