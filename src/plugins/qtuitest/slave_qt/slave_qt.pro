# NOTE: this file is used for building QtUiTest from outside of Qtopia.
# Please don't delete this file or the non-qbuild portions of it.

!qbuild:isEmpty(QTOPIA_PROJECT_ROOT) {
    CONFIG+=standalone
}

VPATH+=$$PWD
INCLUDEPATH+=$$PWD

HEADERS += \
        qtestslave.h \
        qtestslaveglobal.h \
        qtestwidgets.h

SOURCES += \
        qtestslave.cpp \
        qtestwidgets.cpp

standalone {
    TEMPLATE=lib
    MOC_DIR=$$OUT_PWD/.moc
    OBJECTS_DIR=$$OUT_PWD/.obj
    INCLUDEPATH+=$$(QTOPIA_DEPOT_PATH)/src/libraries/qtuitest
    TARGET=qtslave
}

