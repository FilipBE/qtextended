TEMPLATE=app
CONFIG+=qtopia no_tr
TARGET=scanner

QTOPIA+=comm

HEADERS=scanner.h
SOURCES=scanner.cpp \
        main.cpp

desktop.files=scanner.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

# Package information (used for make packages)
pkg.name=scanner
pkg.desc=Bluetooth device discovery example
pkg.domain=trusted

requires(enable_bluetooth)

