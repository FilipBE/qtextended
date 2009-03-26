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

#include "appointmentpicker.h"
#include "datebook.h"
#include "dayview.h"
#include "monthview.h"

#include <qappointment.h>

#include <QAction>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QtopiaApplication>
#include <QSoftMenuBar>

AppointmentPicker::AppointmentPicker( DateBook *db, QSet<QPimSource> sources, QWidget *parent, Qt::WFlags f )
    : QDialog( parent, f ),
    datebook( db ),
    mSources(sources)
{
    setWindowTitle( tr("Appointment Picker") );
    setWindowIcon( QPixmap( ":image/AppointmentPicker" ) );

    views = new QStackedWidget( this );
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( views );
    layout->setMargin(0);

    dayView = 0;
    monthView = 0;

    QtopiaApplication::setMenuLike(this, true);
    lastToday = QDate::currentDate();
    viewMonth( lastToday );
}

AppointmentPicker::~AppointmentPicker()
{
}

void AppointmentPicker::viewToday()
{
    lastToday = QDate::currentDate();
    viewDay( lastToday );
}

void AppointmentPicker::viewDay()
{
    viewDay( currentDate() );
}

void AppointmentPicker::viewDay(const QDate& dt)
{
    initDay();
    dayView->selectDate( dt );
    dayView->firstTimed();
    views->setCurrentIndex( views->indexOf( dayView ) );
}

void AppointmentPicker::viewMonth()
{
    viewMonth( currentDate() );
}

void AppointmentPicker::viewMonth( const QDate& dt)
{
    initMonth();
    monthView->setSelectedDate( dt );
    views->setCurrentIndex( views->indexOf( monthView ) );
}

void AppointmentPicker::viewMonthAgain()
{
    viewMonth(dayView->currentDate());
}

bool AppointmentPicker::appointmentSelected() const
{
    if (views->currentWidget() && views->currentWidget() == dayView) {
        return dayView->currentIndex().isValid();
    }
    return false;
}

QAppointment AppointmentPicker::currentAppointment() const
{
    return dayView->currentAppointment();
}

QDate AppointmentPicker::currentDate()
{
    if ( dayView && views->currentWidget() == dayView ) {
        return dayView->currentDate();
    } else if ( monthView && views->currentWidget() == monthView ) {
        return monthView->selectedDate();
    } else {
        return QDate(); // invalid;
    }
}

void AppointmentPicker::initDay()
{
    if ( !dayView ) {
        dayView = new DayView(0, QCategoryFilter(), mSources);
        views->addWidget( dayView );
        int endTime = qMin(qMax(datebook->startTime + 8, 17), 24);
        dayView->setDaySpan( datebook->startTime, endTime );
        QSoftMenuBar::setLabel(dayView, Qt::Key_Back, QSoftMenuBar::Back);
        connect( dayView, SIGNAL(showDetails()),
                this, SLOT(accept()) );
        connect( dayView, SIGNAL(closeView()),
                this, SLOT(viewMonthAgain()) );
    }
}

void AppointmentPicker::initMonth()
{
    if ( !monthView ) {
        monthView = new MonthView(0, QCategoryFilter(), mSources);
        monthView->setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
        QSoftMenuBar::setLabel(monthView, Qt::Key_Back, QSoftMenuBar::Cancel);
        // TODO monthView->setMargin(0);
        views->addWidget( monthView );
        connect( monthView, SIGNAL(activated(QDate)),
             this, SLOT(viewDay(QDate)) );
        connect( monthView, SIGNAL(closeView()), this, SLOT(reject()));
    }
}

void AppointmentPicker::nextView()
{
    QWidget* cur = views->currentWidget();
    if ( cur ) {
        if ( cur == dayView )
            viewMonth();
        else if ( cur == monthView )
            viewDay();
    }
}

