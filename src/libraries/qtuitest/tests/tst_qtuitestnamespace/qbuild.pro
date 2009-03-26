TEMPLATE=app
CONFIG+=qtopia unittest
SOURCEPATH+= /src/libraries/qtuitest
TARGET=tst_qtuitestnamespace
MOC_COMPILE_EXCEPTIONS+=qtuitestwidgets_p.h
SOURCES+= \
    tst_qtuitestnamespace.cpp \
    demowidgets.cpp \
    qalternatestack_unix.cpp \
    qelapsedtimer.cpp \
    qeventwatcher.cpp \
    qinputgenerator.cpp \
    qinputgenerator_qws.cpp \
    qtuitestconnectionmanager.cpp \
    qtuitestnamespace.cpp \
    qtuitestrecorder.cpp \
    qtuitestwidgets.cpp

HEADERS+= \
    demowidgets_p.h \
    qalternatestack_p.h \
    qelapsedtimer_p.h \
    qeventwatcher_p.h \
    qinputgenerator_p.h \
    qtuitestconnectionmanager_p.h \
    qtuitestnamespace.h \
    qtuitestrecorder.h \
    qtuitestwidgets_p.h

