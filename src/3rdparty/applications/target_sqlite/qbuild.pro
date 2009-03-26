TEMPLATE=app
TARGET=sqlite

CONFIG+=embedded qt
QT=core
CONFIG-=moc

pkg [
    name=sqlite
    desc="SQLite tool for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

release:DEFINES*=NDEBUG
DEFINES+=SQLITE_OMIT_LOAD_EXTENSION SQLITE_CORE

SOURCEPATH=/qtopiacore/qt/src/3rdparty/sqlite

HEADERS=\
    sqlite3.h

SOURCES=\
    sqlite3.c\
    shell.c

