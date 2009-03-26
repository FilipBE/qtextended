# include() this .pri file in any unit test which wants to use
# the functionality in qfuturesignal.h

PWD=$$path(.,existing)

SOURCES*=\
        $$PWD/qfuturesignal.cpp

HEADERS*=\
        $$PWD/qfuturesignal.h

INCLUDEPATH*=$$PWD

MODULES*=qtopiabase

