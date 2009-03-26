REQUIRES=enable_infrared
QTOPIA*=comm pim
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/comm/obex\
    /src/server/comm/filetransfer\
    /src/server/infrared/powermgr::exists\

HEADERS+=\
        irfilesendservice.h

SOURCES+=\
        irfilesendservice.cpp

irbeamingservice [
    hint=image
    files=$$SERVER_PWD/services/InfraredBeaming/qpe
    path=/services/InfraredBeaming
]

irqdsservice [
    hint=image
    files=$$SERVER_PWD/etc/qds/InfraredBeaming
    path=/etc/qds
]

