TEMPLATE=app
TARGET=tst_qtopiacalc
CONFIG+=qtopia unittest

SOURCEPATH+= /src/applications/calculator

HEADERS += engine.h \
           instruction.h \
           doubleinstruction.h \
           doubledata.h \
           data.h
SOURCES += tst_qtopiacalc.cpp \
           engine.cpp \
           instruction.cpp \
           doubleinstruction.cpp \
           doubledata.cpp \
           data.cpp
