win32:qtopia_project(qtopiadesktop plugin)
else:qtopia_project(stub)
TARGET=outlook

HEADERS+=\
    qmapi.h\
    qoutlook.h\
    sp.h\
    outlooksync.h\
    outlookthread.h\

SOURCES+=\
    qmapi.cpp\
    qoutlook.cpp\
    outlooksync.cpp\
    outlookthread.cpp\
    addressbook.cpp\
    datebook.cpp\
    todo.cpp\

win32 {

    depends(tools/findoutlook,fake)
    system($$fixpath($$QPEDIR/bin/findoutlook)) {
        HEADERS+=qoutlook_detect.h
        SOURCES+=qoutlook_detect.cpp
    } else {
        requires(Outlook)
    }

    depends(libraries/qdwin32)

    LIBS+=-lMAPI32
    QMAKE_LFLAGS+=/NODEFAULTLIB:LIBC

    pics.files=pics/*
    pics.path=$$resdir/pics/outlook
    pics.hint=pics
    INSTALLS+=pics

} else {

    # This is here so that make lupdate can work
    TRANSLATABLES+=$$HEADERS $$SOURCES
    HEADERS=
    SOURCES=
    CONFIG+=i18n
    CONFIG-=no_tr
    CONFIG-=installs
    TRTARGET=lib$$TARGET

}

