# Basic configuration - A Qt Extended application
TEMPLATE=app
CONFIG+=qtopia

# Name the executable
TARGET=example
TARGETDIR=.

# These are the source files that get built to create the application
FORMS=examplebase.ui
HEADERS=example.h
SOURCES=main.cpp example.cpp

## Install the launcher item. The metadata comes from the .desktop file
## and the path here.
#desktop [
#    files=example.desktop
#    path=/apps/Applications
#    trtarget=example-nct
#    hint=nct desktop
#]
#
## Install some pictures.
#pics [
#    files=pics/*
#    path=/pics/example
#    hint=pics
#]
#
## Install help files
#help [
#    source=help
#    files=example.html
#    hint=help
#]
#
## Package information (used for make packages)
#pkg [
#    name=example
#    desc=Example Application
#    version=1.0.0-1
#    maintainer=My Company (www.mycompany.com)
#    license=Commercial
#    domain=window
#]
#
