TEMPLATE=app
CONFIG+=qt
TARGET=qbuild
use_suffix:TARGET=$${TARGET}.bin
QT=core gui script
DEFINES+=QT_NO_DEBUG_STREAM
CONFIG+=release
QMAKE_CXXFLAGS+=-g -include qbuild_config.h
LIBS+=-lutil -rdynamic

# Input
HEADERS+=\
    lexer.h\
    preprocessor.h\
    parser.h\
    solution.h\
    object.h\
    project.h\
    functionprovider.h\
    builtinfunctions.h\
    qtscriptfunctions.h\
    options.h\
    ruleengine.h\
    qfastdir.h\
    process.h\
    qbuild.h\
    startup.h\
    gui.h\
    qoutput.h\

SOURCES+=\
    main.cpp\
    lexer.cpp\
    preprocessor.cpp\
    tokens.cpp\
    parser.cpp\
    solution.cpp\
    object.cpp\
    project.cpp\
    functionprovider.cpp\
    builtinfunctions.cpp\
    qtscriptfunctions.cpp\
    ruleengine.cpp\
    qfastdir.cpp\
    process.cpp\
    qbuild.cpp\
    startup.cpp\
    gui.cpp\

# This crap down here is because qmake doesn't understand -include.
# The point of this is to manually force every .o to depend on qbuild_config.h.

# Create a raw Makefile dependency
defineTest(create_raw_dependency) {
    var=$$1
    dep=$$2
    eval($${var}.depends*=\$$dep)
    export($${var}.depends)
    QMAKE_EXTRA_TARGETS*=$$var
    export(QMAKE_EXTRA_TARGETS)
}

# $$tail(val) returns the last part of a path (even if there's only one part)
defineReplace(tail) {
    path=$$1
    return($$basename(path))
}

for(s,SOURCES) {
    output=$$tail($$s)
    output~=s,.cpp$,.o,
    create_raw_dependency($$output,$$PWD/qbuild_config.h)
}

for(h,HEADERS) {
    output=moc_$$tail($$h)
    output~=s,.h$,.o,
    create_raw_dependency($$output,$$PWD/qbuild_config.h)
}

