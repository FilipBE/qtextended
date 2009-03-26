TEMPLATE=app
TARGET=sysmessages

CONFIG+=qtopia singleexec

pkg [
    name=sysmessages
    desc="System messages for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

contains(PROJECTS,libraries/qtopiamail):DEFINES+=MAIL_EXISTS

HEADERS=\
    sysmessages.h

SOURCES=\
    main.cpp\
    sysmessages.cpp

# Install rules

target [
    hint=sxe
    domain=trusted
]

sysmsgservice [
    hint=image
    files=services/SystemMessages/sysmessages
    path=/services/SystemMessages
]
