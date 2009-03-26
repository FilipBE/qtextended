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
#ifndef ENTRYDIALOG_H
#define ENTRYDIALOG_H

#include <qappointment.h>

#include <QDateTime>
#include <QDialog>

class QTextEdit;
class QTabWidget;
class EntryDetails;
class QDLEditClient;
class QScrollArea;
class DateBookCategorySelector;
class RecurrenceDetails;
class ReminderPicker;
class QLabel;
class QLineEdit;
class QCheckBox;
class QDateEdit;
class QTimeEdit;
class QTimeZoneSelector;

class EntryDialog : public QDialog
{
    Q_OBJECT

public:
    EntryDialog( bool startOnMonday, const QAppointment &appointment, const QTime& defaultAllDayTime,
                 int defaultTimedReminder, QWidget *parent = 0, Qt::WFlags f = 0 );
    ~EntryDialog();

    QAppointment appointment( const bool includeQdlLinks = true);

private slots:
    void updateStartDateTime();
    void updateStartTime();
    void updateEndDateTime();
    void updateEndTime();
    void setWeekStartsMonday( bool );
    void updateTimeUI();
    void initTab(int, QScrollArea *);

private:
    void init();
    void initEventDetails(QScrollArea *);
    void initRepeatDetails(QScrollArea *);
    void initNoteDetails(QScrollArea *);


    void setDates( const QDateTime& s, const QDateTime& e );
    void accept();

    QAppointment mAppointment;
    QAppointment mOrigAppointment;
    bool startWeekOnMonday;
    QTime allDayReminder;
    int timedReminder;
    DateBookCategorySelector *comboCategory;
    RecurrenceDetails *recurDetails;
    ReminderPicker *reminderPicker;
    QTextEdit *editNote;
    QDLEditClient *editnoteQC;

    QLineEdit *mDescription, *mLocation;
    QCheckBox *checkAllDay;
    QDateEdit *startDate, *endDate;
    QTimeEdit *startTime, *endTime;
    QLabel *startTimeLabel, *endTimeLabel;
    QTimeZoneSelector *timezone;
};

#endif
