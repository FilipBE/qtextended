requires(!enable_singleexec)
TEMPLATE=lib
TARGET=spygrind

CONFIG+=qtopia

pkg [
    name=spygrind
    desc="Spygrind library for Qt Extended."
    version=$$QTOPIA_VERSION
    license=$$QTOPIA_LICENSE
    maintainer=$$QTOPIA_MAINTAINER
]

HEADERS=\
    method_p.h\
    qsignalspycallback_p.h\
    qsignalspycollector.h\
    stamp_p.h

SOURCES=\
    hook.cpp\
    method.cpp\
    qsignalspycollector.cpp

malloc [
    TYPE=CONDITIONAL_SOURCES
    CONDITION=enable_malloc_hook
    SOURCES=mallochook.cpp
]
enable_malloc_hook:DEFINES+=SPYGRIND_MALLOC_HOOK

unix [
    TYPE=CONDITIONAL_SOURCES
    CONDITION=unix
    HEADERS=qunixsignalnotifier_p.h
    SOURCES=qunixsignalnotifier.cpp
]

