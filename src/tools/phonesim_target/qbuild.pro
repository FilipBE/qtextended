TEMPLATE=app
TARGET=phonesim

CONFIG+=qt embedded
QT*=xml
MODULES*=\
    qtopiaphone::headers\
    qtopiacomm::headers\
    qtopia::headers\
    qtopiabase::headers\
    phonesim::headers

SOURCEPATH+=\
    ../phonesim\
    ../phonesim/lib\
    /src/libraries/qtopiacomm\
    /src/libraries/qtopiaphone\
    /src/libraries/qtopiabase

DEFINES+=PHONESIM PHONESIM_TARGET

# This stops qtopiaipcmarshal.h including the D-BUS marshalling stubs,
# even though technically we are not building a host binary.
DEFINES+=QTOPIA_HOST

pkg [
    name=phonesim
    desc="Phone simulator for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    phonesim.h\
    server.h\
    simfilesystem.h\
    callmanager.h\
    serial/qgsmcodec.h\
    serial/qatutils.h\
    serial/qatresultparser.h\
    serial/qatresult.h\
    simapplication.h\
    qsimcommand.h\
    qsimterminalresponse.h\
    qsimenvelope.h\
    qsimcontrolevent.h\
    qtopialog.h

SOURCES=\
    main.cpp\
    phonesim.cpp\
    server.cpp\
    callmanager.cpp\
    simfilesystem.cpp\
    serial/qgsmcodec.cpp\
    serial/qatutils.cpp\
    serial/qatresultparser.cpp\
    serial/qatresult.cpp\
    simapplication.cpp\
    qsimcommand.cpp\
    qsimterminalresponse.cpp\
    qsimenvelope.cpp\
    qsimcontrolevent.cpp\
    qtopialog.cpp

