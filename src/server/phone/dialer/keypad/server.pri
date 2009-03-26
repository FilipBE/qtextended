REQUIRES=enable_telephony
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/dialer/abstract\
    /src/server/phone/ui/callcontactmodelview\
    /src/server/phone/media/dtmfaudio\
    /src/server/phone/telephony/dialfilter/abstract\
    /src/server/phone/telephony/dialercontrol\
    /src/server/pim/servercontactmodel\

HEADERS+=\
        numberdisplay.h quickdial.h

SOURCES+=\
        numberdisplay.cpp quickdial.cpp

