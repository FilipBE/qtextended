TEMPLATE=app
CONFIG+=qtopia unittest
SOURCEPATH+= /src/libraries/qtuitest
TARGET=tst_qalternatestack

SOURCES+= \
    tst_qalternatestack.cpp \
    qalternatestack_unix.cpp

HEADERS+= \
    qalternatestack_p.h

