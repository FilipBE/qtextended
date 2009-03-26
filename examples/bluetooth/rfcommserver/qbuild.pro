TEMPLATE=app
CONFIG+=qtopia no_tr
TARGET=rfcommserver

QTOPIA+=comm

HEADERS=rfcommserver.h
SOURCES=rfcommserver.cpp \
        main.cpp

desktop.files=rfcommserver.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

# Package information (used for make packages)
pkg.name=rfcommserver
pkg.desc=Bluetooth RFCOMM server example
pkg.domain=trusted

requires(enable_bluetooth)

