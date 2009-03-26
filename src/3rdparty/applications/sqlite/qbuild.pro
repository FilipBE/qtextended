TEMPLATE=app
TARGET=sqlite

CONFIG+=qt
QT=core
CONFIG-=moc

release:DEFINES*=NDEBUG
DEFINES+=SQLITE_OMIT_LOAD_EXTENSION SQLITE_CORE

SOURCEPATH=/qtopiacore/qt/src/3rdparty/sqlite

HEADERS=\
    sqlite3.h

SOURCES=\
    sqlite3.c\
    shell.c

