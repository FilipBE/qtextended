qtopia_project(stub)

# Depend on everything so we can guarantee that this directory is processed last
for(p,PROJECTS) {
    depends($$p,fake)
}

license.files=$$QTOPIA_DEPOT_PATH/src/qtopiadesktop/dist/LICENSE
license.path=/
INSTALLS+=license

win32:qtstuff.files=$$DQTDIR/bin/assistant.exe \
              #$$DQTDIR/lib/qaxcontainer.dll

unix:!mac:qtstuff.files=$$DQTDIR/bin/assistant
qtstuff.path=$$bindir
win32:INSTALLS+=qtstuff
unix:!max:INSTALLS+=qtstuff

msvcstuff.files=$$QTOPIA_DEPOT_PATH/src/qtopiadesktop/dist/*.dll
msvcstuff.path=$$libdir

commands=$$COMMAND_HEADER
for(lang,LANGUAGES) {
    !equals(commands,$$COMMAND_HEADER):commands+=$$LINE_SEP
    win32:commands+=\
        if not exist $$fixpath($(INSTALL_ROOT)$$resdir/i18n/$$lang)
    commands+=\
        $(MKDIR) $$fixpath($(INSTALL_ROOT)$$resdir/i18n/$$lang) $$LINE_SEP_VERBOSE\
        $(INSTALL_FILE) $$fixpath($$QTOPIA_DEPOT_PATH/i18n/$$lang/.directory)\
            $$fixpath($(INSTALL_ROOT)$$resdir/i18n/$$lang/.directory)
}
langfiles.commands=$$commands
langfiles.CONFIG=no_path
INSTALLS+=langfiles

mac {
    build_bundle.commands=$$COMMAND_HEADER\
        $$QPEDIR/src/qtopiadesktop/build/bin/build_macosx_bundle
    build_bundle.CONFIG=no_path
    # This must go last so depend on everything else that's already defined
    for(i,INSTALLS) {
        build_bundle.depends+=install_$$i
    }
    INSTALLS+=build_bundle
}

