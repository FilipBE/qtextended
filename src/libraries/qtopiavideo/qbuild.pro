TEMPLATE=lib
TARGET=qtopiavideo
MODULE_NAME=qtopiavideo

CONFIG+=qtopia hide_symbols singleexec

pkg [
    name=videolib
    desc="Video library for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    qcamera.h\
    qcameracontrol.h\
    qcameradevice.h\
    qcameradeviceplugin.h\
    qcameradeviceloader.h\
    qcameratools.h\
    qtopiavideo.h\
    qvideoframe.h\
    qimageplanetransform.h\
    qvideosurface.h

SOURCES=\
    qcameracontrol.cpp\
    qcameradeviceplugin.cpp\
    qcameradeviceloader.cpp\
    qcameratools.cpp\
    qtopiavideo.cpp\
    qvideoframe.cpp\
    qimageplanetransform.cpp

QWS [
    TYPE=CONDITIONAL_SOURCES
    CONDITION=!x11
    HEADERS=\
        qabstractvideooutput.h\
        qvideooutputloader.h\
        qvideooutputfactory.h
    SOURCES=\
        qabstractvideooutput.cpp\
        qvideooutputfactory.cpp\
        qvideooutputloader.cpp\
        qvideosurface.cpp
]

X11 [
    TYPE=CONDITIONAL_SOURCES
    CONDITION=x11
    HEADERS=\
        qgenericvideowidget.h
    SOURCES=\
        qvideosurface_x11.cpp\
        qgenericvideowidget.cpp
]

