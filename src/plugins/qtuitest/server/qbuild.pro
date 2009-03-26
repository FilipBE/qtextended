TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=qtuitest_qpeslave

PLUGIN_TYPE=qtuitest_server
PLUGIN_FOR=qtopia

MODULES*=qtuitest qtuitest_appslave::headers
SOURCEPATH+=/src/libraries/qtuitest
# This uses symbols from another plugin!
CONFIG-=link_test

HEADERS +=                      \
        qmemorysampler.h        \
        qtopiaservertestslave.h \
        qtuitestlogreader_p.h
SOURCES +=                          \
        qmemorysampler.cpp          \
        qtopiaservertestslave.cpp   \
        qtuitestlogreader.cpp

MOC_COMPILE_EXCEPTIONS=qtuitestlogreader_p.h

INCLUDEPATH += \
    $$QTOPIA_DEPOT_PATH/src/plugins/qtuitest/slave_qt \
    $$QTOPIA_DEPOT_PATH/src/libraries/qtuitest

