TEMPLATE=app
CONFIG+=qtopia
TARGET=videoconf

CONFIG+=quicklaunch singleexec

MODULES *= qtopiamedia \
           qtopiavideo

# Input
HEADERS += conferencewidget.h payloadmodel.h callcontrols.h
SOURCES += conferencewidget.cpp \
           main.cpp \
           payloadmodel.cpp \
           callcontrols.cpp

desktop.files=videoconf.desktop
desktop.path=/apps/Applications
desktop.trtarget=videoconf-nct
desktop.hint=nct desktop
INSTALLS+=desktop

pics.files=pics/*
pics.path=/pics/videoconf
pics.hint=pics
INSTALLS+=pics

# Set this to trusted for full privileges
target.hint=sxe
target.domain=trusted

#No translations for this example
#STRING_LANGUAGE=en_US
#AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
#LANGUAGES=$$QTOPIA_LANGUAGES
