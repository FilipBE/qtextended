TEMPLATE=app
TARGET=system_qt
SOURCES=main.cpp
QT+=sql svg
contains(QT_CONFIG,embedded):error(Cannot use Qt Embedded as the system Qt!)
