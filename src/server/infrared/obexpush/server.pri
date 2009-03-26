REQUIRES=enable_infrared
QTOPIA*=comm
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/comm/obex\
    /src/server/comm/filetransfer\
    /src/server/infrared/powermgr::exists\

HEADERS+=\
        irobexpushservice.h

SOURCES+=\
        irobexpushservice.cpp

