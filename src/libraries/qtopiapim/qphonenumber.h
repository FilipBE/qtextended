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

#ifndef QPHONENUMBER_H
#define QPHONENUMBER_H

#include <qtopiaglobal.h>
#include <qobject.h>
#include <qstring.h>

class QTOPIAPIM_EXPORT QPhoneNumber
{
private:
    QPhoneNumber() {}

public:

    static QString stripNumber( const QString& number );

    static int matchNumbers( const QString& num1, const QString& num2 );

    static bool matchPrefix( const QString& num, const QString& prefix );

    static QString resolveLetters( const QString& number );

    static QString localNumber( const QString &number);
};

#endif
