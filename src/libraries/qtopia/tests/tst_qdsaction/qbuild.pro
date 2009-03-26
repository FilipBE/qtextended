TEMPLATE=app
CONFIG+=qtopia unittest
SOURCEPATH+=/src/libraries/qtopiabase
get_sourcepath(qtopia)
TARGET=tst_qdsaction
SOURCES+=tst_qdsaction.cpp qtopiachannel.cpp
