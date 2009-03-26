TEMPLATE=plugin
TARGET=pim

PLUGIN_FOR=qtopia
PLUGIN_TYPE=qdsync

CONFIG+=qtopia singleexec
QTOPIA+=pim
MODULES*=qdsync_common

pkg [
    name=synchronization-pim-plugin
    desc="PIM plugin for Synchronization."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

DEFINES+=PIMXML_NAMESPACE=QDSync

HEADERS=\
    qpimsyncstorage.h\
    qpimxml_p.h\

SOURCES=\
    qpimsyncstorage.cpp\
    qpimxml.cpp\

