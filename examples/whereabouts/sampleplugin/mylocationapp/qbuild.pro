TEMPLATE=app
CONFIG+=qtopia no_tr
TARGET=mylocationapp

QTOPIA+=whereabouts

HEADERS=mylocationapp.h
SOURCES=mylocationapp.cpp \
        main.cpp

desktop.files=mylocationapp.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

# Package information (used for make packages)
pkg.name=mylocationapp
pkg.desc=My Location App
pkg.domain=trusted

# Install pictures.
pics.files=pics/*
pics.path=/pics/mylocationapp
pics.hint=pics
INSTALLS+=pics
