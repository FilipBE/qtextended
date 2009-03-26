requires(!system_qt)
include(../common.pri)

configure [
    TYPE=RULE
    help="Configure Qt"
    tests=\
        "$$not($$testFile(../qbuild.configure_args,$$CONFIGURE_ARGS))"\
        # If a Makefile was removed, we need to re-run configure
        # Makefiles get removed when you abort a build while qmake is running
        "$$run_if_not_found($$[OUTPUT.1.ABS])"
    outputFiles=$$QTDIR/config.status ../makefiles
    commands=\
        "#(e)[ ! -f $$[OUTPUT.1] ] || rm $$[OUTPUT.1]"\
        "#(eh)echo configure $$CONFIGURE_ARGS"\
        "#(e)mkdir -p $$QTDIR; cd $$QTDIR; rm -rf config.tests;"\
        "#(e)[ -f $$QT_DEPOT/bin/syncqt ] || mv $$QT_DEPOT/bin/syncqt.disabled $$QT_DEPOT/bin/syncqt"\ 
        "#(ve)find "$$QTDIR" \( -name Makefile -o -name Makefile.* \) -exec rm '{}' \;"
]
free_package:configure.commands+=\
        "#(e)if [ -f $$QT_DEPOT/src/gui/kernel/qapplication_qws.cpp ]; then mv $$QT_DEPOT/src/gui/kernel/qapplication_qws.cpp $$QT_DEPOT/src/gui/kernel/qapplication_qws.cpp.disabled; fi"

configure.commands+=\
        "#(e)cd $$QTDIR; $$QT_PATCH_ENV $$CONFIGURE $$CONFIGURE_ARGS"\
        "#(e)[ -f $$QT_DEPOT/src/gui/kernel/qapplication_qws.cpp ] || mv $$QT_DEPOT/src/gui/kernel/qapplication_qws.cpp.disabled $$QT_DEPOT/src/gui/kernel/qapplication_qws.cpp"\
        # List the Makefiles so that we can re-run when any gets removed
        "#(ve)echo "foo:" >../makefiles; find "$$QTDIR" -name Makefile -printf '\t%p\n' >>../makefiles"\
        "#(e)$$writeFile(../qbuild.configure_args,$$CONFIGURE_ARGS)"

add_input_files_to_rule(configure, $$path(/qtopiacore/qt/configure,existing))
add_input_files_to_rule(configure, $$path(/qtopiacore/qt/mkspecs,existing), "*.conf")
add_input_files_to_rule(configure, $$path(/qtopiacore/qt/mkspecs,existing), "*.pr[iof]")
add_input_files_to_rule(configure, $$path(/qtopiacore/qt/src,existing), "*.pr[iof]")
add_input_files_to_rule(configure, $$path(/qtopiacore/qt/tools,existing), "*.pr[iof]")
add_input_files_to_rule(configure, $$path(/qtopiacore/qt/qmake,existing), "*")
add_input_files_to_rule(configure, $$path(/qtopiacore/qt/config.tests,existing), "*")
add_input_files_to_rule(configure, $$path(/src/build/qt_patch,project), "*")
#print_rule(configure)

