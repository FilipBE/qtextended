qtopia_project(qtopiadesktop core lib)
TARGET=qtopiadesktop
CONFIG+=no_qd_prefix no_tr

QTOPIADESKTOP_HEADERS+=\
    center.h\
    desktopsettings.h\
    qdglobal.h\
    qdplugin.h\
    qdplugindefs.h\
    qlog.h\
    qtopiadesktoplog.h\

QTOPIADESKTOP_PRIVATE_HEADERS+=\
    qdplugin_p.h\

QTOPIADESKTOP_SOURCES+=\
    desktopsettings.cpp\
    qdplugin.cpp\
    qlog.cpp\
    qtopiadesktoplog.cpp\

# Files common to Qtopia Sync Agent and Synchronization
common=$$QTOPIA_DEPOT_PATH/src/tools/qdsync/common
VPATH+=$$common
INCLUDEPATH+=$$common
exists($$common/common.pri):include($$common/common.pri)
else:error(Missing $$common/common.pri)

PREFIX=QTOPIADESKTOP
resolve_include()

sdk_qtopiadesktop_headers.files=$$QTOPIADESKTOP_HEADERS
sdk_qtopiadesktop_headers.path=/include/qtopiadesktop/qtopiadesktop
sdk_qtopiadesktop_headers.hint=headers
INSTALLS+=sdk_qtopiadesktop_headers

sdk_qtopiadesktop_private_headers.files=$$QTOPIADESKTOP_PRIVATE_HEADERS
sdk_qtopiadesktop_private_headers.path=/include/qtopiadesktop/qtopiadesktop/private
sdk_qtopiadesktop_private_headers.hint=headers
INSTALLS+=sdk_qtopiadesktop_private_headers

idep(LIBS+=-l$$TARGET)
uses_export($$TARGET)
qt_inc($$TARGET)
