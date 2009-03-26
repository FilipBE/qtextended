REQUIRES=enable_cell
QTOPIA*=phone
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/telephony/phoneserver/base\

HEADERS+=\
        phoneserverdummymodem.h

SOURCES+=\
        phoneserverdummymodem.cpp

