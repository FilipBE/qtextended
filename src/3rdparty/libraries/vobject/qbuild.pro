TEMPLATE=lib
CONFIG+=qtopia staticlib use_pic singleexec
TARGET=vobject

MODULE_NAME=vobject
LICENSE=FREEWARE

PRIVATE_HEADERS+=\
    vcc_yacc_p.h\
    vobject_p.h\

SOURCES+=\
    vcc_yacc.cpp\
    vobject.cpp\

headers.TYPE=DEPENDS PERSISTED
headers.EVAL="INCLUDEPATH+="$$path(.,project)

