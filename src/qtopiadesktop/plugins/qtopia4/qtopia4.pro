qtopia_project(qtopiadesktop plugin)
TARGET=qtopia4

SOURCES+=\
    qtopia4device.cpp\
    qtopia4sync.cpp

qtopia_depot:!win32 {
    SOURCES+=storagestub.cpp
    RESOURCES+=stubdata.qrc
}

