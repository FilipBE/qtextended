REQUIRES=enable_telephony
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/media/audiohandler/abstract\

HEADERS+=\
        callaudiohandler.h

SOURCES+=\
        callaudiohandler.cpp
