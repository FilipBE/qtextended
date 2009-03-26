# This file contains projects that are part of Qtopia Sync Agent

PROJECTS*=\
    # a placeholder for installing Qt files
    qt\
    libraries/qtopiadesktop\
    server

# Documentation
qtopia_depot:!win32:PROJECTS*=doc

# TCP/IP comms
PROJECTS*=plugins/lan

# Serial comms
PROJECTS*=plugins/serial

# Qtopia 4.x device support
PROJECTS*=plugins/qtopia4

# Windows support code
PROJECTS*=\
    libraries/qdwin32

# Outlook support
PROJECTS*=\
    plugins/outlook\
    tools/findoutlook

include(../custom_qd.pri)
include($$QPEDIR/src/local_qd.pri)

