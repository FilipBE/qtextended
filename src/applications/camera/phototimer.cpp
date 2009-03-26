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

#include "phototimer.h"
#include "minsecspinbox.h"
#include "noeditspinbox.h"

// Qt includes
#include <QTimer>
#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QLCDNumber>

// Qtopia includes
#include <QAnalogClock>
#include <QtopiaApplication>

PhotoTimer::PhotoTimer( QWidget* parent,
                        Qt::WFlags f )
:   QWidget( parent, f ),
    mClock(0)
{
    QGridLayout* layout = new QGridLayout( this );

    mClock = new QAnalogClock( this );
    mClock->display( QTime( 0, mTimeout, mTimeout ) );
    mClock->setFace( QPixmap( ":image/clock/background" ) );
    layout->addWidget( mClock, 0, 0 );
    setLayout( layout );

    setMinimumSize( 70, 70 );
    setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));

    timer = new QTimer( this );
    timer->setInterval( 1000 );
    connect( timer, SIGNAL(timeout()), this, SLOT(timeout()) );
    hide();
}

void PhotoTimer::start(int timeout, int number, int interval)
{
    mTimeout  = timeout;
    mNumber   = number;
    mInterval = interval;

    timer->start();
    show();
}

void PhotoTimer::timeout()
{
    if ( mTimeout <= 0 ) {
        emit( takePhoto() );
        if ( mNumber <= 1 ) {
            timer->stop();
            hide();
        } else {
            mTimeout = mInterval;
            --mNumber;
        }
    } else {
        --mTimeout;
    }

    mClock->display( QTime( 0, mTimeout, mTimeout ) );
}

void PhotoTimer::cancelTimer()
{
    timer->stop();
    hide();
}

PhotoTimerDialog::PhotoTimerDialog( QWidget* parent, Qt::WFlags f )
:   QDialog( parent, f ),
    mTimeout( 5 ),
    mNumber( 1 ),
    mInterval( 1 ),
    mIntervalSpin( 0 )
{
    setWindowTitle( tr( "Photo Timer" ) );
    setModal( true);

    QGridLayout* layout = new QGridLayout( this );

    // Add labels
    layout->addWidget( new QLabel( tr( "Timeout" ) ), 0, 0 );
    QSpinBox* timeout = new CameraMinSecSpinBox( this );
    timeout->setMinimum( 1 );
    timeout->setMaximum( 120 );
    timeout->setValue( mTimeout );
    layout->addWidget( timeout, 0, 1 );
    connect( timeout,
             SIGNAL(valueChanged(int)),
             this,
             SLOT(timeoutChanged(int)) );

    //layout->addWidget( new QLabel( tr( "Photos" ) ),  1, 0 );
    QSpinBox* number = new NoEditSpinBox(this);
    number->setMinimum( 1 );
    number->setMaximum( 50 );
    number->setValue( mNumber );

    //TODO: Disable for now
    //layout->addWidget( number, 1, 1 );
    number->setEnabled(false);
    number->hide();

    connect( number,
             SIGNAL(valueChanged(int)),
             this,
             SLOT(numberChanged(int)) );

    //layout->addWidget( new QLabel( tr( "Interval" ) ), 2, 0 );
    mIntervalSpin = new CameraMinSecSpinBox( this );
    mIntervalSpin->setMinimum( 1 );
    mIntervalSpin->setMaximum( 120 );
    mIntervalSpin->setValue( mInterval );
    mIntervalSpin->setEnabled( false );

    //TODO: Disable for now
    //layout->addWidget( mIntervalSpin, 2, 1 );
    mIntervalSpin->hide();

    connect( mIntervalSpin,
             SIGNAL(valueChanged(int)),
             this,
             SLOT(intervalChanged(int)) );

    setLayout( layout );

    QtopiaApplication::setInputMethodHint(timeout,QtopiaApplication::AlwaysOff);
    QtopiaApplication::setInputMethodHint(number,QtopiaApplication::AlwaysOff);
    QtopiaApplication::setInputMethodHint(mIntervalSpin,QtopiaApplication::AlwaysOff);

}

int PhotoTimerDialog::timeout() const
{
    return mTimeout;
}

int PhotoTimerDialog::number() const
{
    return mNumber;
}

int PhotoTimerDialog::interval() const
{
    return mInterval;
}

void PhotoTimerDialog::timeoutChanged( int timeout )
{
    mTimeout = timeout;
}

void PhotoTimerDialog::numberChanged( int number )
{
    mNumber = number;
    if ( mNumber > 1 )
        mIntervalSpin->setEnabled( true );
    else
        mIntervalSpin->setEnabled( false );
}

void PhotoTimerDialog::intervalChanged( int interval )
{
    mInterval = interval;
}



