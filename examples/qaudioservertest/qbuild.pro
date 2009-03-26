TEMPLATE  = app
TARGET    = qaudioservertest
CONFIG   += qtopia

QTOPIA   *= audio

AVAILABLE_LANGUAGES=en_US
LANGUAGES=$$AVAILABLE_LANGUAGES

FORMS=qaudioservertestbase.ui
HEADERS=qaudioservertest.h 
SOURCES=main.cpp qaudioservertest.cpp

desktop.files=qaudioservertest.desktop
desktop.path=/apps/Applications
desktop.trtarget=qaudioservertest-nct
desktop.hint=nct desktop
INSTALLS+=desktop

pics.files=pics/*
pics.path=/pics/qaudioservertest
pics.hint=pics
INSTALLS+=pics

pkg.name=qaudioservertest
pkg.desc=Audio Server Test Application
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=Commercial
pkg.domain=window
