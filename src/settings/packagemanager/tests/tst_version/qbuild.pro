TEMPLATE=app
CONFIG+=qtopia unittest
TARGET=tst_version

SOURCEPATH+=$$QTOPIA_DEPOT_PATH/src/settings/packagemanager
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/settings/packagemanager
VPATH      +=$$QTOPIA_DEPOT_PATH/src/settings/packagemanager
SOURCES+=tst_version.cpp \
         packageversion.cpp
HEADERS+=packageversion.h
