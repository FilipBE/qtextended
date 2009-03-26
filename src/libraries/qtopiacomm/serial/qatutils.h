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

#ifndef QATUTILS_H
#define QATUTILS_H

#include <qtopiaglobal.h>

#include <qobject.h>
#include <qstring.h>
#include <qbytearray.h>

class QAtResultParser;
class QTextCodec;

class QTOPIACOMM_EXPORT QAtUtils
{
private:
    QAtUtils() {}

public:

    static QString quote( const QString& str );
    static QString quote( const QString& str, QTextCodec *codec );
    static QString decode( const QString& str, QTextCodec *codec );
    static QTextCodec *codec( const QString& gsmCharset );
    static QString toHex( const QByteArray& binary );
    static QByteArray fromHex( const QString& hex );
    static QString decodeNumber( const QString& value, uint type );
    static QString decodeNumber( QAtResultParser& parser );
    static QString encodeNumber( const QString& value, bool keepPlus = false );
    static QString nextString( const QString& buf, uint& posn );
    static uint parseNumber( const QString& str, uint& posn );
    static void skipField( const QString& str, uint& posn );
    static QString stripNumber( const QString& number );
    static bool octalEscapes();
    static void setOctalEscapes( bool value );
    static QString decodeString( const QString& value, uint dcs );
};

#endif
