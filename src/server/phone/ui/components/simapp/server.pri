REQUIRES=enable_cell
QTOPIA*=phone
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/infrastructure/messageboard\
    /src/server/processctrl/appmonitor\

HEADERS+=\
        simapp.h \
        simicons.h \
        simwidgets.h

SOURCES+=\
        simapp.cpp \
        simicons.cpp \
        simwidgets.cpp

simappdesktop [
    hint=desktop
    files=$$SERVER_PWD/simapp.desktop
    path=/apps/Applications
]

simapppics [
    hint=pics
    files=$$SERVER_PWD/pics/*
    path=/pics/simapp
]

simapphelp [
    hint=help
    source=$$SERVER_PWD/help
    files=*.html
]

simappsounds [
    hint=image
    files=$$SERVER_PWD/sounds/*
    path=/sounds/simapp
]

