REQUIRES=enable_telephony
QTOPIA*=phone
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/pim/servercontactmodel\
    /src/server/phone/telephony/callpolicymanager/abstract\

HEADERS+=\
        dialercontrol.h

SOURCES+=\
        dialercontrol.cpp

