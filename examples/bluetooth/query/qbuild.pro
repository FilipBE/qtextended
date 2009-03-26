TEMPLATE=app
CONFIG+=qtopia no_tr
TARGET=query

QTOPIA+=comm

HEADERS=query.h
SOURCES=query.cpp \
        main.cpp

desktop.files=query.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

# Package information (used for make packages)
pkg.name=query
pkg.desc=Bluetooth SDP query example
pkg.domain=trusted

requires(enable_bluetooth)

