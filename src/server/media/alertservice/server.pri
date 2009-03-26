UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\

HEADERS+=\
         alertservicetask.h

SOURCES+=\
         alertservicetask.cpp

alertservice [
    hint=image
    files=$$SERVER_PWD/services/Alert/qpe
    path=/services/Alert
]

