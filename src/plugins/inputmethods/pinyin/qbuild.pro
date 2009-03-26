TEMPLATE=plugin
TARGET=pinyinim

PLUGIN_FOR=qtopia
PLUGIN_TYPE=inputmethods

CONFIG+=qtopia singleexec
MODULES*=inputmatch

pkg [
    name=pinyin-inputmethod
    desc="Pinyin inputmethod plugin for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    pinyinim.h\
    pinyinimpl.h

SOURCES=\
    pinyinim.cpp\
    pinyinimpl.cpp

im [
    hint=image
    files=etc/im/pyim/*
    path=/etc/im/pyim
]

