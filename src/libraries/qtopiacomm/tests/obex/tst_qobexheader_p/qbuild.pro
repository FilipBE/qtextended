TEMPLATE=app
CONFIG+=qtopia unittest
QTOPIA*=comm
MODULES*=openobex
get_sourcepath(qtopiacomm)
TARGET=tst_qobexheader_p
HEADERS=openobexheaderprocessor.h \
        ../headertestdata.h
SOURCES=openobexheaderprocessor.cpp \
        ../headertestdata.cpp \
        tst_qobexheader_p.cpp
