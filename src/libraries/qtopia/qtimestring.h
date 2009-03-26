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

#ifndef QTIMESTRING_H
#define QTIMESTRING_H

#include <QDateTime>
#include <QString>

#include <qtopiaglobal.h>

class QTOPIA_EXPORT QTimeString
{
public:
    enum Length { Short, Medium, Long };
    static QString nameOfWeekDay( int day, Length len = Medium);
    static QString nameOfMonth( int month, Length len = Medium);

    static QString numberDateString( const QDate &d, Length len = Medium );

    static QString currentFormat();
    static QStringList formatOptions();

    static QString localH( int hour );
    static QString localHM( const QTime &t, Length len = Medium );
    static QString localHMS( const QTime &t, Length len = Medium );
    static QString localHMDayOfWeek( const QDateTime &dt, Length len = Medium);
    static QString localHMSDayOfWeek( const QDateTime &dt, Length len = Medium);
    static QString localMD( const QDate &d, Length len = Medium );
    static QString localYMD( const QDate &d, Length len = Medium );
    static QString localYMDHMS( const QDateTime &d, Length len = Medium );
    static QString localDayOfWeek( const QDate &d, Length len = Medium );

    static bool currentAMPM();
private:
    friend class QtopiaApplication;
    static void updateFormats();

};

#endif
