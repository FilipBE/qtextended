TEMPLATE=app
TARGET=quicklauncher

CONFIG+=qtopia singleexec
contains(QTOPIA_MODULES,pim):QTOPIA*=pim

pkg [
    name=quicklauncher
    desc="Quicklauncher stub for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

equals(QTOPIA_SETPROC_METHOD,prctl):DEFINES+=QTOPIA_SETPROC_PRCTL
equals(QTOPIA_SETPROC_METHOD,argv0):DEFINES+=QTOPIA_SETPROC_ARGV0

HEADERS=\
    quicklaunch.h

SOURCES=\
    quicklaunch.cpp\
    main.cpp

# Install rules

target [
    hint=sxe
    domain=trusted
]

