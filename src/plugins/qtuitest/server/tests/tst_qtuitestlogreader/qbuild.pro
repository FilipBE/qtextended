TEMPLATE=app
TARGET=tst_qtuitestlogreader
CONFIG+=qtopia unittest

SOURCEPATH+=../..

SOURCES+=                       \
    qtuitestlogreader.cpp       \
    tst_qtuitestlogreader.cpp

HEADERS+=                       \
    qtuitestlogreader_p.h

MOC_COMPILE_EXCEPTIONS+=qtuitestlogreader_p.h

