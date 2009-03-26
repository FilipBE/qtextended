REQUIRES=enable_telephony
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/callhistory/abstract\
    /src/server/phone/ui/callcontactmodelview\
    /src/server/phone/telephony/dialercontrol\

HEADERS+=\
        callhistory.h

SOURCES+=\
        callhistory.cpp

callhistorydesktop [
    hint=desktop
    files=$$SERVER_PWD/callhistory.desktop
    path=/apps/Applications
]

callhistorypics [
    hint=pics
    files=$$SERVER_PWD/pics/*
    path=/pics/callhistory
]

callhistoryservice [
    hint=image
    files=$$SERVER_PWD/services/CallHistory/qpe
    path=/services/CallHistory
]

