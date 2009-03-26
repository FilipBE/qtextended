!cross_compile {
    TRY_INCLUDEPATHS = /include /usr/include /usr/local/include $$QMAKE_INCDIR $$INCLUDEPATH
    for(p, TRY_INCLUDEPATHS) {
        pp = $$join(p, "", "", "/openssl")
        exists($$pp):INCLUDEPATH *= $$p
    }
}
