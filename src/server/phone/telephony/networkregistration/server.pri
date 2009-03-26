REQUIRES=enable_telephony
QTOPIA*=phone
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/telephony/callpolicymanager/abstract\
    /src/server/infrastructure/messageboard\

HEADERS+=\
        networkregupdate.h

SOURCES+=\
        networkregupdate.cpp

