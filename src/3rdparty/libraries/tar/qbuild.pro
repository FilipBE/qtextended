TEMPLATE=lib
CONFIG+=embedded staticlib use_pic singleexec
TARGET=tar
VERSION=1.1.2

MODULE_NAME=tar
LICENSE=BSD GPL_COMPATIBLE

equals(arch,x86_64) {
    # FIXME this is because CFLAGS_SHLIB isn't being passed (this is a static lib, but it links to a dynamic lib!)
    MKSPEC.CFLAGS+=-fPIC
}

HEADERS=\
    libtar.h\
    libtar_listhash.h

SOURCES=\
    append.c\
    block.c\
    decode.c\
    encode.c\
    extract.c\
    handle.c\
    libtar_hash.c\
    libtar_list.c\
    output.c\
    strlcat.c\
    strlcpy.c\
    strmode.c\
    util.c\
    wrapper.c

DEFINES+=HAVE_LCHOWN HAVE_STRFTIME

pkg.desc=tar library
