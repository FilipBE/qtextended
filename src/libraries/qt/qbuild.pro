requires(!system_qt)
MODULE_NAME=qt
include(common.pri)

target_post [
    TYPE=RULE
    prerequisiteActions=sdk
]

configure [
    TYPE=RULE
    tests=\
        "$$not($$testFile(qbuild.configure_args,$$CONFIGURE_ARGS))"\
        # If a Makefile was removed, we need to re-run configure
        # Makefiles get removed when you abort a build while qmake is running
        "$$run_if_not_found($$[OUTPUT.1.ABS])"
    outputFiles=$$QTDIR/config.status makefiles
    commands=\
        "#(eh)echo $$shellQuote(Qt has changed. You need to run configure.)"\
        "$$error(Fatal error)"
]
add_input_files_to_rule(configure, $$path(/qtopiacore/qt/configure,existing))
add_input_files_to_rule(configure, $$path(/qtopiacore/qt/mkspecs,existing), "*.conf")
add_input_files_to_rule(configure, $$path(/qtopiacore/qt/mkspecs,existing), "*.pr[iof]")
add_input_files_to_rule(configure, $$path(/qtopiacore/qt/src,existing), "*.pr[iof]")
add_input_files_to_rule(configure, $$path(/qtopiacore/qt/tools,existing), "*.pr[iof]")
add_input_files_to_rule(configure, $$path(/qtopiacore/qt/qmake,existing), "*")
add_input_files_to_rule(configure, $$path(/qtopiacore/qt/config.tests,existing), "*")
add_input_files_to_rule(configure, $$path(/src/build/qt_patch,project), "*")
#print_rule(configure)

qt [
    TYPE=RULE
    help="Build Qt"
    inputFiles=makefiles
    outputFiles=$$qt_get_libs($$QTDIR) force_qt
    commands=\
        "#(eh)echo Checking if we need to build Qt..."\
        # Force hide all output (make -q doesn't actually suppress output?)
        "#(se)$$QT_PATCH_ENV $$MAKE -C $$QTDIR -q >/dev/null 2>&1; test $? -ne 0"\
        # Fix up the Qt Makefiles so they don't spew out "cd foo && make" lines
        "#(E)$$QTOPIACORE/fixmakefiles $$shellQuote($$QTDIR)"\
        "#(eh)echo $$MAKE -C $$QTDIR"\
        "#(e)$$QT_PATCH_ENV $$MAKE -C $$QTDIR"
]

qvfb [
    TYPE=RULE
    help="Build QVFb"
    inputFiles=makefiles
    prerequisiteActions=qt
    commands=\
        "#(eh)echo Checking if we need to build QVFb..."\
        # Force hide all output (make -q doesn't actually suppress output?)
        "#(se)$$QT_PATCH_ENV $$MAKE -C $$QTDIR/tools/qvfb -q >/dev/null 2>&1; test $? -ne 0"\
        "#(eh)echo $$MAKE -C $$QTDIR/tools/qvfb"\
        "#(e)$$QT_PATCH_ENV $$MAKE -C $$QTDIR/tools/qvfb"
]

QTLIBS=$$qt_get_libs($$QTDIR,$$SDKROOT/qtopiacore/host)

sdk [
    TYPE=RULE
    help="Install Qt into the SDK"
    prerequisiteActions=qt
    outputFiles=sdkdone $$QTLIBS
    commands=\
        "#(eh)echo $$MAKE -C $$QTDIR/src install"\
        "#(e)rm -rf $$SDKROOT/qtopiacore/host"\
        "#(e)$$QT_PATCH_ENV $$MAKE -C $$QTDIR install"\
        # Put the doc symlink here so that assistant works
        "#(e)$$MKSPEC.DEL_FILE $$SDKROOT/qtopiacore/host/doc"\
        "#(e)$$MKSPEC.SYMBOLIC_LINK $$path(/qtopiacore/qt/doc,existing) $$SDKROOT/qtopiacore/host/doc"
]

enable_qvfb {
    sdk.prerequisiteActions+="#(h)qvfb"
    sdk.commands+=\
        "#(eh)echo $$MAKE -C $$QTDIR/tools/qvfb install"\
        "#(e)$$QT_PATCH_ENV $$MAKE -C $$QTDIR/tools/qvfb install"
}

sdk.commands+=\
    # We need to adjust the time of the files in the SDK or we will cause unnecessary rebuilding
    "#(e)$$QTOPIACORE/fixsdktimestamps $$shellQuote($$QTDIR) $$shellQuote($$SDKROOT/qtopiacore/host)"\
    "#(e):> $$[OUTPUT.0]"

clean [
    TYPE=RULE
    help="Clean Qtopia Core"
    commands=\
        "#(en)rm qbuild.configure_args"\
        "#(eh)echo $$MAKE clean"\
        "#(e)$$QT_PATCH_ENV $$MAKE -C $$QTDIR/src clean"
]
enable_qvfb:clean.commands+=\
        "#(e)$$QT_PATCH_ENV $$MAKE -C $$QTDIR/tools/qvfb clean"

# Non-persisted dependency used when building
depends [
    TYPE=DEPENDS
    EVAL=\
        "build_moc_rule.TYPE=RULE"\
        "build_moc_rule.prerequisiteActions+=""#(h)"$$project()"/target_post"""\
        "build_uic_rule.TYPE=RULE"\
        "build_uic_rule.prerequisiteActions+=""#(h)"$$project()"/target_post"""\
        "build_rcc_rule.TYPE=RULE"\
        "build_rcc_rule.prerequisiteActions+=""#(h)"$$project()"/target_post"""\
        "compiler_source_depends.TYPE=RULE"\
        "compiler_source_depends.prerequisiteActions+=""#(h)"$$project()"/target_post"""
]

# This matches the logic in qbuild/extensions/qt.js
defs [
    TYPE=DEPENDS PERSISTED SDK
    EVAL=\
        "QTDIR="$$SDKROOT/qtopiacore/host\
        "QTINC=$$QTDIR/include"\
        "INCLUDEPATH+=$$QTINC"\
        "QTLIBS=$$QTDIR/lib"\
        "QTDATA=$$QTDIR"
]

pdefs [
    TYPE=DEPENDS PERSISTED
    EVAL=\
        "QTLIBS_P="$$QTDIR"/lib"\
        "QTDATA_P="$$QTDIR
]

