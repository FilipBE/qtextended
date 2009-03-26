qtopia_project(qtopiadesktop app)
CONFIG+=qtestlib unittest no_tr
TARGET=tst_$$TARGET

requires(build_qtopiadesktop)

# Pull in the merge code
VPATH+=$$QTOPIA_DEPOT_PATH/src/qtopiadesktop/server
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/qtopiadesktop/server
QT+=sql
HEADERS+=\
    merge.h\
    mergeitem.h\
    qsyncprotocol.h\

SOURCES+=\
    merge.cpp\
    mergeitem.cpp\
    qsyncprotocol.cpp\

# We need to enable some special test harness-only code.
DEFINES+=QSYNCPROTOCOL_DO_NOT_SET_TIME

HEADERS+=fakeplugins.h
SOURCES+=main.cpp

INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/qtopiadesktop/tests/shared
