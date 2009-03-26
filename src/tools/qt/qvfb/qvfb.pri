SOURCEPATH=\
    /qtopiacore/qt/tools/qvfb\
    /qtopiacore/qt/src/gui/embedded\
    /qtopiacore/qt/tools/shared/deviceskin
QT_SOURCE_TREE=/qtopiacore/qt
PWD=/qtopiacore/qt/tools/shared/deviceskin
CONFIG+=unix x11
include(/qtopiacore/qt/tools/qvfb/qvfb.pro)
CONFIG-=qt
#fetchvarsfromfile("/qtopiacore/qt/tools/shared/deviceskin/deviceskin.pri");
