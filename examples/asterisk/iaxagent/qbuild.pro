requires(enable_voip)
TEMPLATE=app
TARGET=iaxagent

CONFIG+=qtopia singleexec
QTOPIA+=phone audio
MODULES+=iaxclient gsm

HEADERS=\
    iaxcallprovider.h\
    iaxconfiguration.h\
    iaxnetworkregistration.h\
    iaxservicechecker.h\
    iaxtelephonyservice.h

SOURCES=\
    main.cpp\
    iaxcallprovider.cpp\
    iaxconfiguration.cpp\
    iaxnetworkregistration.cpp\
    iaxservicechecker.cpp\
    iaxtelephonyservice.cpp

telephonyservice [
    hint=image
    files=services/Telephony/iaxagent
    path=/services/Telephony
]

