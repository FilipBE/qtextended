TEMPLATE  = app
TARGET    = qaudiomixertest
CONFIG   += qtopia

QTOPIA   *= audio

AVAILABLE_LANGUAGES=en_US
LANGUAGES=$$AVAILABLE_LANGUAGES

FORMS=qaudiomixertestbase.ui
HEADERS=qaudiomixertest.h
SOURCES=main.cpp qaudiomixertest.cpp

desktop.files=qaudiomixertest.desktop
desktop.path=/apps/Applications
desktop.trtarget=qaudiomixertest-nct
desktop.hint=nct desktop
INSTALLS+=desktop

pics.files=pics/*
pics.path=/pics/qaudiomixertest
pics.hint=pics
INSTALLS+=pics

pkg.name=qaudiomixertest
pkg.desc=QAudioMixer Test Application
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=Commercial
pkg.domain=trusted
