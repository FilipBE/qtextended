TEMPLATE=lib
CONFIG+=qt
TARGET=phonesim

# This project has to be here for MODULE_NAME. Use SOURCEPATH because the sources aren't here.
MODULE_NAME=phonesim
SOURCEPATH=..

MODULES*=\
    qtopiaphone::headers\
    qtopiacomm::headers\
    qtopia::headers\
    qtopiabase::headers
QT*=xml network

SOURCEPATH+=\
    /src/libraries/qtopiaphone\
    /src/libraries/qtopiacomm\
    /src/libraries/qtopia\
    /src/libraries/qtopiabase

DEFINES+=PHONESIM
HEADERS= phonesim.h server.h hardwaremanipulator.h \
                  qsmsmessagelist.h \
                  qsmsmessage.h \
                  qcbsmessage.h \
                  callmanager.h \
                  simfilesystem.h \
                  simapplication.h \
                  serial/qgsmcodec.h \
                  serial/qatutils.h \
                  serial/qatresultparser.h \
                  serial/qatresult.h \
		  wap/qwsppdu.h \
                  qsimcommand.h \
                  qsimenvelope.h \
                  qsimterminalresponse.h \
                  qsimcontrolevent.h
SOURCES= phonesim.cpp server.cpp hardwaremanipulator.cpp \
                  qsmsmessagelist.cpp \
		  qsmsmessage.cpp \
		  qcbsmessage.cpp \
                  callmanager.cpp \
                  simfilesystem.cpp \
                  simapplication.cpp \
                  serial/qgsmcodec.cpp \
                  serial/qatutils.cpp \
                  serial/qatresultparser.cpp \
                  serial/qatresult.cpp \
		  wap/qwsppdu.cpp \
                  qsimcommand.cpp \
                  qsimenvelope.cpp \
                  qsimterminalresponse.cpp \
                  qsimcontrolevent.cpp

