TEMPLATE=lib
TARGET=webliteclient
MODULE_NAME=webliteclient

CONFIG+=qtopia
QTOPIA=base

requires(equals(QTOPIA_UI,home))

pkg [
    name=webliteclient
    desc="WebLite engine client library for Qt Extended."
    version=1.0.0-1
    license="GPL v2"
    maintainer="Qt Extended <info@qtextended.org>"
]

SOURCEPATH=..

HEADERS=\
    webliteclient.h\
    webliteimg.h\
    weblitecore.h
    
SOURCES=\
    webliteclient.cpp\
    webliteimg.cpp

