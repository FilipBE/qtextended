REQUIRES=enable_bluetooth
QTOPIA*=comm pim
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\
    /src/server/comm/obex\
    /src/server/comm/filetransfer\
    /src/server/bluetooth/powermgr::exists\

HEADERS+=\
        bluetoothfilesendservice.h

SOURCES+=\
        bluetoothfilesendservice.cpp

btpushservice [
    hint=image
    files=$$SERVER_PWD/services/BluetoothPush/qpe
    path=/services/BluetoothPush
]

btqdsservice [
    hint=image
    files=$$SERVER_PWD/etc/qds/BluetoothPush
    path=/etc/qds
]

