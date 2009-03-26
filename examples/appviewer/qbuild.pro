TEMPLATE=app
CONFIG+=qtopia
TARGET=appviewer

CONFIG+=no_quicklaunch no_singleexec

HEADERS=appviewer.h
SOURCES=main.cpp appviewer.cpp

desktop.files=appviewer.desktop
desktop.path=/apps/Applications
desktop.trtarget=textviewer-nct
desktop.hint=nct desktop
INSTALLS+=desktop

pkg.name=appviewer
pkg.desc=An Example Program to use the Content system and System paths
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=GPL
pkg.domain=trusted

#we don't want translations for this example
#STRING_LANGUAGE=en_US
#AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
#LANGUAGES=$$QTOPIA_LANGUAGES
