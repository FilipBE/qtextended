REQUIRES=enable_voip
QTOPIA*=collective pim
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/infrastructure/messageboard\
    /src/server/phone/telephony/callpolicymanager/abstract\

HEADERS+=\
        presenceservice.h

SOURCES+=\
        presenceservice.cpp

presenceservice [
    hint=image
    files=$$SERVER_PWD/services/Presence/qpe
    path=/services/Presence
]

