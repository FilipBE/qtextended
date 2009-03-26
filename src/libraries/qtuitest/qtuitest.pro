# NOTE: this file is used for building QtUiTest from outside of Qtopia.
# Please don't delete this file or the non-qbuild portions of it.

!qbuild:isEmpty(QTOPIA_PROJECT_ROOT) {
    CONFIG+=standalone
}

!qbuild:!standalone {
    qtopia_project(qtopia lib)
    TARGET=qtuitest
    CONFIG += no_tr
}
DEFINES+=QTUITEST_TARGET

PRIVATE_HEADERS +=        \
    qeventwatcher_p.h     \
    qinputgenerator_p.h   \
    qtuitestconnectionmanager_p.h \
    qtuitestwidgets_p.h

SEMI_PRIVATE_HEADERS += \
    demowidgets_p.h     \
    qalternatestack_p.h \
    qelapsedtimer_p.h   \
    qtestprotocol_p.h   \
    recordevent_p.h

HEADERS +=                      \
    qtuitestglobal.h          \
    qtuitestnamespace.h       \
    qtuitestrecorder.h        \
    qtuitestwidgetinterface.h

SOURCES +=                        \
    demowidgets.cpp               \
    qelapsedtimer.cpp             \
    qeventwatcher.cpp             \
    qinputgenerator.cpp           \
    qtestprotocol.cpp             \
    qtuitestconnectionmanager.cpp \
    qtuitestnamespace.cpp       \
    qtuitestrecorder.cpp        \
    qtuitestwidgetinterface.cpp \
    qtuitestwidgets.cpp

unix {
    SOURCES+=qalternatestack_unix.cpp
    embedded {
        SOURCES += qinputgenerator_qws.cpp
    } else {
        SOURCES += qinputgenerator_x11.cpp
        LIBS    += -lXtst
    }
}

win32 {
    SOURCES += qalternatestack_win.cpp  \
               qinputgenerator_win.cpp
}

!qbuild:!standalone {
    headers.files=$$HEADERS
    headers.path=/include/qtuitest
    headers.hint=sdk headers
    INSTALLS+=headers

    pheaders.files=$$SEMI_PRIVATE_HEADERS
    pheaders.path=/include/qtuitest/private
    pheaders.hint=sdk headers
    INSTALLS+=pheaders

    depends(libraries/qtopiacore/qtestlib)
    idep(LIBS*=-l$$TARGET)
    idep(CONFIG += qtestlib,CONFIG)
    qt_inc($$TARGET)
}

standalone {
    TEMPLATE=lib
    MOC_DIR=$$OUT_PWD/.moc
    OBJECTS_DIR=$$OUT_PWD/.obj
    QT+=network
    HEADERS+=$$SEMI_PRIVATE_HEADERS $$PRIVATE_HEADERS
    VPATH+=$$PWD
    INCLUDEPATH+=$$PWD
    TARGET=qtuitest
}
