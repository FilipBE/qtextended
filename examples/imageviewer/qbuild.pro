TEMPLATE=app
CONFIG+=qtopia
TARGET=iviewer

CONFIG+=quicklaunch

HEADERS += iviewer.h 
SOURCES += main.cpp iviewer.cpp
SOURCES += listscreen.cpp
HEADERS += listscreen.h
SOURCES += imagescreen.cpp
HEADERS += imagescreen.h
SOURCES += inputdialog.cpp
HEADERS += inputdialog.h
SOURCES += infoscreen.cpp
HEADERS += infoscreen.h

desktop.files=imageviewer.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

# Package information (used for make packages)
pkg.name=iviewer
pkg.desc=Image Viewer Application
#pkg.version=1.0.0-1
#pkg.maintainer=Qt Extended <info@qtextended.org>
#pkg.license=Commercial
pkg.domain=trusted


STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
