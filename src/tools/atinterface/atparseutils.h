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

#ifndef ATPARSEUTILS_H
#define ATPARSEUTILS_H

#include <QString>

class AtParseUtils
{
private:
    AtParseUtils() {}

public:

    /*!
        \enum AtParseUtils::Mode

        This enumeration defines the possible types of AT commands.
        \value CommandOnly  No other characters
        \value Get  Command followed by ? character
        \value Set  Command followed by = character
        \value Support  Command followed by =? characters
        \value Error  Invalid or malformed command

        \sa mode()
    */
    enum Mode
    {
        CommandOnly,            // AT+CFOO only
        Get,                    // AT+CFOO?
        Set,                    // AT+CFOO=value
        Support,                // AT+CFOO=?
        Error                   // Syntax error
    };
    static Mode mode( const QString& params );

};

#endif
