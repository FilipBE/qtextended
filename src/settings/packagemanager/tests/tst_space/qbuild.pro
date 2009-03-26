TEMPLATE=app
CONFIG+=qtopia unittest
TARGET=tst_space

SOURCEPATH+=/src/settings/packagemanager

MODULES*=tar

VPATH      +=$$QTOPIA_DEPOT_PATH/src/settings/packagemanager
INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/settings/packagemanager

# Input
FORMS += \
    packagedetails.ui \
    serveredit.ui

HEADERS += \
    domaininfo.h \
    httpfetcher.h \
    installedpackagescanner.h \
    md5file.h \
    packageinformationreader.h \
    packagemanagerservice.h \
    packagemodel.h \
    packageversion.h \
    packageview.h \
    sandboxinstall.h \
    serveredit.h \
    targz.h \
    installcontrol.h \
    packagecontroller.h \
    utils.h


SOURCES +=  \
    domaininfo.cpp \
    httpfetcher.cpp \
    installcontrol.cpp \
    installedpackagescanner.cpp \
    md5file.cpp \
    packagecontroller.cpp \
    packageinformationreader.cpp \
    packagemanagerservice.cpp \
    packagemodel.cpp \
    packageversion.cpp \
    packageview.cpp \
    sandboxinstall.cpp \
    serveredit.cpp \
    targz.cpp \
    tst_space.cpp \
    utils.cpp

DEFINES+=PACKAGEMANAGER_UTILS_TEST
