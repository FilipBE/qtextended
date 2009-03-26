disable_project("blend doesn't exist!")
TEMPLATE=plugin
CONFIG+=qtopia
TARGET=greenphonescreen

#MODULES*=blend

PLUGIN_FOR=qt
PLUGIN_TYPE=gfxdrivers

HEADERS += greenphonescreendriverplugin.h greenphonescreen.h
SOURCES += greenphonescreendriverplugin.cpp greenphonescreen.cpp

