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

#include "atparseutils.h"

/*!
    \class AtParseUtils
    \inpublicgroup QtTelephonyModule
    \brief The AtParseUtils class provides several utility functions that assist with parsing AT commands
*/

/*!
    \fn AtParseUtils::Mode AtParseUtils::mode( const QString& params )
    Determines the mode that an extension command is operating in
    according to the \a params to the command.
    Returns the mode of the command.
*/
AtParseUtils::Mode AtParseUtils::mode( const QString& params )
{
    if ( params.isEmpty() )
        return CommandOnly;
    if ( params[0] == QChar('=') ) {
        if ( params.length() > 1 && params[1] == QChar('?') ) {
            if ( params.length() != 2 )
                return Error;
            else
                return Support;
        } else {
            return Set;
        }
    }
    if ( params[0] == QChar('?') ) {
        if ( params.length() != 1 )
            return Error;
        else
            return Get;
    }
    return Error;
}

