requires(contains(QTOPIAMEDIA_ENGINES,helix))
TEMPLATE=plugin
TARGET=helix

PLUGIN_FOR=qtopia
PLUGIN_TYPE=mediaengines

CONFIG+=qtopia singleexec
QTOPIA*=media video
MODULES*=helix

CONFIG-=warn_on
CONFIG+=warn_off

pkg [
    name=helix-mediaengine
    desc="Helix media engine plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    helixenginefactory.h\
    helixengine.h\
    helixplayer.h\
    helixsession.h\
    helixsite.h\
    helixutil.h\
    helixvideosurface.h\
    helixvideowidget.h\
    qmediahelixsettingsserver.h\
    reporterror.h\
    interfaces.h\
    observer.h\
    config.h

SOURCES=\
    helixenginefactory.cpp\
    helixengine.cpp\
    helixplayer.cpp\
    helixsession.cpp\
    helixsite.cpp\
    helixutil.cpp\
    helixvideosurface.cpp\
    helixvideowidget.cpp\
    qmediahelixsettingsserver.cpp\
    reporterror.cpp\
    iids.cpp

# We need some stuff from Helix itself (passed in via the module.dep file)
depends_load_dependencies()
SOURCEPATH+=\
    $$HELIX_PATH/common/util\
    $$HELIX_PATH/video/vidutil
SOURCES+=\
    HXErrorCodeStrings.c\
    colormap.c
SOURCE_DEPENDS_RULES+=$$HELIX_BUILD_RULE

dbg=$$HELIX_OUT_DIR
DEFINES+=CONFIG_H_FILE=$$define_string($$HELIX_PATH/$$dbg/makefile_ribodefs.h)

settings [
    hint=image
    files=helix.conf
    path=/etc/default/Trolltech
]

