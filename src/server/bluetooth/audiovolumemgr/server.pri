REQUIRES=enable_bluetooth
QTOPIA*=comm
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/media/volumemanagement::exists

HEADERS+=\
        btaudiovolumemanager.h

SOURCES+=\
        btaudiovolumemanager.cpp

