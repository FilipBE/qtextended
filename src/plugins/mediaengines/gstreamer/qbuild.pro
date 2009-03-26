TEMPLATE=plugin
TARGET=gstreamer

PLUGIN_FOR=qtopia
PLUGIN_TYPE=mediaengines

CONFIG+=qtopia singleexec
QTOPIA*=media video
MODULES*=gstreamer

pkg [
    name=gstreamer-mediaengine
    desc="GStreamer media engine plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    gstreamerengine.h\
    gstreamerengineinformation.h\
    gstreamerenginefactory.h\
    gstreamerurisessionbuilder.h\
    gstreamerplaybinsession.h\
    gstreamervideowidget.h\
    gstreamerqtopiavideosink.h\
    gstreamerbushelper.h\
    gstreamermessage.h\
    gstreamersinkwidget.h\
    gstreamerrtpsession.h\
    gstreamerqtopiacamerasource.h

SOURCES=\
    gstreamerengine.cpp\
    gstreamerengineinformation.cpp\
    gstreamerenginefactory.cpp\
    gstreamerurisessionbuilder.cpp\
    gstreamerplaybinsession.cpp\
    gstreamervideowidget.cpp\
    gstreamerqtopiavideosink.cpp\
    gstreamerbushelper.cpp\
    gstreamermessage.cpp\
    gstreamersinkwidget.cpp\
    gstreamerrtpsession.cpp\
    gstreamerqtopiacamerasource.cpp

settings [
    hint=image
    files=gstreamer.conf
    path=/etc/default/Trolltech
]

