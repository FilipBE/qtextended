TEMPLATE  = app
TARGET    = filtering2
CONFIG   += qtopia

QTOPIA   *= media

AVAILABLE_LANGUAGES=en_US
LANGUAGES=$$AVAILABLE_LANGUAGES

CONFIG+=no_tr

# Input
HEADERS = filterdemo2.h
SOURCES = filterdemo2.cpp main.cpp

desktop.files=filtering2.desktop
desktop.path=/apps/Applications
desktop.trtarget=filtering2-nct
desktop.hint=nct desktop
INSTALLS+=desktop

pics.files=pics/*
pics.path=/pics/filtering2
pics.hint=pics
INSTALLS+=pics

# SXE permissions required
pkg.domain=trusted
pkg.name=filtering2
pkg.desc=This is a command line tool used to demonstrate QMediaList class
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=GPL
