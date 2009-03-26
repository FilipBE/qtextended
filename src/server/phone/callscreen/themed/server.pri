REQUIRES=enable_telephony
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/callscreen/abstract\
    /src/server/phone/media/audiohandler/abstract\
    /src/server/phone/media/audiohandler/call\
    /src/server/phone/telephony/dialercontrol\
    /src/server/phone/telephony/dialfilter/abstract\
    /src/server/phone/themecontrol\
    /src/server/pim/servercontactmodel\
    /src/server/processctrl/taskmanagerentry\

HEADERS+=\
        callscreen.h

SOURCES+=\
        callscreen.cpp

