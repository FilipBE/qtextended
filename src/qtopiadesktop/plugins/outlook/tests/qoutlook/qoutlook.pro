qtopia_project(qtopiadesktop app)
CONFIG+=qtestlib unittest no_tr
TARGET=tst_$$TARGET
SOURCES=main.cpp

requires(build_qtopiadesktop)
requires(win32)

# Pull in the outlook plugin's code
include(../outlookplugin.pri)

INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/qtopiadesktop/tests/shared

