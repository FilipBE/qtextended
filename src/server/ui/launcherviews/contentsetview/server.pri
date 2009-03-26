UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/ui/launcherviews/base\

HEADERS+=\
         contentsetlauncherview.h

SOURCES+=\
         contentsetlauncherview.cpp

contentsetviewservice [
    hint=image
    files=$$SERVER_PWD/services/ContentSetView/qpe
    path=/services/ContentSetView
]

