REQUIRES=enable_telephony
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/dialer/abstract\
    /src/server/phone/media/dtmfaudio\
    /src/server/phone/themecontrol\
    /src/server/phone/telephony/dialfilter/abstract\
    /src/server/pim/savetocontacts\
    /src/server/pim/servercontactmodel\

HEADERS+=\
        dialer.h

SOURCES+=\
        dialer.cpp

