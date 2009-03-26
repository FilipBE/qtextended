# This is for regular projects (loaded before qbuild.pro)
#message(default.pri)

DEFAULT_CONFIG=depends installs
CONFIG*=$$DEFAULT_CONFIG

# build system crap
QMAKE.RULES.TYPE = MAKERULES
QMAKE.RULES.clean.PREREQUISITES += clean_objects clean_depends
QMAKE.VARIABLES.TYPE = MAKEVARIABLES
#QMAKE.DEPENDS.TYPE = DEPENDENCY

include(QtopiaSdk:/configs/current.pri)
add_extra_paths()

QPEDIR=$$path(/,generated)

!isEmpty(DEVICE_CONFIG_PATH) {
    INCLUDEPATH+=$$DEVICE_CONFIG_PATH/include
    LIBS+=-L$$DEVICE_CONFIG_PATH/lib
}

# compatibility hacks
CONFIG*=unix phone

# Setup PROJECTS
CONFIG+=projects

# FIXME stupid compatibility hacks
QTOPIA_DEPOT_PATH=
PWD=.
