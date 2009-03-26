TEMPLATE=app
TARGET=radioplayer

CONFIG+=qtopia

DEFINES+=QTOPIA_DUMMY_RADIO

RESOURCES=\
    radioplayer.qrc

FORMS=\
    radioplayer.ui

HEADERS=\
    radioplayer.h\
    radioband.h\
    radiobandv4l.h\
    radiobanddummy.h\
    radiobandmanager.h\
    radiopresets.h\
    radioservice.h\
    rdsgroup.h\
    rdsprograminfo.h

SOURCES=\
    radioplayer.cpp\
    radioband.cpp\
    radiobandv4l.cpp\
    radiobanddummy.cpp\
    radiobandmanager.cpp\
    radiopresets.cpp\
    radioservice.cpp\
    rdsgroup.cpp\
    rdsprograminfo.cpp\
    main.cpp

desktop [
    hint=nct desktop
    files=desktop/radioplayer.desktop
    path=/apps/Applications
    trtarget=radioplayer-nct
]

help [
    hint=help
    source=help
    files=*.html
]

pics [
    hint=pics
    files=pics/*
    path=/pics/radioplayer
]

service [
    hint=image
    files=services/Radio/radioplayer
    path=/services/Radio
]

servicedef [
    hint=image
    files=services/Radio.service
    path=/services
]

