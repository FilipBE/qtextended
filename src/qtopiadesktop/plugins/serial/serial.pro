qtopia_project(qtopiadesktop plugin)
TARGET=serial

SOURCES+=\
    seriallink.cpp\

win32 {
    HEADERS+=regthread.h
    SOURCES+=regthread.cpp
    depends(libraries/qdwin32)
}

