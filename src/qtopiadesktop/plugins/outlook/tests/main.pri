script=$$fixpath($$PWD/main.pl)
win32:script=$$fixpath($$QBS_BIN/runsyncqt) $$script
isEmpty(QMAKE_MOC) {
    win32:QMAKE_MOC = $$[QT_INSTALL_BINS]\moc.exe
    else:QMAKE_MOC = $$[QT_INSTALL_BINS]/moc
}
main.commands=\
    $$script ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT} $$COMMON_TESTS $$LINE_SEP_VERBOSE\
    $$QMAKE_MOC -o $$fixpath($$MOC_DIR/main.moc) ${QMAKE_FILE_OUT}
main.output=$$OUT_PWD/${QMAKE_FILE_BASE}
main.input=MAIN_SOURCES
main.variable_out=SOURCES
main.name=generate ${QMAKE_FILE_IN}
tmp=$$HEADERS $$SOURCES
for(t,tmp) {
    found=0
    for(v,VPATH) {
        exists($$v/$$t) {
            main.depends+=$$v/$$t
            found=1
            break()
        }
    }
}
main.depends+=$$COMMON_TESTS
win32:main.depends+=../common.h

QMAKE_EXTRA_COMPILERS+=main

INCLUDEPATH+=$$QTOPIA_DEPOT_PATH/src/qtopiadesktop/tests/shared

