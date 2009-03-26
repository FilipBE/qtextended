requires(enable_cell)
TEMPLATE=plugin
TARGET=modifiedgsm0710multiplex

PLUGIN_FOR=qtopia
PLUGIN_TYPE=multiplexers

CONFIG+=qtopia
QTOPIA+=comm

HEADERS=\
    modifiedgsm0710.h

SOURCES=\
    modifiedgsm0710.cpp

