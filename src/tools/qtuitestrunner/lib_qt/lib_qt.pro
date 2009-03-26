# NOTE: this file is used for building QtUiTest from outside of Qtopia.
# Please don't delete this file or the non-qbuild portions of it.

!qbuild:isEmpty(QTOPIA_PROJECT_ROOT) {
    CONFIG+=standalone
}

DEFINES += QTOPIA_TEST_HOST

FORMS   +=\
        manualverificationdlg.ui \
        failuredlg.ui \
        recorddlg.ui

SEMI_PRIVATE_HEADERS += \
    qcircularbuffer_p.h \
    qsystemtestmaster_p.h \
    qsystemtest_p.h \
    qelapsedtimer_p.h \
    qtestprotocol_p.h \
    qtestremote_p.h \
    qtestverifydlg_p.h \
    recordevent_p.h

HEADERS +=\
        gracefulquit.h \
        qabstracttest.h \
        qsystemtest.h

SOURCES +=\
        gracefulquit.cpp \
        qabstracttest.cpp \
        qelapsedtimer.cpp \
        qtestprotocol.cpp \
        qtestremote.cpp \
        qtestverifydlg.cpp \
        qsystemtest.cpp \
        qsystemtest_p.cpp \
        qsystemtestmaster.cpp

VPATH+=$$PWD

INCLUDEPATH+=$$PWD

standalone {
    HEADERS*=$$SEMI_PRIVATE_HEADERS $$PRIVATE_HEADERS
    VPATH      +=$$(QTOPIA_DEPOT_PATH)/src/libraries/qtuitest
    INCLUDEPATH+=$$(QTOPIA_DEPOT_PATH)/src/libraries/qtuitest
    CONFIG+=qtestlib
    QT+=network

    win32:DEFINES+=strcasecmp=_stricmp
}

