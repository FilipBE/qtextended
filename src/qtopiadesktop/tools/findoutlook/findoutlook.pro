qtopia_project(qtopiadesktop core app)
requires(win32)
TARGET=findoutlook
CONFIG+=console no_tr no_target
QT=core
DESTDIR=$$QPEDIR/bin

SOURCES+=main.cpp

depends(libraries/qdwin32)
