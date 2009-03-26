requires(enable_cell)
TEMPLATE=plugin
TARGET=pstnmultiplex

PLUGIN_FOR=qtopia
PLUGIN_TYPE=multiplexers

CONFIG+=qtopia
QTOPIA+=comm

HEADERS=\
    pstnmultiplexer.h

SOURCES=\
    pstnmultiplexer.cpp

