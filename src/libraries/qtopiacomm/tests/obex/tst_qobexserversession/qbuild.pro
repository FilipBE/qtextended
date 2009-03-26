TEMPLATE=app
CONFIG+=qtopia unittest
QTOPIA*=comm
MODULES*=openobex
get_sourcepath(qtopiacomm)
TARGET=tst_qobexserversession
HEADERS=putclientwithnofirstpacketbody.h
SOURCES=putclientwithnofirstpacketbody.cpp\
        tst_qobexserversession.cpp
