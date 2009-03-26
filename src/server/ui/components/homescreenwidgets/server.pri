QTOPIA*=pim
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\

HEADERS+=\
        homescreenwidgets.h

SOURCES+=\
        homescreenwidgets.cpp

hswidgets [
    hint=image optional
    files=$$SERVER_PWD/etc/default/Trolltech/HomeScreenWidgets.conf
    path=/etc/default/Trolltech
]

