UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\

HEADERS+=\
    startupapps.h

SOURCES+=\
    preloadcrashdlg.cpp \
    startupapps.cpp

bgapps_settings [
    hint=image
    files=$$SERVER_PWD/etc/default/Trolltech/BackgroundApplications.conf
    path=/etc/default/Trolltech
]

