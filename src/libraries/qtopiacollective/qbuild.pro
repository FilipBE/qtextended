TEMPLATE=lib
TARGET=qtopiacollective
MODULE_NAME=qtopiacollective
VERSION=4.0.0

CONFIG+=qtopia hide_symbols singleexec

pkg [
    name=libcollective
    desc="Collective library for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    qcollectivepresence.h\
    qcollectivepresenceinfo.h\
    qcollectivenamespace.h\
    qcollectivemessenger.h\
    qcollectivesimplemessage.h 

SEMI_PRIVATE_HEADERS=\
    sippresencereader_p.h\
    sippresencewriter_p.h\
    dummypresenceservice_p.h\
    dummypresencecontrol_p.h

SOURCES=\
    qcollectivepresence.cpp\
    qcollectivepresenceinfo.cpp\
    qcollectivenamespace.cpp\
    qcollectivemessenger.cpp\
    qcollectivesimplemessage.cpp\
    sippresencereader.cpp\
    sippresencewriter.cpp\
    dummypresencecontrol.cpp\
    dummypresenceservice.cpp

