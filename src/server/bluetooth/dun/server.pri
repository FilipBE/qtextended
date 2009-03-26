REQUIRES=enable_bluetooth
QTOPIA*=comm
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/bluetooth/serial\

HEADERS+=\
        btdialupservice.h

SOURCES+=\
        btdialupservice.cpp

btdunservices [
    hint=image
    files=$$SERVER_PWD/etc/bluetooth/sdp/dun.xml
    path=/etc/bluetooth/sdp
]

