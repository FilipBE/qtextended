REQUIRES=enable_bluetooth
QTOPIA*=comm
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/bluetooth/powermgr::exists\
    /src/server/bluetooth/servicemgr::exists\

HEADERS+=\
        bluetoothserialportservice.h

SOURCES+=\
        bluetoothserialportservice.cpp

btsppservices [
    hint=image
    files=$$SERVER_PWD/etc/bluetooth/sdp/spp.xml
    path=/etc/bluetooth/sdp
]

