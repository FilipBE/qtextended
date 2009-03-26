TEMPLATE=app
CONFIG+=qtopia unittest

SOURCEPATH+=/src/libraries/mylib
MODULES*=mylib::headers

TARGET=tst_mybutton
HEADERS+=mylib.h
SOURCES+=mylib.cpp main.cpp

