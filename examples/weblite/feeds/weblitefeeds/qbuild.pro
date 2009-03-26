TEMPLATE=lib
TARGET=weblitefeeds
MODULE_NAME=weblitefeeds

CONFIG+=qtopia
MODULES+=webliteclient

requires(equals(QTOPIA_UI,home))

pkg [
    name=weblitefeeds
    desc="WebLite feed parser library for Qt Extended."
    version=1.0.0-1
    license="GPL v2"
    maintainer="Qt Extended <info@qtextended.org>"
]

SOURCEPATH=..

HEADERS=\
    webfeed.h\
    mediafeed.h
    
SOURCES=\
    webfeed.cpp\
    mediafeed.cpp

