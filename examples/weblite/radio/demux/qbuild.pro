TEMPLATE=app
TARGET=radio_demux

CONFIG+=embedded

pkg [
    name=radiodemux
    desc="Radio demultiplexer."
    version=1.0.0-1
    license="GPL v2"
    maintainer="Qt Extended <info@qtextended.org>"
]

SOURCES=\
    radio_demux.cpp

