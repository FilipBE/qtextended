UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\

FORMS+=\
    shutdown.ui

HEADERS+=\
        shutdownimpl.h

SOURCES+=\
        shutdownimpl.cpp

quitdesktop [
    hint=desktop
    files=quit.desktop
    path=/apps/Settings
]
# Comment out this line if a shutdown application entry in the app launcher list is required
quitdesktop.hint+=disabled

