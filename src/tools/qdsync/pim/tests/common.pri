SOURCEPATH+=$$path(.,project)
HEADERS+=common.h
SOURCES+=common.cpp
MODULES+=qdsync_common

# Pull in the qdsync PIM code
SOURCEPATH+=/src/tools/qdsync/pim
DEFINES+=PIMXML_NAMESPACE=$$define_value(QDSync)
HEADERS+=\
    qpimsyncstorage.h\
    qpimxml_p.h\

SOURCES+=\
    qpimsyncstorage.cpp\
    qpimxml.cpp\

QTOPIA+=pim

