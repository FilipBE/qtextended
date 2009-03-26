# Basic configuration - A non-Qt application
TEMPLATE=app

# Name the executable
TARGET=testme
TARGETDIR=.

# Source used to build the project
SOURCES=main.cpp

# This is how to install the executable
target [
    path=$$path(.,project)/bin
    hint=install
]

# This gets us to install main.cpp next to the executable
sources [
    path=$$path(.,project)/bin
    files=main.cpp
    hint=install
]

## This gets us to install some text files next to the executable
#textfiles [
#    path=$$path(.,project)/bin
#    files=*.txt
#    hint=install
#]

