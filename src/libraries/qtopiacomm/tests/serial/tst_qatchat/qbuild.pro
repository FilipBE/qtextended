TEMPLATE=app
CONFIG+=qtopia unittest
include(/tests/shared/qfuturesignal.pri)
QTOPIA*=comm
TARGET=tst_qatchat
HEADERS*=testserialiodevice.h
SOURCES*=tst_qatchat.cpp testserialiodevice.cpp
