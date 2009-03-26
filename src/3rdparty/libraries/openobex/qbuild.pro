TEMPLATE=lib
CONFIG+=embedded
TARGET=openobex

MODULE_NAME=openobex
LICENSE=LGPL

DEFINES+=HAVE_CONFIG_H

HEADERS = config.h \
    btobex.h \
    inobex.h \
    irda.h \
    irda_wrap.h \
    irobex.h \
    databuffer.h \
    openobex/obex.h \
    obex_client.h \
    obex_connect.h \
    openobex/obex_const.h \
    obex_header.h \
    obex_main.h \
    obex_object.h \
    obex_server.h \
    obex_transport.h

SOURCES = btobex.c \
    inobex.c \
    irobex.c \
    databuffer.c \
    obex.c \
    obex_client.c \
    obex_connect.c \
    obex_header.c \
    obex_main.c \
    obex_object.c \
    obex_server.c \
    obex_transport.c

win32:SOURCES += win32compat.c

