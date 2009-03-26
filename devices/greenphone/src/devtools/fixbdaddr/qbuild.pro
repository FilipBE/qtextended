TEMPLATE=app
CONFIG+=qtopia
TARGET=fixbdaddr

QTOPIA*=phone
CONFIG+=quicklaunch

HEADERS		= fixbdaddr.h
SOURCES		= fixbdaddr.cpp main.cpp

desktop.files=fixbdaddr.desktop
desktop.path=/apps/Devtools
desktop.hint=desktop
INSTALLS+=desktop

pkg.desc=Greenphone unique bdaddr fixer
pkg.domain=trusted

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

