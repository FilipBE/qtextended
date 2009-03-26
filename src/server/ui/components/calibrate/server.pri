REQUIRES=!x11
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\

HEADERS+=\
        calibrate.h

SOURCES+=\
        calibrate.cpp \
        register.cpp

calibratedesktop [
    hint=desktop
    files=$$SERVER_PWD/calibrate.desktop
    path=/apps/Settings
]

calibrateservice [
    hint=image
    files=$$SERVER_PWD/services/calibrate/calibrate
    path=/services/calibrate
]

calibratepics [
    hint=pics
    files=$$SERVER_PWD/pics/*
    path=/pics/calibrate
]

