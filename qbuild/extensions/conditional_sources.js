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

\extension conditional_sources

Here's how to do conditional sources.

\code
FOO.TYPE=CONDITIONAL_SOURCES
FOO.CONDITION=enable_my_condition
FOO.HEADERS=foo.h
FOO.SOURCES=foo.cpp
!qbuild:CONDITIONAL_SOURCES(FOO)
\endcode

This mechanism can be used to set the following variables.
\list
\o FORMS
\o HEADERS
\o PRIVATE_HEADERS
\o SEMI_PRIVATE_HEADERS
\o SOURCES
\o RESOURCES
\endlist

Note that the changes to these variables are reflected only after the
conditional_sources extension has finalized. If you need to force this
to happen before your .pro file has finished parsing you can do so.

\code
conditional_sources_finalize()
\endcode

*/

function conditional_sources_init()
{
###
    QMAKE.FINALIZE.conditional_sources.CALL = conditional_sources_finalize
    QMAKE.FINALIZE.conditional_sources.RUN_AFTER_ME = rules
###
}

function conditional_sources_finalize()
{
    if (!project.config("conditional_sources"))
        return;

    var vars = new Array("HEADERS", "SOURCES", "FORMS", "PRIVATE_HEADERS", "SEMI_PRIVATE_HEADERS", "RESOURCES");
    var sources = project.find("TYPE", "CONDITIONAL_SOURCES");
    for (var ii in sources) {
        var obj = qbuild.object(sources[ii]);
        if ( obj.isProperty("CONDITION") ) {
            var condition = obj.property("CONDITION").strValue();
### condition
            cs_ret=0
            <0>:cs_ret=1
###
            if ( project.property("cs_ret").strValue() == "1" ) {
                //project.message(obj.name+" is ON");
                for ( var ii in vars ) {
                    var v = vars[ii];
                    project.property(v).unite(obj.property(v).value());
                }
            } else {
                //project.message(obj.name+" is OFF");
            }
        } else {
            project.warning(obj.name+" has TYPE=CONDITIONAL_SOURCES but has no CONDITION.");
        }
    }
}

