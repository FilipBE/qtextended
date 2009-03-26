TEMPLATE=app
CONFIG+=qtopia unittest
TARGET=tst_qtestprotocol
MODULES*=qtuitest
SOURCEPATH+=/src/libraries/qtuitest

SOURCES+=                   \
    tst_qtestprotocol.cpp   \
    testprotocol.cpp        \
    testprotocolserver.cpp

HEADERS+=                 \
    testprotocol.h        \
    testprotocolserver.h

