TEMPLATE=lib
CONFIG+=qt
TARGET=qtuitestrunner

MODULE_NAME=qtuitestrunner
QT+=network testlib

MODULES*=qtopiabase::headers
MODULES*=qtopia::headers
MODULES*=realtime

SOURCEPATH=..
SOURCEPATH+=../../lib_qt
SOURCEPATH+=/src/libraries/qtuitest
SOURCEPATH+=/src/libraries/qtopia

include(../../lib_qt/lib_qt.pro)

SEMI_PRIVATE_HEADERS += \
        qtopiasystemtest_p.h

HEADERS +=\
        qtopiasystemtest.h \
        qtopiasystemtestmodem.h

SOURCES +=\
        qtopiasystemtest.cpp \
        qtopiasystemtest_p.cpp

contains(PROJECTS,tools/phonesim) {
    MODULES*=phonesim
    dep.headers.TYPE=DEPENDS PERSISTED METOO
    dep.headers.EVAL="DEFINES+=QTUITEST_USE_PHONESIM"
    SOURCES += qtopiasystemtestmodem.cpp
    RESOURCES += qtopiasystemtestmodem.qrc
}

<script>
// Set up the TESTS_SHARED_DIR define to allow finding the shared scripts directory
var srcdir = qbuild.invoke("path", "/", "existing");
project.property("TESTS_SHARED_DIR").setValue(srcdir + "/tests/shared");
</script>
DEFINES*=TESTS_SHARED_DIR=$$define_string($$TESTS_SHARED_DIR)


