/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2009 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

/*!

\extension mkspec

The mkspec tells QBuild how to invoke your compiler and other system utilities.

QBuild currently uses qmake mkspecs for compatibility. Note that it may not use all
of the qmake.conf values that can be set. There are also additional variables that
can be set in qmake.conf to allow QBuild greater control (for example, the optimization
flags). These flags are clearly marked by the \c{QBUILD_} prefix.

*/

var mkspec = {
    properties: {
        /*!
          \qbuild_variable MKSPEC.CC
          \ingroup mkspec_extension
          This variable contains the compiler for C files.
          This comes from the QMAKE_CC variable in qmake.conf.
          This value must be set.
        */
        CC: {
            source: "QMAKE_CC",
            mandatory: 1
        },
        /*!
          \qbuild_variable MKSPEC.CFLAGS
          \ingroup mkspec_extension
          This variable contains the flags for the C compiler.
          This comes from the QMAKE_CFLAGS variable in qmake.conf.
        */
        CFLAGS: {
            source: "QMAKE_CFLAGS"
        },
        /*!
          \qbuild_variable MKSPEC.CFLAGS_WARN_ON
          \ingroup mkspec_extension
          This variable contains the flags that turn on warnings for the C compiler.
          This comes from the QMAKE_CFLAGS_WARN_ON variable in qmake.conf.
          If unset, this value defaults to \c{-Wall -W}.
        */
        CFLAGS_WARN_ON: {
            source: "QMAKE_CFLAGS_WARN_ON",
            defaultval: "-Wall -W"
        },
        /*!
          \qbuild_variable MKSPEC.CFLAGS_WARN_OFF
          \ingroup mkspec_extension
          This variable contains the flags that turn off warnings for the C compiler.
          This comes from the QMAKE_CFLAGS_WARN_OFF variable in qmake.conf.
          If unset, this value defaults to \c{-w}.
        */
        CFLAGS_WARN_OFF: {
            source: "QMAKE_CFLAGS_WARN_OFF",
            defaultval: "-w"
        },
        /*!
          \qbuild_variable MKSPEC.CFLAGS_OPTBASE
          \ingroup mkspec_extension
          This variable contains flags for the C compiler that enable basic optimizations.
          This comes from the QBUILD_CFLAGS_OPTBASE variable in qmake.conf.
          For optimizations that increase the code size, see \l MKSPEC.CFLAGS_OPTMORE.
          Note that you should assign the contents of this variable to QMAKE_CFLAGS_RELEASE for
          compatibility with qmake.
        */
        CFLAGS_OPTBASE: {
            source: "QBUILD_CFLAGS_OPTBASE"
        },
        /*!
          \qbuild_variable MKSPEC.CFLAGS_OPTMORE
          \ingroup mkspec_extension
          This variable contains flags for the C compiler that enable extra optimizations.
          This comes from the QBUILD_CFLAGS_OPTMORE variable in qmake.conf.
          For optimizations that do not increase the code size, see \l MKSPEC.CFLAGS_OPTBASE.
          Note that you should assign the contents of this variable to QMAKE_CFLAGS_RELEASE for
          compatibility with qmake.
        */
        CFLAGS_OPTMORE: {
            source: "QBUILD_CFLAGS_OPTMORE"
        },
        /*!
          \qbuild_variable MKSPEC.CFLAGS_RELEASE
          \ingroup mkspec_extension
          This variable is provided for compatibility with qmake.
          It comes from the QMAKE_CFLAGS_RELEASE variable in qmake.conf.
          Note that this variable is only used if \l MKSPEC.CFLAGS_OPTBASE is undefined.
        */
        CFLAGS_RELEASE: {
            source: "QMAKE_CFLAGS_RELEASE"
        },
        /*!
          \qbuild_variable MKSPEC.CFLAGS_DEBUG
          \ingroup mkspec_extension
          This variable contains flags for the C compiler that enable debugging.
          This comes from the QMAKE_CFLAGS_DEBUG variable in qmake.conf.
        */
        CFLAGS_DEBUG: {
            source: "QMAKE_CFLAGS_DEBUG"
        },
        /*!
          \qbuild_variable MKSPEC.CFLAGS_PIC
          \ingroup mkspec_extension
          This variable contains flags for the C compiler that turn on position-independant
          code generation.
          This comes from the QMAKE_CFLAGS_SHLIB variable in qmake.conf.
          Note that these flags are also passed to static libraries that request this.
        */
        CFLAGS_PIC: {
            source: "QMAKE_CFLAGS_SHLIB"
        },
        /*!
          \qbuild_variable MKSPEC.CFLAGS_THREAD
          \ingroup mkspec_extension
          This variable contains flags for the C compiler when threading is enabled.
          This comes from the QMAKE_CFLAGS_THREAD variable in qmake.conf.
          Note that Qt Extended does not support building without threading so this value is always used.
        */
        CFLAGS_THREAD: {
            source: "QMAKE_CFLAGS_THREAD"
        },
        /*!
          \qbuild_variable MKSPEC.CFLAGS_HIDESYMS
          \ingroup mkspec_extension
          This variable contains flags for the C compiler that cause symbol hiding to be enabled.
          This comes from the QMAKE_CFLAGS_HIDESYMS variable in qmake.conf.
        */
        CFLAGS_HIDESYMS: {
            source: "QMAKE_CFLAGS_HIDESYMS"
        },
        /*!
          \qbuild_variable MKSPEC.CXX
          \ingroup mkspec_extension
          This variable contains compiler used for C++ files.
          This comes from the QMAKE_CXX variable in qmake.conf.
          This value must be set.
        */
        CXX: {
            source: "QMAKE_CXX",
            mandatory: 1
        },
        /*!
          \qbuild_variable MKSPEC.CXXFLAGS
          \ingroup mkspec_extension
          This variable contains the compiler flags used for C++ files.
          This comes from the QMAKE_CXXFLAGS variable in qmake.conf.
          If unset, this value defaults to the contents of \l MKSPEC.CFLAGS.
        */
        CXXFLAGS: {
            source: "QMAKE_CXXFLAGS",
            defaultfrom: "CFLAGS"
        },
        /*!
          \qbuild_variable MKSPEC.CXXFLAGS_DISABLE_EXCEPTIONS
          \ingroup mkspec_extension
          This variable contains the compiler flags used to disable exceptions.
          This comes from the QBUILD_CXXFLAGS_DISABLE_EXCEPTIONS variable in qmake.conf.
          If unset, this value defaults to \c{-fno-exceptions}.
        */
        CXXFLAGS_DISABLE_EXCEPTIONS: {
            source: "QBUILD_CXXFLAGS_DISABLE_EXCEPTIONS",
            defaultval: "-fno-exceptions"
        },
        /*!
          \qbuild_variable MKSPEC.CXXFLAGS_DISABLE_RTTI
          \ingroup mkspec_extension
          This variable contains the compiler flags used to disable RTTI.
          This comes from the QBUILD_CXXFLAGS_DISABLE_RTTI variable in qmake.conf.
          If unset, this value defaults to \c{-fno-rtti}.
        */
        CXXFLAGS_DISABLE_RTTI: {
            source: "QBUILD_CXXFLAGS_DISABLE_RTTI",
            defaultval: "-fno-rtti"
        },
        /*!
          \qbuild_variable MKSPEC.CXXFLAGS_WARN_ON
          \ingroup mkspec_extension
          This variable contains the C++ compiler flags to turn on warnings.
          This comes from the QMAKE_CXXFLAGS_WARN_ON variable in qmake.conf.
          If unset, this value defaults to the contents of \l MKSPEC.CFLAGS_WARN_ON.
        */
        CXXFLAGS_WARN_ON: {
            source: "QMAKE_CXXFLAGS_WARN_ON",
            defaultfrom: "CFLAGS_WARN_ON"
        },
        /*!
          \qbuild_variable MKSPEC.CXXFLAGS_WARN_OFF
          \ingroup mkspec_extension
          This variable contains the C++ compiler flags to turn off warnings.
          This comes from the QMAKE_CXXFLAGS_WARN_OFF variable in qmake.conf.
          If unset, this value defaults to the contents of \l MKSPEC.CFLAGS_WARN_OFF.
        */
        CXXFLAGS_WARN_OFF: {
            source: "QMAKE_CXXFLAGS_WARN_OFF",
            defaultfrom: "CFLAGS_WARN_OFF"
        },
        /*!
          \qbuild_variable MKSPEC.CXXFLAGS_OPTBASE
          \ingroup mkspec_extension
          This variable contains flags for the C++ compiler that enable basic optimizations.
          This comes from the QBUILD_CXXFLAGS_OPTBASE variable in qmake.conf.
          For optimizations that increase the code size, see \l MKSPEC.CXXFLAGS_OPTMORE.
          Note that you should assign the contents of this variable to QMAKE_CXXFLAGS_RELEASE for
          compatibility with qmake.
          If unset, this value defaults to the contents of \l MKSPEC.CFLAGS_OPTBASE.
        */
        CXXFLAGS_OPTBASE: {
            source: "QBUILD_CXXFLAGS_OPTBASE",
            defaultfrom: "CFLAGS_OPTBASE"
        },
        /*!
          \qbuild_variable MKSPEC.CXXFLAGS_OPTMORE
          \ingroup mkspec_extension
          This variable contains flags for the C++ compiler that enable extra optimizations.
          This comes from the QBUILD_CXXFLAGS_OPTMORE variable in qmake.conf.
          For optimizations that do not increase the code size, see \l MKSPEC.CXXFLAGS_OPTBASE.
          Note that you should assign the contents of this variable to QMAKE_CXXFLAGS_RELEASE for
          compatibility with qmake.
          If unset, this value defaults to the contents of \l MKSPEC.CFLAGS_OPTMORE.
        */
        CXXFLAGS_OPTMORE: {
            source: "QBUILD_CXXFLAGS_OPTMORE",
            defaultfrom: "CFLAGS_OPTMORE"
        },
        /*!
          \qbuild_variable MKSPEC.CXXFLAGS_RELEASE
          \ingroup mkspec_extension
          This variable is provided for compatibility with qmake.
          It comes from the QMAKE_CXXFLAGS_RELEASE variable in qmake.conf.
          Note that this variable is only used if \l MKSPEC.CXXFLAGS_OPTBASE is undefined.
          If unset, this value defaults to the contents of \l MKSPEC.CFLAGS_RELEASE.
        */
        CFLAGS_RELEASE: {
            source: "QMAKE_CFLAGS_RELEASE",
            defaultfrom: "CFLAGS_RELEASE"
        },
        /*!
          \qbuild_variable MKSPEC.CXXFLAGS_RELEASE
          \ingroup mkspec_extension
          This variable contains the C++ compiler flags suitable for release builds.
          This comes from the QMAKE_CXXFLAGS_RELEASE variable in qmake.conf.
          If unset, this value defaults to the contents of \l MKSPEC.CFLAGS_RELEASE.
        */
        CXXFLAGS_RELEASE: {
            source: "QMAKE_CXXFLAGS_RELEASE",
            defaultfrom: "CFLAGS_RELEASE"
        },
        /*!
          \qbuild_variable MKSPEC.CXXFLAGS_DEBUG
          \ingroup mkspec_extension
          This variable contains the C++ compiler flags to enable debugging.
          This comes from the QMAKE_CXXFLAGS_DEBUG variable in qmake.conf.
        */
        CXXFLAGS_DEBUG: {
            source: "QMAKE_CXXFLAGS_DEBUG",
            defaultfrom: "CFLAGS_DEBUG"
        },
        /*!
          \qbuild_variable MKSPEC.CXXFLAGS_HIDESYMS
          \ingroup mkspec_extension
          This variable contains the C++ compiler flags that enable symbol hiding.
          This comes from the QMAKE_CXXFLAGS_HIDESYMS variable in qmake.conf.
        */
        CXXFLAGS_HIDESYMS: {
            source: "QMAKE_CXXFLAGS_HIDESYMS",
            defaultfrom: "CFLAGS_HIDESYMS"
        },
        /*!
          \qbuild_variable MKSPEC.LFLAGS
          \ingroup mkspec_extension
          This variable contains the flags passed when linking.
          This comes from the QMAKE_LFLAGS variable in qmake.conf.
        */
        LFLAGS: {
            source: "QMAKE_LFLAGS"
        },
        /*!
          \qbuild_variable MKSPEC.LFLAGS_RELEASE
          \ingroup mkspec_extension
          This variable contains the link flags suitable for release build.
          This comes from the QMAKE_LFLAGS_RELEASE variable in qmake.conf.
        */
        LFLAGS_RELEASE: {
            source: "QMAKE_LFLAGS_RELEASE"
        },
        /*!
          \qbuild_variable MKSPEC.LFLAGS_OPTIMIZE
          \ingroup mkspec_extension
          This variable contains the link flags that enable optimization.
          This comes from the QMAKE_LFLAGS_OPTIMIZE variable in qmake.conf.
        */
        LFLAGS_OPTIMIZE: {
            source: "QMAKE_LFLAGS_OPTIMIZE"
        },
        /*!
          \qbuild_variable MKSPEC.LFLAGS_DEBUG
          \ingroup mkspec_extension
          This variable contains the link flags that enable debugging.
          This comes from the QMAKE_LFLAGS_DEBUG variable in qmake.conf.
        */
        LFLAGS_DEBUG: {
            source: "QMAKE_LFLAGS_DEBUG"
        },
        /*!
          \qbuild_variable MKSPEC.LFLAGS_SHLIB
          \ingroup mkspec_extension
          This variable contains the link flags for shared libraries.
          This comes from the QMAKE_LFLAGS_SHLIB variable in qmake.conf.
        */
        LFLAGS_SHLIB: {
            source: "QMAKE_LFLAGS_SHLIB"
        },
        /*!
          \qbuild_variable MKSPEC.LFLAGS_PLUGIN
          \ingroup mkspec_extension
          This variable contains the link flags for plugins.
          This comes from the QMAKE_LFLAGS_PLUGIN variable in qmake.conf.
        */
        LFLAGS_PLUGIN: {
            source: "QMAKE_LFLAGS_PLUGIN",
            defaultfrom: "LFLAGS_SHLIB"
        },
        /*!
          \qbuild_variable MKSPEC.LFLAGS_SONAME
          \ingroup mkspec_extension
          This variable contains the link flag that sets the soname.
          This comes from the QMAKE_LFLAGS_SONAME variable in qmake.conf.
        */
        LFLAGS_SONAME: {
            source: "QMAKE_LFLAGS_SONAME"
        },
        /*!
          \qbuild_variable MKSPEC.RPATH
          \ingroup mkspec_extension
          This variable contains the link flag that sets the rpath.
          This comes from the QMAKE_RPATH variable in qmake.conf.
        */
        RPATH: {
            source: "QMAKE_RPATH"
        },
        /*!
          \qbuild_variable MKSPEC.MKDIR
          \ingroup mkspec_extension
          This variable contains the command to create directories.
          This comes from the QMAKE_MKDIR variable in qmake.conf.
          If unset, this value defaults to \c{mkdir -p}.
        */
        MKDIR: {
            source: "QMAKE_MKDIR",
            defaultval: "mkdir -p"
        },
        /*!
          \qbuild_variable MKSPEC.DEL_FILE
          \ingroup mkspec_extension
          This variable contains the command to delete files.
          This comes from the QMAKE_DEL_FILE variable in qmake.conf.
          If unset, this value defaults to \c{rm -f}.
        */
        DEL_FILE: {
            source: "QMAKE_DEL_FILE",
            defaultval: "rm -f"
        },
        /*!
          \qbuild_variable MKSPEC.STRIP
          \ingroup mkspec_extension
          This variable contains the command to strip executables.
          This comes from the QMAKE_STRIP variable in qmake.conf.
        */
        STRIP: {
            source: "QMAKE_STRIP"
        },
        /*!
          \qbuild_variable MKSPEC.STRIPFLAGS_SHLIB
          \ingroup mkspec_extension
          This variable contains the flags used when stripping shared libraries.
          This comes from the QMAKE_STRIPFLAGS_SHLIB variable in qmake.conf.
        */
        STRIPFLAGS_SHLIB: {
            source: "QMAKE_STRIPFLAGS_SHLIB"
        },
        /*!
          \qbuild_variable MKSPEC.SYMBOLIC_LINK
          \ingroup mkspec_extension
          This variable contains the command to create a symbolic link.
          This comes from the QBUILD_SYMBOLIC_LINK variable in qmake.conf.
          If unset, this value defaults to \c{ln -s}.
        */
        SYMBOLIC_LINK: {
            source: "QBUILD_SYMBOLIC_LINK",
            defaultval: "ln -s"
        },
        /*!
          \qbuild_variable MKSPEC.INSTALL_FILE
          \ingroup mkspec_extension
          This variable contains the command to install a file.
          This comes from the QBUILD_INSTALL_FILE variable in qmake.conf.
          If unset, this value defaults to \c{install -m 644}.
        */
        INSTALL_FILE: {
            source: "QBUILD_INSTALL_FILE",
            defaultval: "install -m 644"
        },
        /*!
          \qbuild_variable MKSPEC.INSTALL_DIR
          \ingroup mkspec_extension
          This variable contains the command to install a directory.
          This comes from the QBUILD_INSTALL_DIR variable in qmake.conf.
          If unset, this value defaults to \c{cp -Rf}.
        */
        INSTALL_DIR: {
            source: "QBUILD_INSTALL_DIR",
            defaultval: "cp -Rf"
        },
        /*!
          \qbuild_variable MKSPEC.INSTALL_PROGRAM
          \ingroup mkspec_extension
          This variable contains the command to install an executable.
          This comes from the QBUILD_INSTALL_PROGRAM variable in qmake.conf.
          If unset, this value defaults to \c{install -m 755}.
        */
        INSTALL_PROGRAM: {
            source: "QBUILD_INSTALL_PROGRAM",
            defaultval: "install -m 755"
        },
        /*!
          \qbuild_variable MKSPEC.AR
          \ingroup mkspec_extension
          This variable contains the command to create a static library.
          This comes from the QMAKE_AR variable in qmake.conf.
        */
        AR: {
            source: "QMAKE_AR"
        }
    }
}

function mkspec_init()
{
    if ( project.absName.match(/\{file\}\/$/) )
        return;

###
    QMAKE.FINALIZE.mkspec.CALL = mkspec_finalize
###

    var default_mkspec;
    if ( project.resetReason().contains("mkspec") ) {
        default_mkspec = project.resetReason().property("mkspec").strValue();
    } else {
        default_mkspec = qbuild.invoke("globalValue", "default_mkspec");
        if ( default_mkspec == "default" )
            default_mkspec = qbuild.qtValue("QT_DATA")+"/mkspecs/default";
    }
    mkspec_load(default_mkspec);
}

function mkspec_load(arg)
{
    if ( arg instanceof Array ) {
        for ( var ii in arg ) {
            var path = qbuild.invoke("path", arg[ii], "existing");
            if ( path != "" ) {
                mkspec_do_load(path, arg)
                return;
            }
        }
        project.error("Cannot load any of the mkspecs: "+arg.join(", "));
    } else {
        var path = qbuild.invoke("path", arg, "existing");
        if ( path )
            mkspec_do_load(arg);
        else
            project.error("Cannot load mkspec: "+arg);
    }
}

function mkspec_do_load(spec, alternatives)
{
    var out = project.property("MKSPEC");
    if ( !out.isEmpty() && out.strValue() != spec ) {
        if ( project.resetReason().contains("mkspec") )
            project.error("You cannot set the mkspec more than once (setting to "+spec+", was previously set to "+out.strValue()+")");
        project.reset("mkspec", spec);
        return;
    }
    echo("mkspec", "Loading mkspec "+spec);
    var mkspec_sproj = project.sproject(spec+"/qmake.conf");
    var file = mkspec_sproj.nodePath()+mkspec_sproj.filename();
    if ( qbuild.invoke("exists", file) != "true" ) {
        project.error("mkspec file is missing: "+file);
    }
    var mkspec_proj = mkspec_sproj.fileModeProject();
    out.clear();
    out.setValue(spec);
    for ( var ii in mkspec.properties ) {
        var prop = mkspec.properties[ii];
        var source = mkspec_proj.property(prop.source);
        if ( prop.mandatory && !source )
            project.error("MKSPEC "+spec+" does not set mandatory variable "+prop.source);
        if ( source && !source.isEmpty() )
            out.property(ii).setValue(source.value());
        else if ( prop.defaultval )
            out.property(ii).setValue(prop.defaultval);
        else if ( prop.defaultfrom ) {
            source = mkspec_proj.property(prop.defaultfrom);
            if ( source && !source.isEmpty() )
                out.property(ii).setValue(source.value());
        }
        if ( !out.isProperty(ii) )
            out.property(ii).setValue("");
        if ( out.property(ii).isEmpty() )
            echo("mkspec", "mkspec value "+ii+" is empty");
        else
            echo("mkspec", "mkspec value "+ii+" is \""+out.property(ii).strValue()+"\"");
    }
    // If CFLAGS_OPTBASE is not defined, set it to CFLAGS_RELEASE and clear CFLAGS_OPTMORE
    if (!mkspec_proj.isProperty("CFLAGS_OPTBASE")) {
        out.property("CFLAGS_OPTBASE").setValue(out.property("CFLAGS_RELEASE").value());
        out.property("CFLAGS_OPTMORE").setValue("");
    }
    if ( alternatives ) {
        // There's more than one mkspec that matches... Add them all so that switching
        // to the SDK does not cause everything to rebuild.
        for ( var ii in alternatives ) {
            var path = qbuild.invoke("path", alternatives[ii], "existing");
            if ( path == "" )
                path = qbuild.invoke("path", alternatives[ii], "generated");
            out.property("INCLUDEPATH").unite(path);
        }
    } else {
        out.property("INCLUDEPATH").unite(spec);
    }
}

function mkspec_finalize()
{
    var out = project.property("MKSPEC");
    var extra = project.property("MKSPEC_EXTRA");
    for ( var ii in mkspec.properties ) {
        if ( extra.isProperty(ii) ) {
            out.property(ii).append(extra.property(ii).value());
        }
    }
}

