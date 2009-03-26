REQUIRES=enable_bluetooth
QTOPIA*=comm
SERVER_DEPS*=\
    /src/server/core_server\

# This is necessary for Handsfree / Headset to work
equals(QTOPIA_SOUND_SYSTEM,alsa) {
    DEFINES+=HAVE_ALSA
}

HEADERS+=scomisc_p.h
SOURCES+=$$device_overrides(/src/server/bluetooth/scomisc/scomisc.cpp)

