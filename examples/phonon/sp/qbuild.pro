
TEMPLATE = app

CONFIG += qtopia no_quicklaunch no_singleexec no_tr qtopia_main

TARGET = sp

HEADERS = \
            mainwindow.h

SOURCES = \
            main.cpp \
            mainwindow.cpp

QT+=phonon

pkg.name = sp
pkg.desc = Simple Phonon Test App
pkg.domain = trusted

