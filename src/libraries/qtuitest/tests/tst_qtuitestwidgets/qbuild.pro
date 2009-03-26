TEMPLATE=app
CONFIG+=qtopia unittest
SOURCEPATH+= /src/libraries/qtuitest
TARGET=tst_qtuitestwidgets
MOC_COMPILE_EXCEPTIONS+=qtuitestwidgets_p.h

SOURCES+= \
    tst_qtuitestwidgets.cpp \
    demowidgets.cpp \
    qalternatestack_unix.cpp \
    qeventwatcher.cpp \
    qinputgenerator.cpp \
    qinputgenerator_qws.cpp \
    qtuitestconnectionmanager.cpp \
    qtuitestnamespace.cpp \
    qtuitestrecorder.cpp \
    qtuitestwidgets.cpp \
    qtuitestwidgetinterface.cpp

HEADERS+= \
    demowidgets_p.h \
    qalternatestack_p.h \
    qeventwatcher_p.h \
    qinputgenerator_p.h \
    qtuitestconnectionmanager_p.h \
    qtuitestnamespace.h \
    qtuitestrecorder.h \
    qtuitestwidgets_p.h

DEFINES+=QT_STATICPLUGIN

