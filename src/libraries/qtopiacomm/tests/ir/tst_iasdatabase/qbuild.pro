TEMPLATE=app
CONFIG+=qtopia unittest
QTOPIA*=comm
TARGET=tst_iasdatabase
SOURCES=tst_iasdatabase.cpp
requires(enable_infrared)
