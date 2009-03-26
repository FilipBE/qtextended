qtopia_project(stub)

CONFIG(release,release|debug):release=release
else:release=debug
unix:qdoc=$$fixpath(qdoc3)
else:qdoc=$$fixpath(qdoc3)
unix:quote=$$LITERAL_QUOTE
else:quote=$$LITERAL_ESCAPED_QUOTE

FEATURES=$$fixpath($$QTOPIA_DEPOT_PATH/src/qtopiadesktop/build/qt_patch)

MAKE_RULE=\
    $$ENV MAKEFILE=Makefile\
        QMAKEFEATURES=$$quote$$FEATURES$$quote\
        QMAKEPATH=$$quote$$QT_DEPOT_PATH$$quote\
            $$MAKE -C $$fixpath($$DQTDIR/tools/qdoc3)\
                MAKEFILE=Makefile
win32:MAKE_RULE+=\
                QMAKE_QMAKE=$$fixpath($$QTDIR/bin/qmake)

###
### NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
###
### These paths must match what is used in scripts/mkdocs
###
### NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE NOTE
###
MKDOCS_SOURCEDIR=$$fixpath($$PWD/src)
MKDOCS_DESTDIR=$$fixpath($$OUT_PWD/html)
MKDOCS_GENDIR=$$fixpath($$OUT_PWD/src/gen)
MKDOCS_SRC_BUILD=$$fixpath($$QTOPIA_DEPOT_PATH/src/qtopiadesktop/build)
MKDOCS_QDROOT=$$fixpath($$QTOPIA_DEPOT_PATH/src/qtopiadesktop)
MKDOCS_QDSYNC=$$fixpath($$QTOPIA_DEPOT_PATH/src/tools/qdsync)
MKDOCS_CONF=qtopiadesktop

mkdocs=$$fixpath($$PWD/mkdocs)
win32:mkdocs=$$fixpath($$QBS_BIN/runsyncqt) $$mkdocs

mkdocs.commands=$$COMMAND_HEADER\
    #$$MAKE_RULE $$LINE_SEP\
    echo Building docs... $$LINE_SEP\
    $$mkdocs $$MKDOCS_SOURCEDIR\
        $$MKDOCS_DESTDIR\
        $$MKDOCS_GENDIR\
        $$MKDOCS_SRC_BUILD\
        $$MKDOCS_QDROOT\
        $$MKDOCS_QDSYNC\
        $$MKDOCS_CONF\
        $$QPEDIR\
        $$QPEDIR\
        $$QTOPIA_DEPOT_PATH\
        $$DQTDIR\
        $$qdoc\
        $$fixpath($$PWD)
QMAKE_EXTRA_TARGETS+=mkdocs
ALL_DEPS+=mkdocs
win32:create_raw_dependency(mkdocs,FORCE)
else:create_raw_dependency(.PHONY,mkdocs)

win32:link_qd_docs.commands=$$COMMAND_HEADER\
    $$fixpath($$DQTDIR/bin/assistant) -addContentFile $$fixpath($$QPEDIR/doc/html/$$MKDOCS_CONF/$${MKDOCS_CONF}.dcf)
else:link_qd_docs.commands=$$COMMAND_HEADER\
    [ -f $$DQTDIR/bin/assistant ] && $$DQTDIR/bin/assistant -addContentFile $$fixpath($$QPEDIR/doc/html/$$MKDOCS_CONF/$${MKDOCS_CONF}.dcf); true
QMAKE_EXTRA_TARGETS+=link_qd_docs
link_qd_docs.depends+=mkdocs
ALL_DEPS+=link_qd_docs
depends(tools/qt/assistant,fake)

QMAKE_EXTRA_TARGETS+=redirect_install
