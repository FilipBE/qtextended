
# common

PROJECTS*=\
    plugins/qtopiacore/kbddrivers/n810\
    plugins/audiohardware/nokia\
    plugins/qtopiacore/gfxdrivers/nokia\
    plugins/whereabouts/n810 \
    devtools\
    devtools/logread

enable_dbus:SERVER_PROJECTS*=\
    server/n810/battery\
    server/n810/hardware
   

!enable_qvfb {
    PROJECTS*=\
        plugins/cameras/v4lwebcams
}

equals(QTOPIA_UI,home) {
   # Home edition projects
 
    SERVER_PROJECTS*=\
        server/media/genericvolumeservice

} else {
   # Phone edition projects


}

