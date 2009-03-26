TEMPLATE=app
TARGET=webliteserver

CONFIG+=qtopia
MODULES+=webliteclient

requires(equals(QTOPIA_UI,home))

pkg [
    name=webliteserver
    desc="WebLite Engine."
    version=1.0.0-1
    license="GPL v2"
    maintainer="Qt Extended <info@qtextended.org>"
]

HEADERS=\
    webliteserver.h
    
SOURCES=\
    webliteserver.cpp

settings [
    hint=image
    files=weblite-server.conf
    path=/etc/default/Trolltech
]

