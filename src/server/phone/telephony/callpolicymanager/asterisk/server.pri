REQUIRES=enable_voip
QTOPIA*=phone
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/telephony/callpolicymanager/abstract\

HEADERS+=\
        asteriskmanager.h

SOURCES+=\
        asteriskmanager.cpp

