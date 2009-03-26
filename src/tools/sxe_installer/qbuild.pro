requires(enable_sxe)
TEMPLATE=app
TARGET=sxe_installer

CONFIG+=qt
MODULES*=\
    qtopiabase::headers\
    qtopiasecurity::headers
SOURCEPATH+=\
    /src/libraries/qtopiabase\
    /src/libraries/qtopiasecurity\
    /qtopiacore/qt/src/gui/embedded

DEFINES+=SXE_INSTALLER

HEADERS=\
    qtopiasxe.h\
    qpackageregistry.h\
    qtopianamespace.h\
    keyfiler_p.h\
    qsxepolicy.h

SOURCES=\
    main.cpp\
    qtopiasxe.cpp\
    qpackageregistry.cpp\
    qtopianamespace_lock.cpp\
    keyfiler.cpp\
    qsxepolicy.cpp

