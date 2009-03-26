# The actual helix build tree is located somewhere else so redirect the commands
HELIX_PATH=$$path(../helixbuild,generated)
SRCDIR=$$path(..,project)

# setup the helix bulid tree
setup_helixbuild [
    TYPE=RULE
    outputFiles=\
        ../setup_helixbuild\
        $$path(../helixbuild,generated)
    commands="#(ve)[ ""$VERBOSE_SHELL"" = 1 ] && set -x
        HELIX_PATH="""$$HELIX_PATH"""
        SRCDIR="""$$SRCDIR"""
        HELIX_CONFIG="""$$HELIX_CONFIG"""
        QPEDIR="""$$QPEDIR"""
        QTEDIR="""$$QTDIR"""
        RPATHVALUE="""$$RPATHVALUE"""
        LOGGING="""$$LOGGING"""
        set -e
        echo ""Creating the helixbuild directory...""
        if [ -d $HELIX_PATH ]; then rm -rf $HELIX_PATH; fi
        mkdir -p $HELIX_PATH
        cp -aRpf $SRCDIR/src/* $HELIX_PATH
        cp -aRpf $SRCDIR/trolltech/src/* $HELIX_PATH
        chmod -R u+w $HELIX_PATH
        chmod +x $HELIX_PATH/build/bin/*
        cd $HELIX_PATH
        if [ -d $SRCDIR/trolltech/patches ]; then
            for patch in $SRCDIR/trolltech/patches/*.patch; do
                patch -g0 -t -p2 <$patch
            done
        fi
        find $HELIX_PATH -type d -name CVS -prune -print | xargs -r rm -rf
        "\
        "#(e)touch $$[OUTPUT.0]"
]
add_input_files_to_rule(setup_helixbuild,$$SRCDIR/src,*)
add_input_files_to_rule(setup_helixbuild,$$SRCDIR/trolltech,*)
add_input_files_to_rule(setup_helixbuild,$$SRCDIR,qbuild.pro)

