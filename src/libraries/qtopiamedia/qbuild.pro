TEMPLATE=lib
TARGET=qtopiamedia
MODULE_NAME=qtopiamedia
VERSION=4.0.0

CONFIG+=qtopia hide_symbols singleexec
QTOPIA*=video

PRIVATE_HEADERS=\
    mediastyle_p.h\
    qmediacontentplayer_p.h\
    private/mediaserverproxy_p.h

SEMI_PRIVATE_HEADERS=\
    qmediahandle_p.h\
    activitymonitor_p.h\
    observer_p.h\
    qmediametainfocontrol_p.h

HEADERS=\
    media.h\
    qmediatools.h\
    qmediawidgets.h\
    qmediaabstractcontrol.h\
    qmediacontrol.h\
    qmediavideocontrol.h\
    qmediaseekcontrol.h\
    qmediacontent.h\
    qmediadevice.h\
    qmediaencoder.h\
    qmediadecoder.h\
    qmediacodecplugin.h\
    qmediartpsessionengine.h\
    qmediartpsession.h\
    qmarqueelabel.h\
    qmediaplaylist.h\
    qmedialist.h\
    qmediamenu.h\
    server/qmediaenginefactory.h\
    server/qmediaengine.h\
    server/qmediaengineinformation.h\
    server/qmediaserversession.h\
    server/qmediasessionbuilder.h\
    server/qmediasessionrequest.h\
    server/qmediaabstractcontrolserver.h\
    server/qmediavideocontrolserver.h\
    server/qmediaseekcontrolserver.h

SOURCES=\
    media.cpp\
    mediastyle.cpp\
    qmediatools.cpp\
    qmediawidgets.cpp\
    activitymonitor.cpp\
    qmediametainfocontrol.cpp\
    qmediaabstractcontrol.cpp\
    qmediacontrol.cpp\
    qmediavideocontrol.cpp\
    qmediaseekcontrol.cpp\
    qmediacontent.cpp\
    qmediacontentplayer.cpp\
    qmediacodecplugin.cpp\
    qmediadecoder.cpp\
    qmediaencoder.cpp\
    qmediartpsessionengine.cpp\
    qmediartpsession.cpp\
    qmarqueelabel.cpp\
    qmediaplaylist.cpp\
    qmedialist.cpp\
    qmediamenu.cpp\
    qmediahandle.cpp\
    private/mediaserverproxy.cpp\
    server/qmediaengine.cpp\
    server/qmediaengineinformation.cpp\
    server/qmediaserversession.cpp\
    server/qmediasessionbuilder.cpp\
    server/qmediasessionrequest.cpp\
    server/qmediaabstractcontrolserver.cpp\
    server/qmediavideocontrolserver.cpp\
    server/qmediaseekcontrolserver.cpp

HELIX [
    TYPE=CONDITIONAL_SOURCES
    CONDITION=contains(QTOPIAMEDIA_ENGINES,helix)
    HEADERS=qmediahelixsettingscontrol.h
    SOURCES=qmediahelixsettingscontrol.cpp
]

