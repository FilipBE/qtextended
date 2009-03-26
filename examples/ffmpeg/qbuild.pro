requires(enable_ffmpeg)
TEMPLATE=plugin
CONFIG+=qtopia
TARGET=ffmpeg

PLUGIN_FOR=qtopia
PLUGIN_TYPE=mediaengines

QTOPIA*=media audio

HEADERS	= \
        ffmpegengine.h \
        ffmpegengineinformation.h \
        ffmpegenginefactory.h \
        ffmpegurisessionbuilder.h \
        ffmpegplaybinsession.h \
        ffmpegdirectpainterwidget.h \
        ffmpegsinkwidget.h 

SOURCES	= \
        ffmpegengine.cpp \
        ffmpegengineinformation.cpp \
        ffmpegenginefactory.cpp \
        ffmpegurisessionbuilder.cpp \
        ffmpegplaybinsession.cpp \
        ffmpegdirectpainterwidget.cpp \
        ffmpegsinkwidget.cpp 

LIBS += -lavcodec -lavformat

ffmpeg_settings.files=ffmpeg.conf
ffmpeg_settings.path=/etc/default/Trolltech
INSTALLS += ffmpeg_settings

AVAILABLE_LANGUAGES=en_US
LANGUAGES=$$AVAILABLE_LANGUAGES
STRING_LANGUAGE=en_US
AVAILABLE_LANGUAGES=$$QTOPIA_AVAILABLE_LANGUAGES
LANGUAGES=$$QTOPIA_LANGUAGES
