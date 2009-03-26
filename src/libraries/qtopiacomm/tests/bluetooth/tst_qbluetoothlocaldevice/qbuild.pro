TEMPLATE=app
CONFIG+=qtopia unittest
include(../bluetooth.pri)
SOURCEPATH+=/src/libraries/qtopiacomm/bluetooth
TARGET=tst_qbluetoothlocaldevice
SOURCES += tst_qbluetoothlocaldevice.cpp \
           qbluetoothlocaldevicemanager.cpp \
           qbluetoothlocaldevice.cpp
