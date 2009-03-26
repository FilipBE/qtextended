TEMPLATE=lib
CONFIG+=embedded singleexec
TARGET=gsm
VERSION=1.0.0

MODULE_NAME=gsm
LICENSE=FREEWARE

HEADERS=\
    gsm.h\

SOURCES=\
    add.c\
    code.c\
    decode.c\
    gsm_create.c\
    gsm_decode.c\
    gsm_destroy.c\
    gsm_encode.c\
    gsm_option.c\
    long_term.c\
    lpc.c\
    preprocess.c\
    rpe.c\
    short_term.c\
    table.c\

DEFINES+=FAST SASR WAV49

pkg.desc=GSM library
pkg.domain=trusted

