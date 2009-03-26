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

\extension qtopia

This extension handles linking to the Qt Extended public libs (as the qt extension does for Qt).

It can be used by adding:
\code
CONFIG+=qtopia
\endcode
to your project files.

*/

function qtopia_init()
{
###
    CONFIG*=qt embedded depends
    QMAKE.FINALIZE.qtopia.CALL = qtopia_finalize
    QMAKE.FINALIZE.qtopia.RUN_AFTER_ME = qt depends cpp_compiler rules installs templates defaults
    QTOPIA=base qtopialib
    DEPENDS.AUTO_PERSIST+=QTOPIA

    CONFIG *= phone

    drmagent:DEFINES+=QTOPIA_DRM
    enable_voip:DEFINES+=QTOPIA_VOIP
    enable_cell:DEFINES+=QTOPIA_CELL
    enable_bluetooth:DEFINES+=QTOPIA_BLUETOOTH
    enable_infrared:DEFINES+=QTOPIA_INFRARED
    enable_vpn:DEFINES+=QTOPIA_VPN
    enable_dbus_ipc:DEFINES+=QTOPIA_DBUS_IPC
    !enable_sxe:DEFINES+=QT_NO_SXE
    enable_pictureflow:DEFINES+=USE_PICTUREFLOW
    DEFINES+=QT_KEYPAD_NAVIGATION
    CONFIG += unix
    DEFINES += BUILDER=$$define_string($$BUILDER)

    QT_DEPOT_PATH=$$path(/qtopiacore/qt,existing)
###
}

function qtopia_finalize()
{
    if (!project.config("qtopia"))
        return;

###
    quicklaunch:equals(LAUNCH_METHOD,quicklaunch) {
        QTOPIA*=base qtopialib
        TEMPLATE=plugin
        PLUGIN_FOR=qtopia
        PLUGIN_TYPE=application
        DEFINES+=QTOPIA_APP_INTERFACE
        CONFIG+=hide_symbols

        instsymlink.path=/bin
        instsymlink.commands=\
            "#(e)rm -f $$QTOPIA_IMAGE/$$instsymlink.path/"$$TARGET\
            "$$MKSPEC.SYMBOLIC_LINK quicklauncher $$QTOPIA_IMAGE/$$instsymlink.path/"$$TARGET
        instsymlink.hint=image
    }
###

    // Pull in dependencies (which may add to QTOPIA)
    //project.info("calling depends_load_dependencies from qtopia_finalize");
    depends_load_dependencies();
    var modules = project.property("MODULES");
    var qtopia = project.property("QTOPIA").value();
    for ( var ii in qtopia ) {
        var module = qtopia[ii];
        modules.unite( qtopia_module(module) );
    }
}

function qtopia_module(module)
{
    if ( module == "qtopialib" ) {
        return "qtopia";
    } else if ( module == "handwriting" ) {
        return module;
    } else {
        return "qtopia"+module;
    }
}

