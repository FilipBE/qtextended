REQUIRES=enable_bluetooth
QTOPIA*=comm
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\

HEADERS+=\
        bluetoothservicemanager.h

SOURCES+=\
        bluetoothservicemanager.cpp

bluetooth_serv_settings [
    hint=image
    files=$$SERVER_PWD/etc/default/Trolltech/BluetoothServices.conf
    path=/etc/default/Trolltech
]

