TEMPLATE=app
CONFIG+=qtopia no_tr
TARGET=devinfo

QTOPIA+=comm

HEADERS=devinfo.h
SOURCES=devinfo.cpp \
        main.cpp

desktop.files=devinfo.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

# Package information (used for make packages)
pkg.name=devinfo
pkg.desc=Bluetooth Device Info
pkg.domain=trusted

requires(enable_bluetooth)

