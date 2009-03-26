TEMPLATE = lib
CONFIG += qt warn_on qdbus create_prl link_pkgconfig

contains(QT_CONFIG, reduce_exports):CONFIG += hide_symbols
contains(QT_CONFIG, debug):contains(QT_CONFIG, release):CONFIG += debug_and_release build_all
contains(QT_CONFIG, embedded):CONFIG += embedded

TARGET = QtBluetooth

CONFIG(debug, debug|release) {
    OBJECTS_DIR = tmp/debug
} else {
    OBJECTS_DIR = tmp/release
}

QT = core xml

INCLUDEPATH += .

MOC_DIR = tmp
VERSION         = 4.3.0

embedded:QMAKE_CXXFLAGS+=-fno-rtti

DESTDIR=../lib

BLUETOOTH_HEADERS+=\
    qbluetoothaddress.h\
    qbluetoothlocaldevice.h\
    qbluetoothlocaldevicemanager.h\
    qbluetoothnamespace.h\
    qbluetoothpasskeyagent.h\
    qbluetoothpasskeyrequest.h\
    qbluetoothremotedevice.h\
    qbluetoothabstractserver.h\
    qbluetoothscoserver.h\
    qbluetoothrfcommserver.h\
    qbluetoothabstractsocket.h\
    qbluetoothscosocket.h\
    qbluetoothrfcommsocket.h\
    qbluetoothsdpquery.h\
    qbluetoothsdprecord.h\
    qbluetoothsdpuuid.h\
    qbluetoothrfcommserialport.h \
    qbluetoothl2capsocket.h \
    qbluetoothl2capserver.h \
    qbluetoothl2capdatagramsocket.h \
    qbluetoothglobal.h

BLUETOOTH_OBEX_HEADERS += \
    qbluetoothobexserver.h\
    qbluetoothobexsocket.h

BLUETOOTH_SOURCES+=\
    qbluetoothaddress.cpp\
    qbluetoothlocaldevice.cpp\
    qbluetoothlocaldevicemanager.cpp\
    qbluetoothnamespace.cpp\
    qbluetoothpasskeyagent.cpp\
    qbluetoothpasskeyrequest.cpp\
    qbluetoothabstractserver.cpp\
    qbluetoothscoserver.cpp\
    qbluetoothrfcommserver.cpp\
    qbluetoothabstractsocket.cpp\
    qbluetoothscosocket.cpp\
    qbluetoothrfcommsocket.cpp\
    qbluetoothremotedevice.cpp\
    qbluetoothsdpquery.cpp\
    qbluetoothsdprecord.cpp\
    qbluetoothsdpuuid.cpp\
    qbluetoothrfcommserialport.cpp \
    qsdpxmlparser.cpp \
    qsdpxmlgenerator.cpp \
    qbluetoothl2capsocket.cpp \
    qbluetoothl2capserver.cpp \
    qbluetoothl2capdatagramsocket.cpp

BLUETOOTH_OBEX_SOURCES += \
    qbluetoothobexserver.cpp\
    qbluetoothobexsocket.cpp

BLUETOOTH_PRIVATE_HEADERS+=\
    qbluetoothnamespace_p.h\
    qsdpxmlparser_p.h \
    qsdpxmlgenerator_p.h \
    qbluetoothabstractsocket_p.h

enable_obex {
	BLUETOOTH_SOURCES += $$BLUETOOTH_OBEX_SOURCES
	BLUETOOTH_HEADERS += $$BLUETOOTH_OBEX_HEADERS

	INCLUDEPATH+=$$QOBEX_DIR/include/QtObex
	LIBS+=-L$$QOBEX_DIR/lib -lQtObex
}

SOURCES += $$BLUETOOTH_SOURCES
HEADERS += $$BLUETOOTH_HEADERS $$BLUETOOTH_PRIVATE_HEADERS

TARGET = $$qtLibraryTarget($$TARGET)
target.path=$$QBLUETOOTH_PREFIX/lib

targ_headers.files = $$BLUETOOTH_HEADERS
targ_headers.path = $$QBLUETOOTH_PREFIX/include/QtBluetooth

targ_headers_local.commands = mkdir -p ../include/QtBluetooth ; \
        for file in $$BLUETOOTH_HEADERS; do \
        $(INSTALL_FILE) "\$$file" ../include/QtBluetooth; \
        done
targ_headers_local.CONFIG+=no_default_install

unix {
   CONFIG += create_libtool create_pc explicitlib
   QMAKE_PKGCONFIG_LIBDIR = $$QBLUETOOTH_PREFIX/lib
   QMAKE_PKGCONFIG_INCDIR = $$QBLUETOOTH_PREFIX/include/QtBluetooth
   QMAKE_PKGCONFIG_DESCRIPTION = Qtopia/Qt Bluetooth Module
   QMAKE_PKGCONFIG_DESTDIR = pkgconfig
   QMAKE_PKGCONFIG_NAME = QtBluetooth
}

targ_pkgconfig.path = $$QBLUETOOTH_PREFIX/pkgconfig

qtincludes.path = $$QBLUETOOTH_PREFIX/include/QtBluetooth
qtincludes.commands = for file in qtincludes/*; do \
	$(INSTALL_FILE) "\$$file" $$QBLUETOOTH_PREFIX/include/QtBluetooth; \
	done

qtincludes_local.commands = mkdir -p ../include/QtBluetooth ; \
        for file in qtincludes/*; do \
        $(INSTALL_FILE) "\$$file" ../include/QtBluetooth; \
        done
qtincludes_local.CONFIG+=no_default_install

QMAKE_EXTRA_TARGETS += qtincludes_local targ_headers_local

INSTALLS += target targ_headers targ_pkgconfig qtincludes
ALL_DEPS += qtincludes_local targ_headers_local

