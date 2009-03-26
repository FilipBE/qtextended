TEMPLATE=app
TARGET=vsexplorer

CONFIG+=qtopia singleexec
enable_bluetooth:QTOPIA*=comm
#enable_readline {
#    MODULES*=readline
#    DEFINES+=USE_READLINE
#}

pkg [
    name=vsexplorer
    desc="Valuespace explorer for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

SOURCES=\
    vsexplorer.cpp

# Install rules

target [
    hint=sxe
    domain=trusted
]

