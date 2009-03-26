TEMPLATE=app
CONFIG+=qtopia unittest
include(/tests/shared/qfuturesignal.pri)
QTOPIA*=collective
TARGET=tst_dummypresenceservice
SOURCES *= tst_dummypresenceservice.cpp
