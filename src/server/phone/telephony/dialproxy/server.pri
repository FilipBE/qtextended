REQUIRES=enable_telephony
QTOPIA*=phone
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/telephony/dialercontrol\
    /src/server/phone/media/audiohandler/abstract\

HEADERS+=\
        dialproxy.h dialerservice.h

SOURCES+=\
        dialproxy.cpp dialerservice.cpp

dialerservice [
    hint=image
    files=$$SERVER_PWD/services/Dialer/qpe
    path=/services/Dialer
]

