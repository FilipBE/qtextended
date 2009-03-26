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

#include "recurrencedetails.h"

#include <QDebug>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QGroupBox>
#include <QGridLayout>
#include <QTimeString>
#include <QCheckBox>
#include <QSpinBox>
#include <QScrollArea>
#include <QRadioButton>
#include <QLabel>


// Combobox indices
enum {RT_None = 0, RT_Daily, RT_Weekly, RT_Monthly, RT_Yearly};

RecurrenceDetails::RecurrenceDetails(QAppointment& appt, QObject *parent )
    : QObject ( parent ),
    mGuiInited(false),
    mAppointment(appt),
    startMonday(Qtopia::weekStartsOnMonday())
{
}


static inline void increment(Qt::DayOfWeek &day)
{
    switch(day)
    {
        case Qt::Monday:
            day = Qt::Tuesday;
            break;
        case Qt::Tuesday:
            day = Qt::Wednesday;
            break;
        case Qt::Wednesday:
            day = Qt::Thursday;
            break;
        case Qt::Thursday:
            day = Qt::Friday;
            break;
        case Qt::Friday:
            day = Qt::Saturday;
            break;
        case Qt::Saturday:
            day = Qt::Sunday;
            break;
        case Qt::Sunday:
            day = Qt::Monday;
            break;
    }
}

void RecurrenceDetails::refreshLabels()
{
    if (mGuiInited) {
        Qt::DayOfWeek startDay = startMonday ? Qt::Monday : Qt::Sunday;
        Qt::DayOfWeek day = startDay;
        int idx = 0;

        do {
            weekDayToggle[idx++]->setText(QTimeString::nameOfWeekDay(day, QTimeString::Long));
            increment(day);
        } while (day != startDay);

        //  Update the 'monthy' radio button text
        QDate date = mAppointment.start().date();

        dayOfMonth->setText(tr("Day %1 of the month", "eg. Day 3 of the month").arg(date.day()));

        weekDayOfMonth->setText(tr("The %1 %2", "eg. The second Monday (of the month)")
            .arg(trSmallOrdinal(((date.day() - 1) / 7) + 1))
            .arg(QTimeString::localDayOfWeek(date, QTimeString::Long)));

        lastWeekDayOfMonth->setText(tr("The %1 %2\nfrom the end",
            "eg. The third Tuesday from the end (of the month)")
            .arg(trSmallOrdinal(((date.daysInMonth() - date.day()) / 7) + 1))
            .arg(QTimeString::localDayOfWeek(date, QTimeString::Long)));

    }
}

void RecurrenceDetails::repeatTypeChanged()
{
    switch (repeatCB->currentIndex())
    {
        case RT_None:
            weeklyGB->hide();
            monthlyGB->hide();
            break;

        case RT_Daily:
        case RT_Yearly:
            weeklyGB->hide();
            monthlyGB->hide();
            break;

        case RT_Weekly:
            weeklyGB->show();
            monthlyGB->hide();
            break;

        case RT_Monthly:
            weeklyGB->hide();
            monthlyGB->show();
            break;
    }

    bool enable = (repeatCB->currentIndex() != RT_None);
    repEveryL->setEnabled(enable);
    repEverySB->setEnabled(enable);
    untilGB->setEnabled(enable);
    updateAppointment();
}

QString RecurrenceDetails::trSmallOrdinal(int n) const
{
    if ( n == 1 ) return tr("first", "eg. first Friday of month");
    else if ( n == 2 ) return tr("second", "eg. second Friday of month");
    else if ( n == 3 ) return tr("third", "eg. third Friday of month");
    else if ( n == 4 ) return tr("fourth", "eg. fourth Friday of month");
    else if ( n == 5 ) return tr("fifth", "eg. fifth Friday of month");
    else return QString::number(n);
}

void RecurrenceDetails::initGui(QFormLayout *parentlayout)
{
    if (!mGuiInited) {
        repeatFieldsW = new QWidget();
        untilDate = new QDateEdit();

        repeatCB = new QComboBox();
        repeatCB->addItem(tr("None"));
        repeatCB->addItem(tr("Daily"));
        repeatCB->addItem(tr("Weekly"));
        repeatCB->addItem(tr("Monthly"));
        repeatCB->addItem(tr("Yearly"));
        parentlayout->addRow(tr("Repeat"), repeatCB);

        repEverySB = new QSpinBox();
        repEverySB->setMinimum(1);
        repEverySB->setValue(1);
        repEveryL = new  QLabel(tr("Frequency"));
        repEveryL->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        parentlayout->addRow(repEveryL, repEverySB);

        // until date
        untilGB = new QGroupBox();
        untilGB->setTitle(tr("Repeat until"));
        untilGB->setCheckable(true);
        QFormLayout *ufl = new QFormLayout();
        ufl->addRow(tr("Until"), untilDate);
        untilGB->setLayout(ufl);

        parentlayout->addRow(untilGB);

        // Daily doesn't get anything.

        // Weekly
        weeklyGB = new QGroupBox();
        QGridLayout *gl = new QGridLayout();
        weeklyGB->setTitle(tr("On these days"));
        for (int day=0; day < 7; day++ ) {
            weekDayToggle.append(new QCheckBox(""));
            gl->addWidget(weekDayToggle[day], day, 0);
        }
        weeklyGB->setLayout(gl);

        parentlayout->addRow(weeklyGB);

        // monthly
        monthlyGB = new QGroupBox();
        monthlyGB->setTitle(tr("Repeat on", "Repeat on The second Monday"));
        gl = new QGridLayout();
        gl->addWidget((dayOfMonth = new QRadioButton(tr("Day XX of the month"))));
        gl->addWidget((weekDayOfMonth = new QRadioButton(tr("The second Monday"))));
        gl->addWidget((lastWeekDayOfMonth = new QRadioButton(tr("The third last Monday"))));

        monthlyGB->setLayout(gl);

        parentlayout->addRow(monthlyGB);

        // Yearly doesn't get anything either

        connect(repeatCB, SIGNAL(currentIndexChanged(int)), this, SLOT(repeatTypeChanged()));
        connect(untilGB, SIGNAL(clicked()), this, SLOT(updateRepeatUntil()));

        mGuiInited = true;
    }
    updateUI();
}

void RecurrenceDetails::updateUI()
{
    if (mGuiInited ) {
        startMonday = Qtopia::weekStartsOnMonday();

        if (mAppointment.repeatRule() == QAppointment::NoRepeat) {
            repEverySB->setValue(1);
        } else {
            repEverySB->setValue(mAppointment.frequency());
        }

        int i;
        Qt::DayOfWeek day = startMonday ? Qt::Monday: Qt::Sunday;
        for (i = 0; i < 7; i++) {
            QAbstractButton *tb = weekDayToggle[i];

            if (mAppointment.start().date().dayOfWeek() == day) {
                tb->setEnabled(false);
                tb->setChecked(true);
            } else {
                tb->setEnabled(true);
                tb->setChecked(mAppointment.repeatOnWeekDay(day));
            }

            increment(day);
        }

        // Default
        dayOfMonth->setChecked(true);
        repeatCB->blockSignals(true);
        switch(mAppointment.repeatRule()) {
            default:
            case QAppointment::NoRepeat:
                repeatCB->setCurrentIndex(RT_None);
                break;
            case QAppointment::Daily:
                repeatCB->setCurrentIndex(RT_Daily);
                break;
            case QAppointment::Weekly:
                repeatCB->setCurrentIndex(RT_Weekly);
                break;
            case QAppointment::MonthlyDate:
                repeatCB->setCurrentIndex(RT_Monthly);
                break;
            case QAppointment::MonthlyDay:
                repeatCB->setCurrentIndex(RT_Monthly);
                weekDayOfMonth->setChecked(true);
                break;
            case QAppointment::MonthlyEndDay:
                repeatCB->setCurrentIndex(RT_Monthly);
                lastWeekDayOfMonth->setChecked(true);
                break;
            case QAppointment::Yearly:
                repeatCB->setCurrentIndex(RT_Yearly);
                break;
        }
        repeatCB->blockSignals(false);

        untilDate->setDate(mAppointment.repeatUntil());
        if (!mAppointment.hasRepeat() || mAppointment.repeatForever())
            untilGB->setChecked(false);
        else
            untilGB->setChecked(true);

        refreshLabels();
        repeatTypeChanged();
    }
}

void RecurrenceDetails::updateRepeatUntil()
{
    if (untilGB->isChecked()) {
        mAppointment.setRepeatUntil(untilDate->date());
        untilDate->setDate(mAppointment.repeatUntil());
    }
}

bool RecurrenceDetails::eventFilter( QObject *receiver, QEvent *event )
{
    if (event->type() == QEvent::Resize) {
        if( scrollArea && scrollArea->widget() && receiver == scrollArea->viewport() )
            scrollArea->widget()->setFixedWidth( scrollArea->viewport()->width() );
    }
    return false;
}

void RecurrenceDetails::updateAppointment()
{
    if (mGuiInited) {
        int i;
        switch(repeatCB->currentIndex()) {
            case RT_None:
                mAppointment.setRepeatRule(QAppointment::NoRepeat);
                break;
            case RT_Daily:
                mAppointment.setRepeatRule(QAppointment::Daily);
                break;
            case RT_Weekly:
                mAppointment.setRepeatRule(QAppointment::Weekly);

                {
                    Qt::DayOfWeek day = startMonday ? Qt::Monday: Qt::Sunday;
                    for (i = 0; i < 7; i++) {
                        QAbstractButton *tb = weekDayToggle[i];
                        mAppointment.setRepeatOnWeekDay(day, tb->isChecked());

                        increment(day);
                    }
                }
                break;
            case RT_Monthly:
                if (dayOfMonth->isChecked())
                    mAppointment.setRepeatRule(QAppointment::MonthlyDate);
                else if (weekDayOfMonth->isChecked())
                    mAppointment.setRepeatRule(QAppointment::MonthlyDay);
                else
                    mAppointment.setRepeatRule(QAppointment::MonthlyEndDay);
                break;
            case RT_Yearly:
                mAppointment.setRepeatRule(QAppointment::Yearly);
                break;
        }
        mAppointment.setFrequency(repEverySB->value());

        if (untilGB->isChecked()) {
            mAppointment.setRepeatUntil(untilDate->date());
            untilDate->setDate(mAppointment.repeatUntil());
        } else {
            mAppointment.setRepeatForever();
        }
    }
}

RecurrenceDetails::~RecurrenceDetails()
{
}

