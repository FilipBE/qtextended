REQUIRES=enable_telephony
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\

HEADERS+=\
        abstractdialfilter.h

SOURCES+=\
        abstractdialfilter.cpp

phone_settings [
    hint=image
    files=$$SERVER_PWD/etc/default/Trolltech/Phone.conf
    path=/etc/default/Trolltech
]

