REQUIRES=enable_bluetooth
QTOPIA*=comm
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/bluetooth/scomisc\
    /src/server/bluetooth/audiovolumemgr\
    /src/server/bluetooth/powermgr::exists\
    /src/server/bluetooth/servicemgr::exists\

HEADERS+=\
    btheadsettask.h \
    qbluetoothhsagserver_p.h \
    qbluetoothhsservice_p.h 

SOURCES+=\
    btheadsettask.cpp \
    qbluetoothhsagserver.cpp \
    qbluetoothhsservice.cpp 

bthsservices [
    hint=image
    files=$$SERVER_PWD/etc/bluetooth/sdp/hsag.xml
    path=/etc/bluetooth/sdp
]

