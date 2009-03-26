REQUIRES=enable_bluetooth
QTOPIA*=comm
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/comm/obex\
    /src/server/comm/filetransfer\
    /src/server/bluetooth/powermgr::exists\
    /src/server/bluetooth/servicemgr::exists\

HEADERS+=\
        bluetoothobexpushservice.h

SOURCES+=\
        bluetoothobexpushservice.cpp

btoppservices [
    hint=image
    files=$$SERVER_PWD/etc/bluetooth/sdp/opp.xml
    path=/etc/bluetooth/sdp
]
