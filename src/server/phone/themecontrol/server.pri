UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\

MODULES*=qtopiatheming

HEADERS+=\
        serverthemeview.h \
        themebackground_p.h \
        themecontrol.h \
        qabstractthemewidgetfactory.h \
        themesetuptask.h

SOURCES+=\
        serverthemeview.cpp \
        themebackground_p.cpp \
        themecontrol.cpp \
        themesetuptask.cpp

defaulttheme [
    files=$$SERVER_PWD/theme/*
    path=/etc/themes/default
    hint=image
]

defaultpics [
    files=$$SERVER_PWD/pics/*
    path=/pics/themes/default
    hint=pics
]

