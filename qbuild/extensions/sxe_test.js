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

\extension sxe_test

*/

function sxe_test_init()
{
###
    QMAKE.FINALIZE.sxe_test [
        CALL = sxe_test_finalize
        RUN_BEFORE_ME = cpp_compiler
    ]
###
}

function sxe_test_finalize()
{
###
    sxe_test.TYPE=RULE
    sxe_test.inputFiles=$$COMPILER.TARGETFILE
    sxe_test.commands=\
        "#(eh)echo sxe_test $$[INPUT.0]"\
        "#(ve)TARGET="""$$[INPUT.0]"""
            if strings $TARGET | grep XOXOXOauthOXOXOX99 >/dev/null 2>&1; then
                :
            else
                echo ""ERROR: $TARGET does not have the required SXE symbols.""
                echo ""Please see the QSXE_APP_KEY documentation for details.""
                echo ""Hint: Type QSXE_APP_KEY into assistant's index box.""
                exit 1
            fi
        "

    default.TYPE=RULE
    default.prerequisiteActions+=sxe_test
###
}

