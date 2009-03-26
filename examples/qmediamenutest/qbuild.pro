TEMPLATE  = app
TARGET    = qmediamenutest
CONFIG   += qtopia

QTOPIA   *= media

AVAILABLE_LANGUAGES=en_US
LANGUAGES=$$AVAILABLE_LANGUAGES

# Input
HEADERS += qmediamenutest.h
SOURCES += qmediamenutest.cpp main.cpp

desktop.files=qmediamenutest.desktop
desktop.path=/apps/Applications
desktop.trtarget=qmediamenutest-nct
desktop.hint=nct desktop
INSTALLS+=desktop

pics.files=pics/*
pics.path=/pics/qmediamenutest
pics.hint=pics
INSTALLS+=pics

# SXE permissions required
pkg.domain=trusted
pkg.name=qmediamenutest
pkg.desc=This is a test program used to demonstrate QMediaMenu class
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=GPL
