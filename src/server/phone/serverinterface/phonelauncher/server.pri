SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/browserscreen/abstract\
    /src/server/phone/contextlabel/abstract\
    /src/server/phone/header/abstract\
    /src/server/phone/homescreen/abstract\
    /src/server/phone/secondarydisplay/abstract\
    /src/server/ui/abstractinterfaces/taskmanager\
    /src/server/processctrl/appmonitor\

enable_telephony:SERVER_DEPS*=\
    /src/server/phone/callhistory/abstract\
    /src/server/phone/dialer/abstract\
    /src/server/phone/callscreen/abstract\
    /src/server/phone/telephony/callpolicymanager/abstract\
    /src/server/phone/telephony/dialercontrol\
    /src/server/phone/telephony/dialproxy\
    /src/server/phone/media/audiohandler/abstract\

HEADERS+=\
        phonelauncher.h

SOURCES+=\
        phonelauncher.cpp

