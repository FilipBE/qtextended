# NOTE: this file is used for building QtUiTest from outside of Qtopia.
# Please don't delete this file or the non-qbuild portions of it.

!qbuild:isEmpty(QTOPIA_PROJECT_ROOT) {
    CONFIG+=standalone
}

TARGET=qtuitestoverrides
SOURCES = \
        overrides.c

standalone {
    TEMPLATE=lib
    MOC_DIR=$$OUT_PWD/.moc
    OBJECTS_DIR=$$OUT_PWD/.obj
    VPATH+=$$PWD
    INCLUDEPATH+=$$PWD
}

