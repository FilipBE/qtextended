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
#ifndef APPOINTMENTPICKER_H
#define APPOINTMENTPICKER_H

#include <qdialog.h>
#include <qdatetime.h>
#include <QAppointmentModel>

class QStackedWidget;
class DayView;
class MonthView;
class QAppointment;
class DateBook;


class AppointmentPicker : public QDialog
{
    Q_OBJECT

public:
    AppointmentPicker( DateBook *db, QSet<QPimSource> sources, QWidget *parent = 0, Qt::WFlags f = 0 );
    ~AppointmentPicker();

    bool appointmentSelected() const;
    QAppointment currentAppointment() const;
    QDate currentDate();

private slots:
    void nextView();
    void viewDay(const QDate& dt);
    void viewMonthAgain();

private:
    void viewToday();
    void viewDay();
    void viewMonth();
    void viewMonth(const QDate& dt);
    void initDay();
    void initMonth();

    QStackedWidget *views;
    DayView *dayView;
    MonthView *monthView;
    QDate lastToday; // last date that was the selected as 'Today'
    DateBook *datebook;
    QSet<QPimSource> mSources;

};



#endif
