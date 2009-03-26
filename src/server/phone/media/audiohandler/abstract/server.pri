REQUIRES=enable_telephony
QTOPIA*=audio
SERVER_DEPS*=\
    /src/server/core_server\

HEADERS+=\
        abstractaudiohandler.h

SOURCES+=\
        abstractaudiohandler.cpp

MOC_COMPILE_EXCEPTIONS+=\
        abstractaudiohandler.h

