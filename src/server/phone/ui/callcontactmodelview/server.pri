REQUIRES=enable_telephony
QTOPIA*=pim phone
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/pim/savetocontacts

MODULES*=inputmatch

HEADERS+=\
        callcontactlist.h

SOURCES+=\
        callcontactlist.cpp

