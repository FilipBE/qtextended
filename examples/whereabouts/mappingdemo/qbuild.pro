TEMPLATE=app
CONFIG+=qtopia
TARGET=mappingdemo

QTOPIA*=whereabouts
CONFIG+=quicklaunch

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES

HEADERS=mappingdemo.h
SOURCES=mappingdemo.cpp \
        main.cpp

desktop.files=mappingdemo.desktop
desktop.path=/apps/Applications
desktop.hint=desktop
INSTALLS+=desktop

# Package information (used for make packages)
pkg.name=mappingdemo
pkg.desc=Mapping Demo
pkg.domain=trusted

# Install pictures.
pics.files=pics/*
pics.path=/pics/mappingdemo
pics.hint=pics
INSTALLS+=pics

# Install sample NMEA log
nmea.files=nmea_sample.txt
nmea.path=/etc/whereabouts
INSTALLS+=nmea

