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

#ifndef DATEBOOKSETTINGS_H
#define DATEBOOKSETTINGS_H

#include <QDialog>
#include <QAppointment>

class QGroupBox;
class QSpinBox;
class QComboBox;
class ReminderPicker;

class DateBookSettings : public QDialog
{
    Q_OBJECT
public:
    DateBookSettings( bool whichClock, QWidget *parent = 0,
                      Qt::WFlags = 0 );
    virtual ~DateBookSettings();

    void setStartTime( int newStartViewTime );
    int startTime() const;

    void setPresetAlarm(QAppointment::AlarmFlags, int minutes);

    QAppointment::AlarmFlags alarmType() const;
    int alarmDelay() const;

    enum ViewType {DayView = 0, MonthView, WeekView};
    ViewType defaultView() const;
    void setDefaultView( ViewType viewType);

private slots:
    void slot12Hour( int );
    void slotChangeClock( bool );

protected:
    QGroupBox *fraView;
    QComboBox *cmbDefaultView;
    QSpinBox *spinStart;
    QGroupBox *fraAlarm;
    ReminderPicker *picker;

private:
    void init();
    bool ampm;
    int oldtime;
    QAppointment mAppt;
};
#endif
