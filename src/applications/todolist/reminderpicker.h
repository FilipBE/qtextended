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

#ifndef REMINDERPICKER_H
#define REMINDERPICKER_H

#include <QDateTime>
#include <QWidget>
#include <QString>
#include <QList>

#include "qappointment.h"

class QFormLayout;
class QComboBox;
class QTimeEdit;
class QLabel;

class ReminderPicker : public QObject {
    Q_OBJECT;

    public:
        ReminderPicker(QObject *parent, QFormLayout *f, QAppointment& appt);

        static QString formatReminder(bool allDay, QAppointment::AlarmFlags flag, int minutes);

    public slots:
        void updateUI(bool enable=true);

    private slots:
        void reminderChanged(int index);
        void reminderDelayChanged(int index);
        void reminderTimeChanged(const QTime& time);
        void updateReminderMinutes();

    private:
        void initCB(bool force);
        void splitReminderMinutes(int& dayminutes, int &timeminutes);
        class ReminderEntry{
        public:
            int minutes;
            QString label;
            typedef enum {AllDay = 0x01, NotAllDay = 0x02, Both = 0x03} entry_type;
            entry_type type;

            ReminderEntry(entry_type allday, int m, const QString& l) : minutes(m), label(l),type(allday) {}
            ReminderEntry() : minutes(-1), type(Both) {}
            bool isOther() const {return (minutes == -1);}
        };

        static QList < ReminderEntry > reminderEntries;
        static bool listInited;
        QComboBox *comboReminder;
        QComboBox *comboReminderDelay;
        QTimeEdit *timeEdit;
        QLabel *timeLabel;
        bool mAllDay;

        QAppointment& mAppointment;
};

#endif
