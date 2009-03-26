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

#include "datebooksettings.h"
#include <QFormLayout>
#include <QSpinBox>
#include <QGroupBox>
#include <QComboBox>
#include <QApplication>

#include "../todolist/reminderpicker.h"

DateBookSettings::DateBookSettings( bool whichClock, QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ),
      ampm( whichClock ), oldtime(0)
{
    init();
    setObjectName("settings");
    QObject::connect( qApp, SIGNAL(clockChanged(bool)),
                      this, SLOT(slotChangeClock(bool)) );
}

DateBookSettings::~DateBookSettings()
{
}

void DateBookSettings::setStartTime( int newStartViewTime )
{
    if ( ampm ) {
        if ( newStartViewTime >= 12 ) {
            newStartViewTime %= 12;
            if ( newStartViewTime == 0 )
                newStartViewTime = 12;
            spinStart->setSuffix( tr(":00 PM") );
        }
        else if ( newStartViewTime == 0 ) {
            newStartViewTime = 12;
            spinStart->setSuffix( tr(":00 AM") );
        }
        oldtime = newStartViewTime;
    }
    spinStart->setValue( newStartViewTime );
}

int DateBookSettings::startTime() const
{
    int returnMe = spinStart->value();
    if ( ampm ) {
        if ( returnMe != 12 && spinStart->suffix().contains(tr("PM"), Qt::CaseInsensitive) )
            returnMe += 12;
        else if (returnMe == 12 && spinStart->suffix().contains(tr("AM"),  Qt::CaseInsensitive))
            returnMe = 0;
    }
    return returnMe;
}


void DateBookSettings::setPresetAlarm( QAppointment::AlarmFlags alarmType, int presetTime )
{
    mAppt.setAlarm(presetTime, alarmType);
    picker->updateUI();
}

QAppointment::AlarmFlags DateBookSettings::alarmType() const
{
    return mAppt.alarm();
}

int DateBookSettings::alarmDelay() const
{
    return mAppt.alarmDelay();
}

void DateBookSettings::slot12Hour( int i )
{
    if ( ampm ) {
        if ( spinStart->suffix().contains( tr("AM"), Qt::CaseInsensitive ) ) {
            if ( oldtime == 12 && i == 11 || oldtime == 11 && i == 12 )
                spinStart->setSuffix( tr(":00 PM") );
        } else {
            if ( oldtime == 12 && i == 11 || oldtime == 11 && i == 12 )
                spinStart->setSuffix( tr(":00 AM") );
        }
        oldtime = i;
    }
}

void DateBookSettings::init()
{
    setObjectName(QString::fromUtf8("DateBookSettingsBase"));

    QFormLayout *fl = new QFormLayout();
    QFormLayout *gbfl = new QFormLayout();
    fraView = new QGroupBox();
    cmbDefaultView = new QComboBox(fraView);

    spinStart = new QSpinBox(fraView);
    spinStart->setWrapping(true);
    spinStart->setMaximum(23);
    QObject::connect(spinStart,SIGNAL(valueChanged(int)), this,SLOT(slot12Hour(int)));

    fraAlarm = new QGroupBox();

    setWindowTitle(QApplication::translate("DateBookSettingsBase", "Settings", 0, QApplication::UnicodeUTF8));
    fraView->setTitle(QApplication::translate("DateBookSettingsBase", "View", 0, QApplication::UnicodeUTF8));
    gbfl->addRow(QApplication::translate("DateBookSettingsBase", "Default view", 0, QApplication::UnicodeUTF8), cmbDefaultView);

    cmbDefaultView->clear();
    cmbDefaultView->insertItems(0, QStringList()
         << QApplication::translate("DateBookSettingsBase", "Day", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("DateBookSettingsBase", "Month", 0, QApplication::UnicodeUTF8));

    gbfl->addRow(QApplication::translate("DateBookSettingsBase", "Day starts at", 0, QApplication::UnicodeUTF8), spinStart);
    spinStart->setSuffix(QApplication::translate("DateBookSettingsBase", ":00", 0, QApplication::UnicodeUTF8));
    fraAlarm->setTitle(QApplication::translate("DateBookSettingsBase", "Preset", 0, QApplication::UnicodeUTF8));

    fraView->setLayout(gbfl);

    gbfl = new QFormLayout();
    picker = new ReminderPicker(this, gbfl, mAppt);
    fraAlarm->setLayout(gbfl);

    fl->addRow(fraView);
    fl->addRow(fraAlarm);

    setLayout(fl);

    if ( ampm ) {
        spinStart->setMinimum( 1 );
        spinStart->setMaximum( 12 );
        spinStart->setValue( 12 );
        spinStart->setSuffix( tr(":00 AM") );
        oldtime = 12;
    } else {
        spinStart->setMinimum( 0 );
        spinStart->setMaximum( 23 );
        spinStart->setSuffix( tr(":00") );
    }
}

void DateBookSettings::slotChangeClock( bool whichClock )
{
    int saveMe;
    saveMe = spinStart->value();
    if ( ampm && spinStart->suffix().contains( tr("AM"), Qt::CaseInsensitive ) ) {
        if ( saveMe == 12 )
            saveMe = 0;
    } else if ( ampm && spinStart->suffix().contains( tr("PM"), Qt::CaseInsensitive )  ) {
        if ( saveMe != 12 )
            saveMe += 12;
    }
    ampm = whichClock;
    init();
    setStartTime( saveMe );
}

DateBookSettings::ViewType DateBookSettings::defaultView() const
{
    return (DateBookSettings::ViewType) cmbDefaultView->currentIndex();
}

void DateBookSettings::setDefaultView(DateBookSettings::ViewType type)
{
    // If we had holes in the range we'd need to translate
    if (type >= 0 && type < cmbDefaultView->count())
        cmbDefaultView->setCurrentIndex(type);
}

