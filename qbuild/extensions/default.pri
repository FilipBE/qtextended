# This is for regular projects (loaded before qbuild.pro)
#message(default.pri)

DEFAULT_CONFIG=depends installs i18n
CONFIG*=$$DEFAULT_CONFIG

# Qtopia-specific defaults are set by the defaults extension
CONFIG+=defaults
# Pull in src/config.pri
include(QtopiaSdk:/configs/current.pri)
add_extra_paths()

# Slightly dodgy check to see if we're bootstrapping the SDK
# config.status exists at the top of the build tree so if it exists
# assume we're building from the build tree. Extra dependencies
# will be enabled.
exists($$path(/config.status,generated)):CONFIG+=in_build_tree

QPEDIR=$$path(/,generated)

!isEmpty(DEVICE_CONFIG_PATH) {
    INCLUDEPATH+=$$DEVICE_CONFIG_PATH/include
    MKSPEC.LFLAGS+=-L$$DEVICE_CONFIG_PATH/lib
}

# compatibility hacks
CONFIG*=unix phone

# Setup PROJECTS
CONFIG+=projects

# A special hook for linking to singleexec_link libraries
enable_singleexec:CONFIG+=singleexec_libs

# FIXME stupid compatibility hacks
QTOPIA_DEPOT_PATH=
PWD=.

ensure_dot_qbuild [
    TYPE=RULE
    commands="$$MKSPEC.MKDIR $$path(.qbuild,generated)"
]

