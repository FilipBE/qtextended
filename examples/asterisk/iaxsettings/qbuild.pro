requires(enable_voip)
TEMPLATE=app
TARGET=iaxsettings

CONFIG+=qtopia
QTOPIA+=phone

FORMS=\
    iaxsettingsbase.ui 

HEADERS=\
    iaxsettings.h

SOURCES=\
    iaxsettings.cpp\
    main.cpp

desktop [
    hint=desktop
    files=iaxsettings.desktop
    path=/apps/Settings
]

pics [
    hint=pics
    files=pics/*
    path=/pics/iaxsettings
]

