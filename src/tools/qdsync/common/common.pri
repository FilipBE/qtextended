QTOPIADESKTOP_HEADERS+=\
    qserialiodevice.h\
    qserialport.h\
    serialport.h\
    qdlinkhelper.h\
    qdthread.h\
    trace.h\
    qcopchannel_qd.h\
    qcopenvelope_qd.h\

QTOPIADESKTOP_PRIVATE_HEADERS+=\
    qserialiodevice_p.h\

QTOPIADESKTOP_SOURCES+=\
    qserialiodevice.cpp\
    qserialport.cpp\
    serialport.cpp\
    qdlinkhelper.cpp\
    qdthread.cpp\
    trace.cpp\
    qcopchannel_qd.cpp\
    qcopenvelope_qd.cpp\

win32 {
    QTOPIADESKTOP_HEADERS+=iothread.h
    QTOPIADESKTOP_SOURCES+=iothread.cpp
}

