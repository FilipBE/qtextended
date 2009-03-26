TEMPLATE=app
TARGET=filtering

CONFIG+=qtopia

HEADERS=\
    filterdemo.h

SOURCES=\
    filterdemo.cpp\
    main.cpp

desktop [
    hint=nct desktop
    files=filtering.desktop
    path=/apps/Applications
    trtarget=filtering-nct
]

pics [
    hint=pics
    files=pics/*
    path=/pics/filtering
]

help [
    hint=help
    source=help
    files=*.html
]

    # SXE permissions required
    pkg.domain=trusted
    pkg.name=filtering
    pkg.desc=This is a command line tool used to demonstrate database access and filtering
    pkg.version=1.0.0-1
    pkg.maintainer=Qt Extended <info@qtextended.org>
    pkg.license=GPL
