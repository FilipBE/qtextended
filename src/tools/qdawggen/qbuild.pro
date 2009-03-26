TEMPLATE=app
TARGET=qdawggen

CONFIG+=qt
MODULES*=\
    qtopia::headers\
    qtopiabase::headers

SOURCEPATH+=/src/libraries/qtopiabase

HEADERS=\
    qdawg.h\
    global.h\
    qmemoryfile_p.h

SOURCES=\
    main.cpp\
    qdawg.cpp\
    global.cpp\
    qmemoryfile.cpp\
    qmemoryfile_unix.cpp

