REQUIRES=enable_telephony
QTOPIA*=phone comm
SERVER_DEPS*=\
    /src/server/core_server\

HEADERS+=\
        dtmfaudio.h

SOURCES+=\
        dtmfaudio.cpp

MOC_COMPILE_EXCEPTIONS+=\
        dtmfaudio.h

