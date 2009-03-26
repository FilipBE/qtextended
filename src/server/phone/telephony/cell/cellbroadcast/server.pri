REQUIRES=enable_cell
QTOPIA*=phone
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/infrastructure/messageboard\
    /src/server/phone/telephony/callpolicymanager/abstract\

HEADERS+=\
        cellbroadcastcontrol.h

SOURCES+=\
        cellbroadcastcontrol.cpp

