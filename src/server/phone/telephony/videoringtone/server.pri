REQUIRES=enable_qtopiamedia
QTOPIA*=media
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/telephony/ringcontrol::exists\

HEADERS+=\
         videoringtone.h

SOURCES+=\
         videoringtone.cpp

