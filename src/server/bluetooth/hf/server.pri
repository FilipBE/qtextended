REQUIRES=enable_bluetooth
QTOPIA*=comm
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/bluetooth/scomisc\
    /src/server/bluetooth/audiovolumemgr\
    /src/server/bluetooth/powermgr::exists\
    /src/server/bluetooth/servicemgr::exists\

HEADERS+=\
      bthandsfreetask.h \
      qbluetoothhfagserver_p.h \
      qbluetoothhfservice_p.h

SOURCES+=\
      bthandsfreetask.cpp \
      qbluetoothhfagserver.cpp \
      qbluetoothhfservice.cpp

bthfservices [
    hint=image
    files=$$SERVER_PWD/etc/bluetooth/sdp/hfag.xml
    path=/etc/bluetooth/sdp
]

