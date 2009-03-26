UNIFIED_NCT_LUPDATE=1

SOURCES+=\
    main.cpp

target [
    hint=sxe
    domain=qpe
]

servicedefs [
    hint=image
    files=$$SERVER_PWD/services/*.service
    path=/services
]

usbservicedefs [
    hint=image
    files=$$SERVER_PWD/services/UsbGadget/*.service
    path=/services/UsbGadget
]

help [
    hint=help
    source=$$SERVER_PWD/help
    files=*.html
]

defaultalerts [
    hint=image content nct
    files=$$SERVER_PWD/ringtones/*.wav
    path=/etc/SystemRingTones
    categories=SystemRingtones
    trtarget=QtopiaRingTones
]

qpepics [
    hint=pics
    files=$$SERVER_PWD/pics/qpe/*
    path=/pics/qpe
]

phonepics [
    hint=pics
    files=$$SERVER_PWD/pics/qpephone/*
    path=/pics/qpe/phone
]

equals(QTOPIA_UI,home) {
    deskphonepics [
        hint=pics
        files=$$SERVER_PWD/pics/qpedeskphone/*
        path=/pics/qpe/deskphone
    ]
}

splash [
    hint=image
    files=$$SERVER_PWD/pics/qpesplash/$${QTOPIA_DISP_WIDTH}x$${QTOPIA_DISP_HEIGHT}/splash.gif
    path=/pics/qpe
]
!exists($$splash.files):splash.files=$$SERVER_PWD/pics/qpesplash/splash.png

enable_sxe {
    security.hint=image
    security.commands=\
        "mkdir -p $$QTOPIA_IMAGE/etc/rekey $$QTOPIA_IMAGE/etc/sxe_qtopia $$QTOPIA_IMAGE/etc/sxe_domains"\
        "install -m 0600 "$$path($$SERVER_PWD/etc,project)"/sxe.* $$QTOPIA_IMAGE/etc"\
        "install -m 0500 "$$path($$SERVER_PWD/etc/sxe_domains,project)"/* $$QTOPIA_IMAGE/etc/sxe_domains"
    SXE_SCRIPTS=sxe_qtopia sxe_sandbox sxe_unsandbox sxe_reloadconf
    for(file,SXE_SCRIPTS) {
        security.commands+=\
            "install -m 0500 "$$path($$SERVER_PWD/etc/sxe_qtopia/$$file,existing)" $$QTOPIA_IMAGE/etc/sxe_qtopia"
    }
    path=$$path($$SERVER_PWD/etc/sxe_domains,existing)
    !equals(path,$$path($$SERVER_PWD/etc/sxe_domains,project)) {
        security.commands+=\
            "install -m 0500 "$$path($$SERVER_PWD/etc/sxe_domains,existing)"/* $$QTOPIA_IMAGE/etc/sxe_domains"
    }

    security_settings [
        hint=image
        files=$$SERVER_PWD/etc/default/Trolltech/Sxe.conf
        path=/etc/default/Trolltech
    ]
}

# Install dictionaries
dicts.hint=image
qdawggen=$$path(QtopiaSdk:/bin/qdawggen,generated)
!equals(QTOPIA_HOST_ENDIAN,$$QTOPIA_TARGET_ENDIAN):qdawggen+=-e
commands=$$COMMAND_HEADER
for(lang,LANGUAGES) {
    # `words' must exist... any other files are done too
    exists($$SERVER_PWD/etc/dict/$$lang/words) {
        dicts.commands+=\
            "#(e)$$MKSPEC.MKDIR $$QTOPIA_IMAGE/etc/dict/"$$lang\
            "#(e)find "$$path($$SERVER_PWD/etc/dict/$$lang,project)" -maxdepth 1 -type f | xargs -r -t $$qdawggen $$QTOPIA_IMAGE/etc/dict/"$$lang
    }
}
dicts.depends=\
    "#(oh)/src/tools/qdawggen/check_enabled"\
    "#(oh)/src/tools/qdawggen/target"

