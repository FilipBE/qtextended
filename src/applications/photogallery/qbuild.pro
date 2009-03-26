TEMPLATE=app
CONFIG+=qtopia
TARGET=photogallery

CONFIG+=quicklaunch singleexec

MODULES*=homeui
MODULES*=pictureflow

STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
UNIFIED_NCT_LUPDATE=1

pkg [
    name=photos
    desc="Photos application for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    albummodel.h\
    albumribbon.h\
    albumselector.h\
    durationslider.h\
    imageselector.h\
    imageview.h\
    imageloader.h\
    photogallery.h\
    qsmoothiconview.h\
    slideshowview.h\
    smoothimagemover.h\
    titlewindow.h\
    thumbcache.h\
    thumbmodel.h\
    zoomslider.h

SOURCES=\
    albummodel.cpp\
    albumribbon.cpp\
    albumselector.cpp\
    durationslider.cpp\
    imageselector.cpp\
    imageview.cpp\
    imageloader.cpp\
    photogallery.cpp\
    qsmoothiconview.cpp\
    slideshowview.cpp\
    smoothimagemover.cpp\
    titlewindow.cpp\
    thumbcache.cpp\
    thumbmodel.cpp\
    zoomslider.cpp\
    main.cpp

# Install rules

target [
    hint=sxe
    domain=trusted
]

desktop [
    hint=desktop
    files=photogallery.desktop
    path=/apps/Applications
]

service [
    hint=image
    files=services/PhotoGallery/photogallery
    path=/services/PhotoGallery
]

# FIXME this should not be accessing another app's icons
pics [
    hint=pics
    files=pics/*
    path=/pics/photogallery
]

