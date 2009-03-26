REQUIRES=enable_telephony
QTOPIA*=phone
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/phone/telephony/dialercontrol\
    /src/server/pim/servercontactmodel\
    /src/server/phone/profileprovider::exists\

HEADERS+=\
        ringtoneservice.h \
        ringcontrol.h

SOURCES+=\
        ringtoneservice.cpp \
        ringcontrol.cpp

ringtoneservice [
    hint=image
    files=$$SERVER_PWD/services/Ringtone/qpe
    path=/services/Ringtone
]

