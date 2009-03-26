TEMPLATE=app
CONFIG+=embedded
TARGET=atd

SOURCES = atd.cpp greenphone.cpp
HEADERS = greenphone.h

at.files=at
at.path=/bin
at.hint=script
INSTALLS+=at

pkg.desc=atd daemon
pkg.domain=trusted

