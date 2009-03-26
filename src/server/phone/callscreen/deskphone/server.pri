REQUIRES=enable_telephony
MODULES*=homeui
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/callscreen/abstract\
    /src/server/phone/media/audiohandler/abstract\
    /src/server/phone/telephony/dialercontrol\
    /src/server/pim/servercontactmodel\

HEADERS+=\
        deskcallscreen.h \
        deskcallscreendialogs.h

SOURCES+=\
        deskcallscreen.cpp \
        deskcallscreendialogs.cpp

FORMS+=\
        deskcallscreen.ui

