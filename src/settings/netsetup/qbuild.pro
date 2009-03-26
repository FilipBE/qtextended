TEMPLATE=app
TARGET=netsetup

CONFIG+=qtopia singleexec quicklaunch
QTOPIA*=comm
enable_cell:QTOPIA*=phone

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=netsetup
    desc="Network settings for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
    multi=\
        plugins/network/lan\
        plugins/network/bluetooth\
        plugins/network/bluetoothpan\
        plugins/network/dialing
]

FORMS=\
    gatewaybase.ui\
    mmsbase.ui\
    browserbase.ui

HEADERS=\
    networkui.h\
    addnetwork.h\
    wapui.h\
    addwapui.h

SOURCES=\
    networkui.cpp\
    addnetwork.cpp\
    main.cpp\
    wapui.cpp\
    addwapui.cpp

vpn [
    TYPE=CONDITIONAL_SOURCES
    CONDITION=enable_vpn
    HEADERS=vpnui.h
    SOURCES=vpnui.cpp
]

# Install rules

target [
    hint=sxe
    domain=trusted
]

desktop [
    hint=desktop
    files=netsetup.desktop
    path=/apps/Settings
]

help [
    hint=help
    source=help
    files=*.html
]

otaservice [
    hint=image
    files=etc/qds/NetworkSetup
    path=/etc/qds
]

pics [
    hint=pics
    files=pics/*
    path=/pics/netsetup
]

service [
    hint=image
    files=services/NetworkSetup/netsetup
    path=/services/NetworkSetup
]

