TEMPLATE=app
CONFIG+=qtopia no_quicklaunch
TARGET=messagenavigator

# We use the PIM and messaging libraries
QTOPIA*=pim mail

# These are the source files that get built to create the application
FORMS=messagenavigatorbase.ui
HEADERS=messagenavigator.h foldermodel.h
SOURCES=main.cpp messagenavigator.cpp foldermodel.cpp

# Install the launcher item. The metadata comes from the .desktop file
# and the path here.
desktop.files=messagenavigator.desktop
desktop.path=/apps/Applications
desktop.trtarget=messagenavigator-nct
desktop.hint=nct desktop
INSTALLS+=desktop

# Install some pictures.
pics.files=pics/*
pics.path=/pics/messagenavigator
pics.hint=pics
INSTALLS+=pics

# Install help files
help.source=help
help.files=messagenavigator.html
help.hint=help
INSTALLS+=help

# Package information (used for make packages)
pkg.name=messagenavigator
pkg.desc=Message Navigator Application
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=Commercial
pkg.domain=trusted
STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

