REQUIRES=drmagent
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/ui/launcherviews/base\

HEADERS+=\
        archiveviewer.h

SOURCES+=\
        archiveviewer.cpp

archivesdesktop [
    hint=desktop
    files=$$SERVER_PWD/archives.desktop
    path=/apps/Applications
]

archiveshelp [
    hint=help
    source=$$SERVER_PWD/help
    files=*.html
]

archivespics [
    hint=pics
    files=$$SERVER_PWD/pics/*
    path=/pics/archives
]

