REQUIRES=enable_telephony
QTOPIA*=phone
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/media/audiohandler/abstract\
    /src/server/phone/media/dtmfaudio\
    /src/server/phone/telephony/callpolicymanager/abstract\

HEADERS+=\
        dialtoneaudiohandler.h

SOURCES+=\
        dialtoneaudiohandler.cpp
