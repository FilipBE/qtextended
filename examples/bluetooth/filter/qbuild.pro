TEMPLATE=app
CONFIG+=qtopia no_tr
TARGET=filter

QTOPIA+=comm

HEADERS=filter.h
SOURCES=filter.cpp \
        main.cpp

desktop.files=filter.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

# Package information (used for make packages)
pkg.name=filter
pkg.desc=Bluetooth device filter example
pkg.domain=trusted

requires(enable_bluetooth)

