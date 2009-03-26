TEMPLATE=app
CONFIG+=embedded
TARGET=test_drmagent
SOURCES=main.cpp
include(../locate_drmagent.pri)
LIBS+=$$DRMAGENT -lpthread
