requires(enable_cell)
TEMPLATE=plugin
TARGET=multiportmultiplex

PLUGIN_FOR=qtopia
PLUGIN_TYPE=multiplexers

CONFIG+=qtopia
QTOPIA+=comm

HEADERS=\
    multiportmultiplexer.h

SOURCES=\
    multiportmultiplexer.cpp

