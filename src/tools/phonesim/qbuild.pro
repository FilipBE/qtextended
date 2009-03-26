TEMPLATE=app
TARGET=phonesim

CONFIG+=qt
QT*=network xml
MODULES*=phonesim

FORMS=\
    controlbase.ui

HEADERS=\
    control.h\
    attranslator.h\
    gsmspec.h\
    gsmitem.h\
    

SOURCES=\
    main.cpp\
    control.cpp\
    attranslator.cpp\
    gsmspec.cpp\
    gsmitem.cpp

