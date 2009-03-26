requires(enable_cell)
TEMPLATE=plugin
CONFIG+=qtopia
TARGET=greenphonevendor

QTOPIA*=phonemodem

PLUGIN_FOR=qtopia
PLUGIN_TYPE=phonevendors

HEADERS		=  vendor_greenphone_p.h greenphoneplugin.h
SOURCES	        =  vendor_greenphone.cpp greenphoneplugin.cpp

