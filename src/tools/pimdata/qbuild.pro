TEMPLATE=app
TARGET=pimdata

CONFIG+=qtopia singleexec quicklaunch
QTOPIA*=pim

DEFINES+=PIMXML_NAMESPACE=PimDataGen

# Packaged with pimdatagui

SOURCEPATH+=../qdsync/pim

RESOURCES=\
    namefiles.qrc

SOURCES=\
    main.cpp\
    cgen.cpp\
    generator.cpp\
    qpimxml.cpp

HEADERS=\
    cgen.h\
    generator.h\
    qpimxml_p.h

# Install rules

target [
    hint=sxe
    domain=trusted
]

