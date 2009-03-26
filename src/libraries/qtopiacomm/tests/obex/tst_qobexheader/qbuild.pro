TEMPLATE=app
CONFIG+=qtopia unittest
QTOPIA*=comm
MODULES*=openobex
get_sourcepath(qtopiacomm)
TARGET=tst_qobexheader
HEADERS=../headertestdata.h
SOURCES=tst_qobexheader.cpp \
        ../headertestdata.cpp
