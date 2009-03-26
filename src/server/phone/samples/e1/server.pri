REQUIRES=enable_cell
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/telephony/dialercontrol\
    /src/server/phone/telephony/dialproxy\
    /src/server/phone/themecontrol\
    /src/server/phone/ui/browserstack\
    /src/server/ui/launcherviews/base\

HEADERS+=\
    e1_bar.h\
    e1_battery.h\
    e1_header.h\
    e1_launcher.h\
    e1_dialog.h\
    e1_dialer.h\
    e1_error.h\
    e1_callhistory.h\
    e1_phonebrowser.h\
    e1_popup.h\
    e1_callscreen.h\
    colortint.h\
    e1_incoming.h\
    e1_telephony.h\
 
SOURCES+=\
    e1_bar.cpp\
    e1_battery.cpp\
    e1_header.cpp\
    e1_launcher.cpp\
    e1_phonebrowser.cpp\
    colortint.cpp\
    e1_dialer.cpp\
    e1_error.cpp\
    e1_incoming.cpp\
    e1_dialog.cpp\
    e1_callhistory.cpp\
    e1_popup.cpp\
    e1_callscreen.cpp\
    e1_telephony.cpp

samplespics [
    hint=pics
    files=$$SERVER_PWD/pics/samples/*
    path=/pics/samples
]

samplesprofilepics [
    hint=pics
    files=$$SERVER_PWD/pics/profiles/*
    path=/pics/profiles
]

