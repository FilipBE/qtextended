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

#include "ipvalidator.h"

class IPValidatorPrivate {
public:
    IPValidatorPrivate()
    {
    }

    static QValidator::State validateIP4( QString &key, int& pos);
    static QValidator::State validateIP6( QString &key, int& pos );
private:
};

QValidator::State IPValidatorPrivate::validateIP4( QString &key, int& /*pos*/)
{
    QString k;
    int lastNumber = 0;
    int count = 0;
    for ( int i = 0; i<key.length(); i++ ) {
        char c = key[i].toUpper().toLatin1();
        if ( c>= '0' && c<= '9' ) {
            lastNumber = QChar( c ).digitValue() + lastNumber*10;
            if ( lastNumber > 255 || lastNumber < 0  )
                return QValidator::Invalid;
            else {
                k += c;
            }
        } else if ( c == '.' && count<3 ) {
            count++;
            lastNumber = 0;
            k += c;
        } else {
            return QValidator::Invalid;
        }
    }

    key = k;
    return QValidator::Acceptable;
}

QValidator::State IPValidatorPrivate::validateIP6( QString &key, int& /*pos*/ )
{
    QString k;
    int digitCount = 0;
    bool doubleColon = false;
    int colonCount = 0;
    char lastchar = 0;
    bool hasLetter = false;
    bool ip4Mode = false;
    int lastIP4Number = 0;
    int dotCount = 0;
    bool prefix = false;
    int prefixNumber = 0;

    for ( int i = 0; i<key.length(); i++ ) {
        char c = key[i].toLower().toLatin1();
        if ( c>='0' && c<='9' && digitCount < 4 && !prefix && !ip4Mode)
        {
            //parse IPv6 number
            digitCount++;
            lastIP4Number = QChar( c ).digitValue() + lastIP4Number*10;
        }
        else if ( c>='a' && c<='f' && digitCount < 4 && !prefix && !ip4Mode)
        {
            //parse IPv6 number
            hasLetter = true;
            digitCount++;
            lastIP4Number = 0;
        }
        else if ( c == ':' && colonCount < 7 && !prefix && !ip4Mode)
        {
            //parse IPv6 colons
            digitCount = 0;
            colonCount++;
            hasLetter = false;
            if ( lastchar == c ) {
                if ( !doubleColon ) //we can have one double colon in whole address only
                    doubleColon = true;
                else
                    return QValidator::Invalid;
            }
            lastIP4Number = 0;
        }
        else if ( c == '.'  && !prefix && !hasLetter && lastchar != ':' && !ip4Mode &&
                lastIP4Number < 256 && lastIP4Number > 0 && colonCount > 2
                && colonCount < 7)  //an attached IPv4 address needs 32 bit
        {
            //parse legacy IPv4 part at end of IP6 address e.g. 10:10::10.3.3.5
            ip4Mode = true;
            lastIP4Number = 0;
            dotCount++;
        }
        else if ( c>='0' && c<='9' && !prefix && ip4Mode )
        {
            //parse ip4 address digits
            lastIP4Number = QChar( c ).digitValue() + lastIP4Number*10;
            if ( lastIP4Number > 255 || lastIP4Number < 0  )
                return QValidator::Invalid;
        }
        else if ( c == '.' && ip4Mode && dotCount < 3)
        {
            //dots for IP4 part
            dotCount++;
            lastIP4Number = 0;
        }
        else if ( c == '/' && !prefix && colonCount > 2)
        {
            // a valid IPv6 has at least 3 colons
            // now enter prefix mode
            prefix = true;
        }
        else if ( c>='1' && c<='9' && prefix)
        {
            //parse the prefix-length
            prefixNumber = QChar( c ).digitValue() + prefixNumber*10;
            if ( prefixNumber > 128 )
                return QValidator::Invalid;
        }
        else
        {
            return QValidator::Invalid;
        }

        k += c;
        lastchar = c;
    }

    key = k;
    return QValidator::Acceptable;
}


/*!
  \class IPValidator
    \inpublicgroup QtBaseModule
  \brief The IPValidator class provides validation for IP addresses.
  \internal


  The IPValidator is used by the Qt Extended network plug-ins. It is used for validation of
  user input for QLineEdit fields. It supports IPv4 and IPv6 addresses.
  */

IPValidator::IPValidator( QWidget* parent )
    : QValidator( parent ), d( 0 )
{
}

IPValidator::~IPValidator()
{
    if ( d )
        delete d;
    d = 0;
}

QValidator::State IPValidator::validate( QString &key, int &pos ) const
{
    //valid IPv4 address?
    QString kip4 = key;
    int pip4 = pos;
    QValidator::State ip4 = IPValidatorPrivate::validateIP4( kip4, pip4 );
    if ( ip4 == QValidator::Acceptable  ) {
        key = kip4;
        pos = pip4;
        return QValidator::Acceptable;
    }
    //valid IPv6 address?
    QString kip6 = key;
    int pip6 = pos;
    QValidator::State ip6 = IPValidatorPrivate::validateIP6( kip6, pip6 );
    if ( ip6 == QValidator::Acceptable  ) {
        key = kip6;
        pos = pip6;
        return QValidator::Acceptable;
    }

    return QValidator::Invalid;
}

void IPValidator::fixup( QString & input ) const
{
    //we may want to reimplement this later
    fixup_impl( input );
}

void IPValidator::fixup_impl( QString & input ) const
{
    QValidator::fixup( input );
}
