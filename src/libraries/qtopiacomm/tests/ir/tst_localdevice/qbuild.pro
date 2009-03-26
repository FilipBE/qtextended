TEMPLATE=app
CONFIG+=qtopia unittest
QTOPIA*=comm
TARGET=tst_localdevice
SOURCES=tst_localdevice.cpp
requires(enable_infrared)
