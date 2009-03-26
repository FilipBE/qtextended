QTOPIA*=comm
UNIFIED_NCT_LUPDATE=1
SERVER_DEPS*=\
    /src/server/core_server\

HEADERS+=\
        usbgadgettask.h

SOURCES+=\
        usbgadgettask.cpp

usbgadget [
    hint=image optional
    files=$$SERVER_PWD/etc/default/Trolltech/Usb.conf
    path=/etc/default/Trolltech
]

usbethernetservice [
    hint=image
    files=$$SERVER_PWD/services/UsbGadget/Ethernet/qpe
    path=/services/UsbGadget/Ethernet
]

usbstorageservice [
    hint=image
    files=$$SERVER_PWD/services/UsbGadget/Storage/qpe
    path=/services/UsbGadget/Storage
]

usbdesktop [
    hint=desktop
    files=$$SERVER_PWD/usbconnectionmode.desktop
    path=/apps/Settings
]

usbservicehelp [
    hint=help
    source=$$SERVER_PWD/help
    files=*.html
]

