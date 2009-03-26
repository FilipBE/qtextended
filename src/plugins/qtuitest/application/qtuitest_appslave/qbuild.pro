TEMPLATE=plugin
CONFIG+=qtopia singleexec
TARGET=qtuitest_appslave

PLUGIN_TYPE=qtuitest_application
PLUGIN_FOR=qtopia

MODULES*=qtuitest
MODULE_NAME=qtuitest_appslave

SOURCEPATH+=.. \
    ../../slave_qt  \
    /src/libraries/qtuitest

HEADERS += \
        qtopiasystemtestslave.h

SOURCES += \
        qtopiasystemtestslave.cpp

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

