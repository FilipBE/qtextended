TEMPLATE=app
TARGET=defines
TARGETDIR=.
SOURCES=main.cpp
DEFINES+=FOO
DEFINES+=BAR=1
DEFINES+=BAZ=$$define_value("1 + 1")
DEFINES+=STR=$$define_string("some string")
