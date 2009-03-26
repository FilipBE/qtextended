REQUIRES=enable_cell
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/browserscreen/abstract\
    /src/server/phone/callhistory/abstract\
    /src/server/phone/callscreen/abstract\
    /src/server/phone/contextlabel/abstract\
    /src/server/phone/dialer/abstract\
    /src/server/phone/header/abstract\
    /src/server/phone/telephony/callpolicymanager/abstract\
    /src/server/phone/telephony/dialproxy\
    /src/server/phone/themecontrol\
    /src/server/phone/ui/browserstack\
    /src/server/ui/launcherviews/base\

HEADERS+=\
    e3_phonebrowser.h\
    e3_launcher.h\
    e3_today.h\
    e3_navipane.h\
    e3_clock.h

SOURCES+=\
    e3_phonebrowser.cpp\
    e3_launcher.cpp\
    e3_today.cpp\
    e3_navipane.cpp\
    e3_clock.cpp

samples_settings [
    hint=image
    files=$$SERVER_PWD/etc/default/Trolltech/E3.conf
    path=/etc/default/Trolltech
]

