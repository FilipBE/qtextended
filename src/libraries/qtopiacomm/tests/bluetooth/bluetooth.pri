requires(enable_bluetooth)
CONFIG+=no_tr
PWD=$$path(.,existing)
HEADERS+=$$PWD/qtopiabluetoothunittest.h
SOURCES+=$$PWD/qtopiabluetoothunittest.cpp
MODULES*=qtopiacomm
