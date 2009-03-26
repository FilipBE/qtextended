MODULE_NAME=helix

# This is a stub project (external build system) but it depends on libqtopia
CONFIG+=qtopia
TEMPLATE=blank
MODULES*=qtopia

target_post [
    TYPE=RULE
    prerequisiteActions=build_helix
]

# The actual helix build tree is located somewhere else so redirect the commands
HELIX_PATH=$$path(helixbuild,generated)

release {
    release=-t release
    LOGGING=no
    BUILDPART=splay_nodist_qtopia 
} else {
    release=-t debug
    LOGGING=yes
    BUILDPART=splay_nodist_qtopia_log
}

enable_rpath:!isEmpty(MKSPEC.RPATH):RPATHVALUE=$$MKSPEC.RPATH$$QTOPIA_PREFIX

qt_get_qtdir()
isEmpty(QTDIR):loud_disable_project("Can't get QTDIR!")
# We don't actually want these to do anything!
CONFIG-=qt qtopia

# setup the helix bulid tree
setup_helixbuild [
    TYPE=RULE
    inputFiles=\
        $$HELIX_PATH/common/util/HXErrorCodeStrings.c\
        $$HELIX_PATH/video/vidutil/colormap.c
    outputFiles=\
        setup_helixbuild
    commands=\
        "#(eh)echo $$shellQuote(Helix has changed. You need to run configure.)"\
        "$$error(Fatal error)"
]
add_input_files_to_rule(setup_helixbuild,$$path(.,project)/src,*)
add_input_files_to_rule(setup_helixbuild,$$path(.,project)/trolltech,*)

# configure helix
helixconf [
    TYPE=RULE
    inputFiles=setup_helixbuild
    outputFiles=helixconf
    commands="#(ve)[ ""$VERBOSE_SHELL"" = 1 ] && set -x
        HELIX_PATH="""$$HELIX_PATH"""
        SRCDIR="""$$path(.,project)"""
        HELIX_CONFIG="""$$HELIX_CONFIG"""
        SDKROOT="""$$SDKROOT"""
        QTEDIR="""$$SDKROOT/qtopiacore/target"""
        RPATHVALUE="""$$RPATHVALUE"""
        LOGGING="""$$LOGGING"""
        set -e
        $SRCDIR/helixconf ""$HELIX_CONFIG""\
            $HELIX_PATH/build/umakepf/helix-client-qtopia-nodist.pf\
            ""$QTEDIR""\
            ""$SDKROOT""\
            ""$RPATHVALUE""\
            ""$HELIX_PATH/buildrc""\
            ""$LOGGING""
        "\
        "#(e)touch $$[OUTPUT.0]"
]

# Run the Helix build system
build_helix [
    TYPE=RULE
    inputFiles=helixconf
    outputFiles=build_helix
    prerequisiteActions=target_pre
    commands="#(ve)[ ""$VERBOSE_SHELL"" = 1 ] && set -x
        HELIX_PATH="""$$HELIX_PATH"""
        HELIX_SYSTEM_ID="""$$HELIX_SYSTEM_ID"""
        release="""$$release"""
        BUILDPART="""$$BUILDPART"""
        build_helix="""$$path(build_helix,generated)"""
        [ -f ""$build_helix"" ] && rm ""$build_helix""
        cd $HELIX_PATH
        export BUILD_ROOT=""$HELIX_PATH/build""
        export PATH=""$PATH:$BUILD_ROOT/bin""
        export BUILDRC=""$HELIX_PATH/buildrc""
        export SYSTEM_ID=""$HELIX_SYSTEM_ID""
        echo ""Building Helix... (See $HELIX_PATH/build.log for details)""
        python build/bin/build.py -v -p green $release -P helix-client-qtopia-nodist $BUILDPART >build.log 2>&1
        if [ $? -ne 0 ]; then
            echo ""There was a problem building Helix.""
            cat $HELIX_PATH/build.log
            exit 1
        fi
        "\
        "#(e)touch $$[OUTPUT.0]"
]

clean [
    TYPE=RULE
    commands="rm -rf "$$HELIX_PATH" build_helix helixconf setup_helixbuild"
]

release:debug=release
else:debug=debug

# install some libs and binaries at "make install" time
install_libs [
    TYPE=RULE
    inputFiles=build_helix
    commands="#(ve)[ ""$VERBOSE_SHELL"" = 1 ] && set -x
        HELIX_PATH="""$$HELIX_PATH"""
        debug="""$$debug"""
        STRIP="""$$MKSPEC.STRIP"""
        STRIPFLAGS="""$$MKSPEC.STRIPFLAGS_LIB"""
        IMAGE="""$$QTOPIA_IMAGE"""
        INSTALL_FILE="""$$MKSPEC.INSTALL_FILE"""
        INSTALL_PROGRAM="""$$MKSPEC.INSTALL_PROGRAM"""
        mkdir -p $IMAGE/lib/helix
        mkdir -p $IMAGE/bin
        for file in $HELIX_PATH/$debug/*.so*; do
            echo $INSTALL_FILE $file $IMAGE/lib/helix
            $INSTALL_FILE $file $IMAGE/lib/helix
            if [ $debug = release -a -n ""$STRIP"" ]; then
                echo $STRIP $STRIPFLAGS $IMAGE/lib/helix/$(basename $file)
                $STRIP $STRIPFLAGS $IMAGE/lib/helix/$(basename $file)
            fi
        done
        echo $INSTALL_PROGRAM $HELIX_PATH/$debug/splay $IMAGE/bin
        $INSTALL_PROGRAM $HELIX_PATH/$debug/splay $IMAGE/bin
        if [ $debug = release -a -n ""$STRIP"" ]; then
            echo $STRIP $IMAGE/bin/splay
            $STRIP $IMAGE/bin/splay
        fi
        "
]

image [
    TYPE=RULE
    prerequisiteActions+=install_libs
]

<script>
project.property("HELIX_BUILD_RULE").setValue(project.name+"build_helix");
</script>

dep.headers.TYPE=DEPENDS PERSISTED
dep.headers.EVAL=\
    "HELIX_PATH="$$HELIX_PATH\
    "INCLUDEPATH+="$$HELIX_PATH"/common/include"\
    "INCLUDEPATH+="$$HELIX_PATH"/common/dbgtool/pub"\
    "INCLUDEPATH+="$$HELIX_PATH"/common/runtime/pub"\
    "INCLUDEPATH+="$$HELIX_PATH"/common/util/pub"\
    "INCLUDEPATH+="$$HELIX_PATH"/client/include"\
    "INCLUDEPATH+="$$HELIX_PATH"/video/include"\
    "INCLUDEPATH+="$$HELIX_PATH"/client/videosvc/pub"\
    "INCLUDEPATH+="$$HELIX_PATH"/video/colconverter/pub"\
    "HELIX_BUILD_RULE="$$HELIX_BUILD_RULE

