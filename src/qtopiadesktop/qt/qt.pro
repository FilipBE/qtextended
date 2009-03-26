qtopia_project(stub)

#for(lib,QT) {
#    lib=$$resolveqt($$lib)
#    var=$$lower($$lib)
#    unix:!mac {
#        libdqt=lib$$lib
#        CONFIG(debug,debug|release):libdqt=$${libdqt}_debug
#        commands=\
#            install -c $$LITERAL_QUOTE$$DQTDIR/lib/$${libdqt}.so.$$DQT_VERSION$$LITERAL_QUOTE $$LITERAL_QUOTE$(INSTALL_ROOT)$$libdir/$${libdqt}.so.$$DQT_VERSION$$LITERAL_QUOTE $$LINE_SEP_VERBOSE\
#            ln -sf $${libdqt}.so.$${DQT_VERSION} $$LITERAL_QUOTE$(INSTALL_ROOT)$$libdir/$${libdqt}.so$$LITERAL_QUOTE $$LINE_SEP_VERBOSE\
#            ln -sf $${libdqt}.so.$${DQT_VERSION} $$LITERAL_QUOTE$(INSTALL_ROOT)$$libdir/$${libdqt}.so.$$DQT_MAJOR_VERSION$$LITERAL_QUOTE $$LINE_SEP_VERBOSE\
#            ln -sf $${libdqt}.so.$${DQT_VERSION} $$LITERAL_QUOTE$(INSTALL_ROOT)$$libdir/$${libdqt}.so.$${DQT_MAJOR_VERSION}.$$DQT_MINOR_VERSION$$LITERAL_QUOTE
#        eval($${var}.commands=\$$commands)
#    } else:mac {
#        libdqt=lib$$lib
#        CONFIG(debug,debug|release):libdqt=$${libdqt}_debug
#        commands=\
#            install -c $$LITERAL_QUOTE$$DQTDIR/lib/$${libdqt}.$${DQT_VERSION}.dylib$$LITERAL_QUOTE $$LITERAL_QUOTE$(INSTALL_ROOT)$$libdir/$${libdqt}.$${DQT_VERSION}.dylib$$LITERAL_QUOTE $$LINE_SEP_VERBOSE\
#            ln -sf $${libdqt}.$${DQT_VERSION}.dylib $$LITERAL_QUOTE$(INSTALL_ROOT)$$libdir/$${libdqt}.dylib$$LITERAL_QUOTE $$LINE_SEP_VERBOSE\
#            ln -sf $${libdqt}.$${DQT_VERSION}.dylib $$LITERAL_QUOTE$(INSTALL_ROOT)$$libdir/$${libdqt}.$${DQT_MAJOR_VERSION}.dylib$$LITERAL_QUOTE $$LINE_SEP_VERBOSE\
#            ln -sf $${libdqt}.$${DQT_VERSION}.dylib $$LITERAL_QUOTE$(INSTALL_ROOT)$$libdir/$${libdqt}.$${DQT_MAJOR_VERSION}.$${DQT_MINOR_VERSION}.dylib$$LITERAL_QUOTE
#        eval($${var}.commands=\$$commands)
#    } else:win32 {
#        libdqt=$$lib
#        CONFIG(debug,debug|release):libdqt=$${libdqt}d
#        files=$$DQTDIR/lib/$$libdqt$${DQT_MAJOR_VERSION}.dll
#        eval($${var}.files=\$$files)
#    }
#    eval($${var}.path=\$$libdir)
#    INSTALLS+=$$var
#}
#qt_plugins.path=$$resdir/qt_plugins
## On the Mac, the PREFIX is *forced* to $bundle/Contents so we can't put this in the resources dir
#mac:qt_plugins.path=/qt_plugins
#qt_plugins.commands=$(COPY_DIR) $$fixpath($$DQTDIR/plugins/*) $$fixpath($(INSTALL_ROOT)$$qt_plugins.path)
#INSTALLS+=qt_plugins

# qt.conf is dead. Patch libQtCore instead.
unix:!mac {
    libdqt=libQtCore
    #CONFIG(debug,debug|release):libdqt=$${libdqt}_debug
    libdqt=$${libdqt}.so.$$DQT_VERSION
    prefix=$(INSTALL_ROOT)
} else:mac {
    libdqt=libQtCore
    #CONFIG(debug,debug|release):libdqt=$${libdqt}_debug
    libdqt=$${libdqt}.$${DQT_VERSION}.dylib
    # On the Mac, the PREFIX is *forced* to $bundle/Contents so it doesn't matter what we write into libQtCore
    prefix=notused
} else:win32 {
    libdqt=QtCore
    CONFIG(debug,debug|release):libdqt=$${libdqt}d
    libdqt=$${libdqt}$${DQT_MAJOR_VERSION}.dll
    prefix=$(INSTALL_ROOT)
}
ipatchqt.commands=$$COMMAND_HEADER\
    $$fixpath($$QBS_BIN/patchqt) $$fixpath($(INSTALL_ROOT)$$libdir/$$libdqt) $$prefix
ipatchqt.CONFIG=no_path
#ipatchqt.depends=install_qtcore
INSTALLS+=ipatchqt
# "make patchqt" is a convenience function that calls "make install_ipatchqt"
patchqt.depends+=install_ipatchqt
QMAKE_EXTRA_TARGETS+=patchqt

qt_ts.file=qt___LANG__
qt_ts.outfile=qt
#qt_ts.source=$$QT_DEPOT_PATH/translations
# The Qtopia translations of Qt are more complete so use them instead
qt_ts.source=$$QTOPIA_DEPOT_PATH/src/qt
qt_ts.hint=extra_ts
INSTALLS+=qt_ts

