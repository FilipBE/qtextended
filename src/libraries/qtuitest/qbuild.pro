TEMPLATE=lib
CONFIG+=qtopia hide_symbols singleexec
TARGET=qtuitest
VERSION=4.0.0

MODULE_NAME=qtuitest

MOC_COMPILE_EXCEPTIONS+=qtuitestwidgets_p.h

include(qtuitest.pro)
