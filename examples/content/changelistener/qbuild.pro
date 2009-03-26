TEMPLATE=app
TARGET=changelistener

CONFIG+=qtopia

HEADERS=\
    changelistener.h

SOURCES=\
    main.cpp\
    changelistener.cpp

desktop [
    hint=nct desktop
    files=changelistener.desktop
    path=/apps/Applications
    trtarget=notesdemo-nct
]

pics [
    hint=pics
    files=pics/*
    path=/pics/changelistener
]

help [
    hint=help
    source=help
    files=*.html
]

