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

\extension embedded

The embedded extension switches from the 'host' mkspec to the 'target' mkspec.
This is a once-only change per project and is non-reversible.

*/

function embedded_init()
{
    if ( !project.resetReason().contains("embedded") )
        project.reset("embedded");
###
    # This is where the mkspec should be located
    path=QtopiaSdk:$$XPLATFORM_SDK

    # The first time Qtopia is built, the mkspec won't be in the SDK
    # Look in the build/source tree instead
    path+=$$XPLATFORM_SDK

    # If we need this one, it means the mkspec is not in any of the SDK,
    # build or source trees. Must be something external.
    # (eg. configure -xplatform /over/there)
    path+=$$XPLATFORM_ABS

    mkspec_load($$path)
###
}

