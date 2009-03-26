PROJECTS*=\
    devtools\
    devtools/apm\
    devtools/chvol\
    plugins/qtopiacore/kbddrivers/greenphone \
    plugins/audiohardware/greenphone 

!enable_qvfb {
    PROJECTS*=\
        plugins/cameras/greenphone 
}

#the pxa overlay video output plugin is fast, but pxa overlay doesn't initialize each time
#enable_qtopiamedia {
#    PROJECTS*=\
#        plugins/videooutput/pxaoverlay
#}

enable_cell {
    PROJECTS*=\
        devtools/fixbdaddr
}

enable_greenphone_effects {
    PROJECTS *= \
        plugins/qtopiacore/gfxdrivers/greenphone
}
