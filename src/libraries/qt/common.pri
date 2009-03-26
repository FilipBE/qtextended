QT_DEPOT=$$path(/qtopiacore/qt,existing)
FEATURES=
!isEmpty(DEVICE_CONFIG_PATH):exists($$DEVICE_CONFIG_PATH/features/qt_patch):FEATURES=$$DEVICE_CONFIG_PATH/features/qt_patch:
FEATURES=$$FEATURES$$path(/src/build/qt_patch,project)
QT_PATCH_ENV=\
    MAKEFILE=Makefile\
    QMAKEFEATURES=$$shellQuote($$FEATURES)\
    QMAKEPATH=$$shellQuote($$QT_DEPOT)\
    QPE_CONFIG=$$shellQuote($$CONFIG)\
    QTOPIACORE_CONFIG=$$shellQuote($$QTOPIACORE_CONFIG)\
    QPEDIR=$$shellQuote($$QPEDIR)\
    SDKROOT=$$shellQuote($$SDKROOT)\
    ADD_CFLAGS=$$shellQuote($$QTOPIA_ADD_CFLAGS)
QTDIR=$$path(/qtopiacore/host,generated)
QTOPIACORE=$$path(../qtopiacore,project)
CONFIGURE_ARGS=$$DQT_CONFIG
CONFIGURE=$$QT_DEPOT/configure

