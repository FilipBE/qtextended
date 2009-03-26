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
#ifndef APPOINTMENTDETAILS_H
#define APPOINTMENTDETAILS_H

#include <qtextbrowser.h>
#include <QOccurrence>
#include <QDLBrowserClient>

class DateBook;

class AppointmentDetails : public QDLBrowserClient
{
    friend class DateBook;

    Q_OBJECT
public:
    AppointmentDetails( QWidget *parent = 0 );

    void init( const QOccurrence &ev );

    QOccurrence occurrence() const { return mOccurrence; }

signals:
    void done();

protected:
    void keyPressEvent( QKeyEvent * );
    QString createPreviewText( const QOccurrence &ev );
    static QString formatDateTimes(const QOccurrence& ev, const QDate& today);
    static QString formatDate(const QDate& date, const QDate& today);

private:
    QWidget   *previousDetails;
    QOccurrence mOccurrence;
    bool mIconsLoaded;
};

#endif
