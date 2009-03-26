TEMPLATE=lib
CONFIG+=qtopia staticlib use_pic singleexec
TARGET=g711

MODULE_NAME=g711
LICENSE=FREEWARE

HEADERS+=\
    g711.h\

SOURCES+=\
    g711.c\

headers.TYPE=DEPENDS PERSISTED
headers.EVAL="INCLUDEPATH+="$$path(.,project)

