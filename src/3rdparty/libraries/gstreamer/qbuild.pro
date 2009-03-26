MODULE_NAME=gstreamer
requires(exists(/config.tests/mediaengines/gstreamer.pri))
include(/config.tests/mediaengines/gstreamer.pri)

DEP.headers [
    TYPE=DEPENDS PERSISTED SDK
    EVAL=\
        "MKSPEC.CFLAGS+="$$GSTREAMER_CFLAGS\
        "MKSPEC.CXXFLAGS+="$$GSTREAMER_CFLAGS
]

DEP.libs [
    TYPE=DEPENDS PERSISTED SDK
    EVAL=\
        "LIBS+="$$GSTREAMER_LIBS\
        "LIBS+=-lgstvideo-0.10"

]

