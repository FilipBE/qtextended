TEMPLATE=app
TARGET=simpleobexdemo

CONFIG+=qtopia
QTOPIA+=comm

HEADERS=\
    obexclientwindow.h\
    obexquoteserver.h

SOURCES=\
    main.cpp\
    obexclientwindow.cpp\
    obexquoteserver.cpp

desktop [
    hint=desktop
    files=simpleobexdemo.desktop
    path=/apps/Applications
]

