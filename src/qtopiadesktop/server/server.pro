qtopia_project(qtopiadesktop app)
TARGET=qtopiasyncagent
TRTARGET=server

DEFINES+=VERSION="$$define_string($$QPE_VERSION)"
!isEmpty(VENDOR):DEFINES+=VENDOR="$$define_string($$VENDOR)"

HEADERS+=\
    qtopiadesktopapplication.h\
    mainwindow.h\
    connectionstatuswidget.h\
    statusbar.h\
    desktopwrapper.h\
    logwindow.h\
    settingsdialog.h\
    pluginchooser.h\
    pluginmanager.h\
    qpluginmanager.h\
    qtopiaresource_p.h\
    connectionmanager.h\
    syncmanager.h\

SOURCES+=\
    main.cpp\
    qtopiadesktopapplication.cpp\
    mainwindow.cpp\
    connectionstatuswidget.cpp\
    statusbar.cpp\
    desktopwrapper.cpp\
    logwindow.cpp\
    settingsdialog.cpp\
    pluginchooser.cpp\
    pluginmanager.cpp\
    qpluginmanager.cpp\
    qtopiaresource.cpp\
    connectionmanager.cpp\
    syncmanager.cpp\

# Plugins
SOURCES+=\
    infopage.cpp\
    pluginspage.cpp\
    qlogpage.cpp\
    qcopconnection.cpp\

# Always rebuild infopage.o so that the build date is correct
win32:create_raw_dependency($$fixpath($$OBJECTS_DIR/infopage.obj),FORCE)
else:create_raw_dependency($$fixpath($$OBJECTS_DIR/infopage.o),FORCE)

# Test page to do all sorts of debugging things
qtopia_depot:SOURCES+=testpage.cpp

# QtSingleApplication
HEADERS+=qtsingleapplication.h
SOURCES+=qtsingleapplication.cpp
win32:SOURCES+=qtsingleapplication_win.cpp
mac:SOURCES+=qtsingleapplication_mac.cpp
unix:!mac:SOURCES+=qtsingleapplication_x11.cpp

# TrayIcon
HEADERS+=trayicon.h
SOURCES+=trayicon.cpp

# The sync code doesn't work with Qt 4.2
equals(DQT_MINOR_VERSION,2):CONFIG+=no_sync
no_sync {
    DEFINES+=NO_SYNC
} else {
    QT+=sql
    HEADERS+=\
        merge.h\
        mergeitem.h\
        qsyncprotocol.h\

    SOURCES+=\
        merge.cpp\
        mergeitem.cpp\
        qsyncprotocol.cpp\

}

# These icons should be based on pics/appicon.png
win32:RC_FILE=server.rc
mac:RC_FILE=appicon.icns

qdpics.files=pics/*
qdpics.path=$$resdir/pics
qdpics.hint=pics
INSTALLS+=qdpics

win32:LIBS+=-luser32 -lgdi32 -lshell32
# Don't let qmake install it
mac:CONFIG+=no_target

