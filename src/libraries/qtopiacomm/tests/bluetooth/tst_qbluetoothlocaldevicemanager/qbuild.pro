TEMPLATE=app
CONFIG+=qtopia unittest
include(../bluetooth.pri)
SOURCEPATH+=/src/libraries/qtopiacomm/bluetooth
TARGET=tst_qbluetoothlocaldevicemanager
SOURCES += tst_qbluetoothlocaldevicemanager.cpp \
           qbluetoothlocaldevicemanager.cpp
