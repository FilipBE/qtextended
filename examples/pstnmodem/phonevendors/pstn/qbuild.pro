requires(enable_cell)
TEMPLATE=plugin
TARGET=pstnvendor

PLUGIN_FOR=qtopia
PLUGIN_TYPE=phonevendors

CONFIG+=qtopia
QTOPIA+=phonemodem

HEADERS=\
    vendor_pstn_p.h\
    pstnplugin.h

SOURCES=\
    vendor_pstn.cpp\
    pstnplugin.cpp

