TEMPLATE=app
TARGET=btsettings

CONFIG+=qtopia singleexec quicklaunch
QTOPIA*=comm

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=btsettings
    desc="Bluetooth settings for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

FORMS=\
    remotedeviceinfo.ui

HEADERS=\
    btsettings.h\
    btsettings_p.h\
    settingsdisplay.h\
    mydevicesdisplay.h\
    localservicesdialog.h\
    pairingagent.h\
    remotedeviceinfodialog.h

SOURCES=\
    btsettings.cpp\
    settingsdisplay.cpp\
    mydevicesdisplay.cpp\
    localservicesdialog.cpp\
    pairingagent.cpp\
    remotedeviceinfodialog.cpp\
    main.cpp

# Install rules

target [
    hint=sxe
    domain=trusted
]


desktop [
    hint=desktop
    files=btsettings.desktop
    path=/apps/Settings
]

help [
    hint=help
    source=help
    files=*.html
]

pics [
    hint=pics
    files=pics/*
    path=/pics/btsettings
]

settings [
    hint=image
    files=etc/default/Trolltech/Bluetooth.conf
    path=/etc/default/Trolltech
]

settings2 [
    hint=image optional
    files=etc/default/Trolltech/BluetoothKnownHeadsets.conf
    path=/etc/default/Trolltech
]

