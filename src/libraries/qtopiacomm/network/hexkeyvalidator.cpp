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

#include "hexkeyvalidator.h"

#include <QString>

/*!
  \internal

  \class HexKeyValidator
    \inpublicgroup QtBaseModule
  \brief The HexKeyValidator class validates hexadecimal keys such as WEP keys and MAC addresses.


  This class should be used in connection with a QLineEdit. It allows the validation 
  of user input.
*/

HexKeyValidator::HexKeyValidator( QWidget* parent, int numDigits )
    : QValidator(parent) , neededNumDigits( numDigits )
{
}

QValidator::State HexKeyValidator::validate( QString& key, int& curs ) const
{
    QString k;
    int hexes=0;
    int ncurs=0;
    int digitCount = 0;
    for (int i=0; i<key.length(); i++) {
        char c=key[i].toUpper().toLatin1();
        if ( c>='0' && c<='9' || c>='A' && c<='F' ) {
            if ( hexes == 2 ) {
                hexes = 0;
                k += ':';
                if ( i<curs ) ncurs++;
            }
            k += c;
            digitCount++;
            hexes++;
            if ( i<curs ) ncurs++;
        } else if ( c == ':' && hexes==2 ) {
            hexes = 0;
            k += c;
            if ( i<curs ) ncurs++;
        } else {
            return Invalid;
        }
    }
    key = k;
    curs = ncurs;
    if ( neededNumDigits ) {
        if ( digitCount < neededNumDigits )
            return Intermediate;
        else if ( digitCount > neededNumDigits )
            return Invalid;
        //else
        //  return Acceptable
    }
    return Acceptable;
}


