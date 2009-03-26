TEMPLATE=app
CONFIG+=qtopia no_tr
TARGET=rfcommclient

QTOPIA+=comm

HEADERS=rfcommclient.h
SOURCES=rfcommclient.cpp \
        main.cpp

desktop.files=rfcommclient.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

# Package information (used for make packages)
pkg.name=rfcommclient
pkg.desc=Bluetooth RFCOMM client example
pkg.domain=trusted

requires(enable_bluetooth)

