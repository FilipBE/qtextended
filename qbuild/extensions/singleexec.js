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

\extension singleexec

*/
function singleexec_init()
{
###
    QMAKE.FINALIZE.singleexec.CALL=singleexec_finalize
    QMAKE.FINALIZE.singleexec.RUN_BEFORE_ME=qtopia defaults i18n
    QMAKE.FINALIZE.singleexec.RUN_AFTER_ME=cpp_compiler templates installs
###
}

function singleexec_finalize()
{
    if ( !project.config("enable_singleexec") || !project.config("singleexec") )
        return;

###
    equals(TEMPLATE,app)|if(qtopia:quicklaunch) {
        instsymlink.path=/bin
        instsymlink.commands=\
            "#(e)rm -f $$QTOPIA_IMAGE/$$instsymlink.path/"$$TARGET\
            "$$MKSPEC.SYMBOLIC_LINK qpe $$QTOPIA_IMAGE/$$instsymlink.path/"$$TARGET
        instsymlink.hint=image
        TARGET=plugin_application_$$TARGET
        CONFIG+=singleexec_link
    } else:plugin {
        DEFINES+=QTOPIA_PLUGIN_TYPE=$$define_string($$PLUGIN_TYPE)
        DEFINES+=QTOPIA_PLUGIN_NAME=$$define_string($$TARGET)
        DEFINES+=QT_STATICPLUGIN
        TARGET=plugin_$${PLUGIN_TYPE}_$$TARGET
        CONFIG+=singleexec_link
    }

    DEFINES*=SINGLE_EXEC QTOPIA_NO_MAIN QTOPIA_APP_INTERFACE QTOPIA_INTERNAL_INITAPP
    equals(TEMPLATE,app):!quicklaunch:DEFINES*=SINGLE_EXEC_USE_MAIN

    TEMPLATE=lib
    TARGETDIR=QtopiaSdk:/lib
    COMPILER.TARGETDIR=$$path(QtopiaSdk:/lib,generated)
    CONFIG+=staticlib
    CONFIG-=plugin
    target.hint=
###
}

