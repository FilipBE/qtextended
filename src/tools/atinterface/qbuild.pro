TEMPLATE=app
TARGET=atinterface

CONFIG+=qtopia singleexec
QTOPIA*=phone comm
enable_cell:QTOPIA*=mail pim

#nothing to translate at this stage
#STRING_LANGUAGE=en_US
#AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
#LANGUAGES=$$QTOPIA_LANGUAGES
#UNIFIED_NCT_LUPDATE=1

pkg [
    name=atinterface
    desc="AT interface for remote access to the device over a cable."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    atcustom.h\
    atcallmanager.h\
    atparseutils.h\
    atcommands.h\
    atgsmnoncellcommands.h\
    atv250commands.h\
    atfrontend.h\
    atindicators.h\
    atinterface.h\
    atoptions.h\
    atsessionmanager.h\
    modememulatorservice.h

SOURCES=\
    main.cpp\
    atcallmanager.cpp\
    atparseutils.cpp\
    atcommands.cpp\
    atgsmnoncellcommands.cpp\
    atv250commands.cpp\
    atfrontend.cpp\
    atindicators.cpp\
    atinterface.cpp\
    atoptions.cpp\
    atsessionmanager.cpp\
    modememulatorservice.cpp

bluetooth [
    TYPE=CONDITIONAL_SOURCES
    CONDITION=enable_bluetooth
    HEADERS=atbluetoothcommands.h
    SOURCES=atbluetoothcommands.cpp
]

cell [
    TYPE=CONDITIONAL_SOURCES
    CONDITION=enable_cell
    HEADERS=\
        atgsmcellcommands.h\
        atsmscommands.h
    SOURCES=\
        atgsmcellcommands.cpp\
        atsmscommands.cpp
]

# Install rules

target [
    hint=sxe
    domain=trusted
]

modememulservice [
    hint=image
    files=services/ModemEmulator/atinterface
    path=/services/ModemEmulator
]

usbservice [
    hint=image
    files=services/UsbGadget/Serial/atinterface
    path=/services/UsbGadget/Serial
]

help [
    hint=help
    source=help
    files=*.html
]

