TEMPLATE=app
CONFIG+=qtopia no_tr
TARGET=bluetoothservice

QTOPIA+=comm

HEADERS=bluetoothservice.h
SOURCES=bluetoothservice.cpp \
        main.cpp

# include the SDP record XML file
# /etc/bluetooth/sdp is where other Qtopia SDP record XML files are located
sdprecord.hint=image
sdprecord.files=SerialPortSDPRecord.xml
sdprecord.path=/etc/bluetooth/sdp

desktop.hint=desktop
desktop.files=bluetoothservice.desktop
desktop.path=/apps/Applications

# Package information (used for make packages)
pkg.name=bluetoothservice
pkg.desc=Bluetooth service example
pkg.domain=trusted

requires(enable_bluetooth)

