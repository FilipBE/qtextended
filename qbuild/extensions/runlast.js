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

\extension runlast

This extension allows you run snippets of code "last". That is, after all other extensions have been finalized.

You need to enable it.

\code
CONFIG+=runlast
RUNLAST+="message(hi mom)"
\endcode

*/

/*!

\qbuild_variable RUNLAST
\ingroup runlast_extension

Assign snippets of code to this variable to have them run "last".
Each element of the list is one snippet so quoting is important.

\code
RUNLAST+=\
    "message(hi mom)"\
    "contains(SOURCES,main.cpp):message(has a main.cpp)"
\endcode

*/


function runlast_init()
{
    // There's no way to make something run "last" so this extension is specially handled in qbuild.
###
    QMAKE.FINALIZE.runlast.CALL = runlast_finalize
    RUNLAST=
###
}

function runlast_finalize()
{
    var runlast = project.property("RUNLAST").value();
    if ( !runlast || !runlast.length )
        return;

    for ( var ii in runlast ) {
        var snippet = runlast[ii];
        project.run(snippet);
    }
}

