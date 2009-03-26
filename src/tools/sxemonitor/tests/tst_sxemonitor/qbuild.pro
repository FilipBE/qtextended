TEMPLATE=app
CONFIG+=qtopia unittest
TARGET=tst_sxemonitor

SOURCES += tst_sxemonitor.cpp \
           ../../../../../src/tools/sxemonitor/sxemonitor.cpp \
           ../../../../../src/tools/sxemonitor/sxemonqlog.cpp

HEADERS += ../../../../../src/tools/sxemonitor/sxemonitor.h \
           ../../../../../src/tools/sxemonitor/sxemonqlog.h     

