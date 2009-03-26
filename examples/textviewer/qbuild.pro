TEMPLATE=app
TARGET=textviewer

CONFIG+=qtopia

HEADERS=\
    textviewer.h

SOURCES=\
    main.cpp\
    textviewer.cpp

desktop [
    hint=nct desktop
    files=textviewer.desktop
    path=/apps/Applications
    trtarget=textviewer-nct
]

