include(common.pri)

# Qtopia-specific defaults are set by the defaults extension
#CONFIG+=defaults

# Pull in src/config.pri
include(QtopiaSdk:/configs/current.pri)

# compatibility hacks
CONFIG*=unix phone

# Setup PROJECTS
CONFIG+=projects
