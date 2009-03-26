qtopia_project(qtopiadesktop core lib)
requires(win32)
TARGET=qdwin32
CONFIG+=no_tr
QT=core
# findoutlook needs this here to run
DLLDESTDIR=$$QPEDIR/bin

HEADERS+=\
    qdwin32config.h\
    typeconversion.h\
    registryhelper.h\

SOURCES+=\
    typeconversion.cpp\
    registryhelper.cpp\

headers.files=$$HEADERS
headers.path=/include/qtopiadesktop/qdwin32
headers.hint=headers
INSTALLS+=headers

# The registry functions are defined in this library
idep(LIBS+=-lAdvapi32,LIBS)

idep(LIBS+=-l$$TARGET)
uses_export($$TARGET)
qt_inc($$TARGET)
