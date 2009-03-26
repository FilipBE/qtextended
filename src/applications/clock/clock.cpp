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

#include "clock.h"

#include <qtopiaapplication.h>
#include <qtopiaipcenvelope.h>
#include <qsettings.h>
#include <qtimestring.h>
#include <qtopia/qsoftmenubar.h>
#include <qlcdnumber.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qpainter.h>
#include <qmessagebox.h>
#include <QDesktopWidget>
#include <QAnalogClock>
#include <QSettings>

Clock::Clock(QWidget *parent, Qt::WFlags f)
    : QWidget(parent, f)
{
    setupUi(this);
    QSettings config("Trolltech","qpe");
    config.beginGroup("Time");
    ampm = config.value( "AMPM" ).toBool();

    if (!ampm)
        clockAmPm->hide();

    analogClock->display( QTime::currentTime() );
    analogClock->setFace( QPixmap( ":image/background" ) );
    QPalette p = analogClock->palette();
    p.setColor(QPalette::Text, Qt::black);
    analogClock->setPalette(p);
    clockLcd->setNumDigits( 5 );
    //clockLcd->setFixedWidth( clockLcd->sizeHint().width() );
    clockLcd->setFixedWidth((int) ( 0.325f*(float)physicalDpiX()));
    clockLcd->setFixedHeight((int)( 0.125f*(float)physicalDpiY()));
    QSoftMenuBar::setLabel(clockLcd, Qt::Key_Select, QSoftMenuBar::NoLabel);
    QSoftMenuBar::setLabel(this, Qt::Key_Select, QSoftMenuBar::NoLabel);

    getDateFormat();

    t = new QTimer( this );
    connect( t, SIGNAL(timeout()), SLOT(updateClock()) );

    connect( qApp, SIGNAL(dateFormatChanged()), SLOT(updateDateFormat()) );
    connect( qApp, SIGNAL(timeChanged()), SLOT(updateClock()) );
    connect( qApp, SIGNAL(clockChanged(bool)), this, SLOT(changeClock(bool)) );

    QTimer::singleShot( 0, this, SLOT(updateClock()) );
}

Clock::~Clock()
{
}

void Clock::updateClock()
{
    QTime tm = QDateTime::currentDateTime().time();
    QString s;
    if ( ampm ) {
        int hour = tm.hour();
        if (hour == 0)
            hour = 12;
        if (hour > 12)
            hour -= 12;
        s.sprintf( "%2d%c%02d", hour, ':', tm.minute() );
        clockAmPm->setText( (tm.hour() >= 12) ? "PM" : "AM" );
        clockAmPm->show();
    } else {
        s.sprintf( "%2d%c%02d", tm.hour(), ':', tm.minute() );
        clockAmPm->hide();
    }
    clockLcd->display( s );
    if (isVisible())
        clockLcd->repaint();
    analogClock->display( QTime::currentTime() );

    QString df = "dddd "+dateFormat;
    df.replace(QString("%D"), QString("d"));
    df.replace(QString("%M"), QString("MMMM"));
    df.replace(QString("%Y"), QString("yyyy"));
    df.replace(QString("/"), QString(" "));
    df.replace(QString("-"), QString(" "));
    df.replace(QString("."), QString(" "));
    date->setText( QDateTime::currentDateTime().toString(df) );

}

void Clock::changeClock( bool a )
{
    ampm = a;
    getDateFormat();
    updateClock();
}

void Clock::showEvent(QShowEvent *e)
{
    updateClock();
    t->start( 1000 );
    QWidget::showEvent(e);
}

void Clock::hideEvent(QHideEvent *e)
{
    t->stop();
    QWidget::hideEvent(e);
}

void Clock::updateDateFormat()
{
    getDateFormat();
    updateClock();
}

void Clock::getDateFormat()
{
    QSettings cfg("Trolltech","qpe");
    cfg.beginGroup( "Date" );
    dateFormat = cfg.value("DateFormat").toString();
    if(dateFormat.isEmpty()) dateFormat = "%M/%D/%Y";
}

