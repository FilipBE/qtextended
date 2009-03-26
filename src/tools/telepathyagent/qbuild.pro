requires(enable_dbus)
TEMPLATE=app
TARGET=telepathyagent

CONFIG+=qtopia singleexec
QTOPIA+=phone collective pim
QT+=dbus

pkg [
    name=telepathyagent
    desc="Telepath agent for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    service.h\
    telepathychannel.h\
    telepathychannelinterfacegroup.h\
    telepathychanneltypetext.h\
    telepathyconnection.h\
    telepathyconnectioninterfacealiasing.h\
    telepathyconnectioninterfaceavatars.h\
    telepathyconnectioninterfacecapabilities.h\
    telepathyconnectioninterfacepresence.h\
    telepathyconnectionmanager.h\
    telepathynamespace.h

SOURCES=\
    main.cpp\
    service.cpp\
    telepathychannel.cpp\
    telepathychannelinterfacegroup.cpp\
    telepathychanneltypetext.cpp\
    telepathyconnection.cpp\
    telepathyconnectioninterfacealiasing.cpp\
    telepathyconnectioninterfaceavatars.cpp\
    telepathyconnectioninterfacecapabilities.cpp\
    telepathyconnectioninterfacepresence.cpp\
    telepathyconnectionmanager.cpp\
    telepathynamespace.cpp

# Install rules

target [
    hint=sxe
    domain=trusted
]

telepathyservice [
    hint=image
    files=services/Telephony/telepathyagent
    path=/services/Telephony
]

