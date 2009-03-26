TEMPLATE=lib
CONFIG+=embedded
TARGET=timidity
VERSION=1.0.0

MODULE_NAME=libtimidity
LICENSE=LGPL

MODULES+=mathlib

HEADERS     = src/common.h \
                src/dls1.h \
                src/dls2.h \
                src/instrum.h \
                src/instrum_dls.h \
                src/mix.h \
                src/options.h \
                src/output.h \
                src/playmidi.h \
                src/readmidi.h \
                src/resample.h \
                src/tables.h \
                src/timidity.h \
                src/timidity_internal.h

SOURCES = src/common.c \
            src/instrum.c \
            src/instrum_dls.c \
            src/mix.c \
            src/output.c \
            src/playmidi.c \
            src/readmidi.c \
            src/resample.c \
            src/stream.c \
            src/tables.c \
            src/timidity.c

equals(QTOPIA_TARGET_ENDIAN,little):DEFINES+=LITTLE_ENDIAN
else:DEFINES+=BIG_ENDIAN
DEFINES+=HAVE_CONFIG_H

pkg.desc=Timidity MIDI library
pkg.domain=trusted

patches.files=$$QTOPIA_DEPOT_PATH/src/3rdparty/libraries/libtimidity/config/*
patches.path=/etc/timidity
INSTALLS+=patches

timidityconfig.commands=\
    "cat "$$path(.,project)"/timidity.cfg | sed 's:QTOPIA_PREFIX:$$QTOPIA_PREFIX:' >$$QTOPIA_IMAGE/etc/timidity/timidity.cfg"
timidityconfig.path=/etc/timidity
INSTALLS+=timidityconfig

