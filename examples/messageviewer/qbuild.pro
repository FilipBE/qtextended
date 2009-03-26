TEMPLATE=app
CONFIG+=qtopia no_quicklaunch
TARGET=messageviewer

# We use the PIM and messaging libraries
QTOPIA*=pim mail

# These are the source files that get built to create the application
FORMS=messageviewerbase.ui
HEADERS=messageviewer.h messagemodel.h messagedelegate.h
SOURCES=main.cpp messageviewer.cpp messagemodel.cpp messagedelegate.cpp

# Install the launcher item. The metadata comes from the .desktop file
# and the path here.
desktop.files=messageviewer.desktop
desktop.path=/apps/Applications
desktop.trtarget=messageviewer-nct
desktop.hint=nct desktop
INSTALLS+=desktop

# Install some pictures.
pics.files=pics/*
pics.path=/pics/messageviewer
pics.hint=pics
INSTALLS+=pics

# Install help files
help.source=help
help.files=messageviewer.html
help.hint=help
INSTALLS+=help

# Package information (used for make packages)
pkg.name=messageviewer
pkg.desc=Message Viewer Application
pkg.version=1.0.0-1
pkg.maintainer=Qt Extended <info@qtextended.org>
pkg.license=Commercial
pkg.domain=trusted

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

