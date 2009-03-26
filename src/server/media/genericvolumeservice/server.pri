QTOPIA*=audio
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/media/volumemanagement::exists\

HEADERS+=\
        genericvolumeservice.h

SOURCES+=\
        genericvolumeservice.cpp

sound_settings [
    hint=image optional
    files=$$SERVER_PWD/etc/default/Trolltech/Sound.conf
    path=/etc/default/Trolltech
]

