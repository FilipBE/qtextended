REQUIRES=enable_voip
QTOPIA*=phone pim collective
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/telephony/callpolicymanager/abstract\

HEADERS+=\
        jabbermanager.h

SOURCES+=\
        jabbermanager.cpp

