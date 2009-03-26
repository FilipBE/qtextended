qtopia_project(qtopiadesktop app)
CONFIG+=qtestlib unittest no_tr
TARGET=tst_$$TARGET

requires(build_qtopiadesktop)
requires(win32)

# Pull in the plugin manager code
include(../pluginmanager.pri)

# Pull in the outlook plugin's code
include(../outlookplugin.pri)
SOURCES+=datebook.cpp

MAIN_SOURCES=$$PWD/main.cpp.in
COMMON_TESTS=$$PWD/common_tests.cpp
include(../main.pri)
