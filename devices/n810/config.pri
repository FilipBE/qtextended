equals(QTOPIA_UI,home) {
    # Home edition projects
    DEFINES+=QTOPIA_HOMEUI_WIDE
}

# common

#DEFINES+=DEBUG
DEFINES+=QT_QWS_N810
DEFINES+=HAVE_V4L2

LAN_NETWORK_CONFIGS=wlan lan
