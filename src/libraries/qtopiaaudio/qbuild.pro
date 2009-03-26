TEMPLATE=lib
TARGET=qtopiaaudio
MODULE_NAME=qtopiaaudio
VERSION=4.0.0

CONFIG+=qtopia hide_symbols singleexec

pkg [
    name=qtextendedaudio
    desc="Audio library for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    qaudioinput.h \
    qaudiooutput.h \
    qaudiostateconfiguration.h \
    qaudiostateplugin.h \
    qaudiostate.h \
    qaudiostateinfo.h \
    qaudiostatemanager.h \
    qaudionamespace.h\
    qaudiointerface.h \
    qaudiomixer.h

PRIVATE_HEADERS=\
    qaudiostatemanagerservice_p.h

SOURCES=\
    qaudiostateconfiguration.cpp \
    qaudiostateplugin.cpp \
    qaudiostate.cpp \
    qaudiostateinfo.cpp \
    qaudiostatemanager.cpp \
    qaudionamespace.cpp \
    qaudiostatemanagerservice.cpp \
    qaudiointerface.cpp

equals(QTOPIA_SOUND_SYSTEM,alsa) {
    MODULES*=alsa
    SOURCES+=\
        qaudioinput_alsa.cpp\
        qaudiooutput_alsa.cpp\
        qaudiomixer_alsa.cpp
    DEFINES+=QTOPIA_HAVE_ALSA
}

equals(QTOPIA_SOUND_SYSTEM,pulse) {
    MODULES*=alsa pulse
    SOURCES+=\
        qaudioinput_pulse.cpp\
        qaudiooutput_pulse.cpp\
        qaudiomixer_alsa.cpp
    DEFINES+=QTOPIA_HAVE_PULSE QTOPIA_HAVE_ALSA
}

equals(QTOPIA_SOUND_SYSTEM,oss) {
    SOURCES+=\
        qaudioinput_oss.cpp\
        qaudiomixer_oss.cpp
    enable_qtopiamedia {
        SOURCES+=\
            qaudiooutput_oss.cpp
        DEFINES+=QTOPIA_HAVE_OSS
    } else {
        SOURCES+=\
            qaudiooutput_qss.cpp
        DEFINES+=QTOPIA_HAVE_QSS
    }
}

