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
#include <typeconversion.h>

QString QDWIN32::tchar_to_qstring( TCHAR *string, int length )
{
    QString ret;
#ifdef UNICODE
    ret = QString( (QChar*)string, length );
#else
    ret = QString::fromLatin1( string, length );
#endif
    return ret;
}

TCHAR *QDWIN32::qstring_to_tchar( const QString &string )
{
#ifdef UNICODE
    return (unsigned short *)string.constData();
#else
    return QString.toLatin1().constData();
#endif
}

