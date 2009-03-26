TEMPLATE=app
TARGET=notesdemo

CONFIG+=qtopia

HEADERS=\
    notesdemo.h

SOURCES=\
    main.cpp\
    notesdemo.cpp

desktop [
    hint=desktop
    files=notesdemo.desktop
    path=/apps/Applications
]

pics [
    hint=pics
    files=pics/*
    path=/pics/notesdemo
]

help [
    hint=help
    source=help
    files=*.html
]

