VPATH+=$$QTOPIA_DEPOT_PATH/src/qtopiadesktop/plugins/outlook
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/qtopiadesktop/plugins/outlook

HEADERS+=\
    qmapi.h\
    qoutlook.h\
    qoutlook_detect.h\
    sp.h\
    outlooksync.h\
    outlookthread.h\

SOURCES+=\
    qmapi.cpp\
    qoutlook.cpp\
    qoutlook_detect.cpp\
    outlooksync.cpp\
    outlookthread.cpp\

LIBS+=-lMAPI32 -lAdvAPI32
QMAKE_LFLAGS+=/NODEFAULTLIB:LIBC

depends(libraries/qdwin32)
