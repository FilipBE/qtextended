TEMPLATE  = app
TARGET    = simpleplayer
CONFIG   += qtopia
QTOPIA   *= media

FORMS=simpleplayerbase.ui
HEADERS=simpleplayer.h basicmedia.h
SOURCES=main.cpp simpleplayer.cpp basicmedia.cpp

desktop.files=simpleplayer.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

pics.files=pics/*
pics.path=/pics
pics.hint=pics
INSTALLS+=pics

pkg.name=SimplePlayer
pkg.desc=Simple Media Player Application
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=Commercial
pkg.domain=window
AVAILABLE_LANGUAGES=en_US
LANGUAGES=$$AVAILABLE_LANGUAGES
