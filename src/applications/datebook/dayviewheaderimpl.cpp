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
#include "dayviewheaderimpl.h"

#include <qtimestring.h>
#include <qtoolbutton.h>
#include <qdatetime.h>
#include <qlayout.h>
#include <QFrame>
#include <QButtonGroup>
#include <QDateTimeEdit>

/*
    Constructs a DateBookDayHeader which is a child of 'parent', with the
    name 'name' and widget flags set to 'f'

    The dialog will by default be modeless, unless you set 'modal' to
    true to construct a modal dialog.
*/
DayViewHeader::DayViewHeader( bool useMonday, QWidget* parent )
    : QWidget( parent ),
    bUseMonday( useMonday )
{
    init();

    setBackgroundRole( QPalette::Button );
    fraDays->setBackgroundRole( QPalette::Button );

    dButton->setDate( currDate );
    connect(dButton,SIGNAL(dateChanged(QDate)),this,SIGNAL(dateChanged(QDate)));
}

/*
    Destroys the object and frees any allocated resources
*/
DayViewHeader::~DayViewHeader()
{
    // no need to delete child widgets, Qt does it all for us
}

void DayViewHeader::init()
{
    back = new QToolButton(this);
    back->setIcon(QIcon(":icon/i18n/previous"));
    back->setAutoRepeat( true );
    back->setAutoRaise( true );
    connect( back, SIGNAL(clicked()), this, SLOT(goBack()) );

    forward = new QToolButton(this);
    forward->setIcon(QIcon(":icon/i18n/next"));
    forward->setAutoRepeat( true );
    forward->setAutoRaise( true );
    connect( forward, SIGNAL(clicked()), this, SLOT(goForward()) );

    fraDays = new QFrame(this);
    grpDays = new QButtonGroup(this);
    grpDays->setExclusive( true );
    connect( grpDays, SIGNAL(clicked(int)), this, SLOT(setDay(int)) );

    cmdDay1 = new QToolButton(fraDays);
    cmdDay2 = new QToolButton(fraDays);
    cmdDay3 = new QToolButton(fraDays);
    cmdDay4 = new QToolButton(fraDays);
    cmdDay5 = new QToolButton(fraDays);
    cmdDay6 = new QToolButton(fraDays);
    cmdDay7 = new QToolButton(fraDays);
    QToolButton *cmdDays[7] = { cmdDay1, cmdDay2, cmdDay3, cmdDay4, cmdDay5, cmdDay6, cmdDay7 };
    for ( int i = 0; i < 7; i++ ) {
        cmdDays[i]->setAutoRaise( true );
        cmdDays[i]->setCheckable( true );
    }

    setupNames();

    dButton = new QDateEdit(this);

    QHBoxLayout *hbox = new QHBoxLayout( this );
    hbox->addWidget(back);
    hbox->addWidget(fraDays);
    hbox->addWidget(forward);
    hbox->addWidget(dButton);

    hbox = new QHBoxLayout( fraDays );
    hbox->addWidget( cmdDay1 );
    hbox->addWidget( cmdDay2 );
    hbox->addWidget( cmdDay3 );
    hbox->addWidget( cmdDay4 );
    hbox->addWidget( cmdDay5 );
    hbox->addWidget( cmdDay6 );
    hbox->addWidget( cmdDay7 );
}

void DayViewHeader::setStartOfWeek( bool onMonday )
{
    bUseMonday = onMonday;
    setupNames();
    setDate( currDate.year(), currDate.month(), currDate.day() );
}

static void setButton( QAbstractButton *btn, int day )
{
    btn->setText( QTimeString::nameOfWeekDay( day + 1, QTimeString::Short ) );
}

void DayViewHeader::setupNames()
{
    int i = 0;
    ::setButton( cmdDay1, (bUseMonday?i:(i+6)%7) ); i++;
    ::setButton( cmdDay2, (bUseMonday?i:(i+6)%7) ); i++;
    ::setButton( cmdDay3, (bUseMonday?i:(i+6)%7) ); i++;
    ::setButton( cmdDay4, (bUseMonday?i:(i+6)%7) ); i++;
    ::setButton( cmdDay5, (bUseMonday?i:(i+6)%7) ); i++;
    ::setButton( cmdDay6, (bUseMonday?i:(i+6)%7) ); i++;
    ::setButton( cmdDay7, (bUseMonday?i:(i+6)%7) );
}

/*
    public slot
*/
void DayViewHeader::goBack()
{
    currDate = currDate.addDays( -7 );
    setDate( currDate.year(), currDate.month(), currDate.day() );
}
/*
    public slot
*/
void DayViewHeader::goForward()
{
    currDate = currDate.addDays( 7 );
    setDate( currDate.year(), currDate.month(), currDate.day() );
}


/*
    public slot
*/
void DayViewHeader::setDate( int y, int m, int d )
{
    currDate.setYMD( y, m, d );
    dButton->setDate(QDate(y,m,d));

    int iDayOfWeek = currDate.dayOfWeek();
    // cleverly adjust the day depending on how we start the week
    if ( bUseMonday )
        iDayOfWeek--;
    else {
        if ( iDayOfWeek == 7 )  // Sunday
            iDayOfWeek = 0;
    }
    QAbstractButton *btn = qobject_cast<QAbstractButton*>(grpDays->children()[iDayOfWeek]);
    if ( btn )
        btn->setChecked( true );
    emit dateChanged( currDate );
}

/*
    public slot
*/
void DayViewHeader::setDay( int day )
{
    int realDay;
    int dayOfWeek = currDate.dayOfWeek();

    // a little adjustment is needed...
    if ( bUseMonday )
        realDay = day + 1 ;
    else if ( !bUseMonday && day == 0 ) // sunday
        realDay = 7;
    else
        realDay = day;
    // special cases first...
    if ( realDay == 7 && !bUseMonday )  {
        while ( currDate.dayOfWeek() != realDay )
            currDate = currDate.addDays( -1 );
    } else if ( !bUseMonday && dayOfWeek == 7 && dayOfWeek > realDay ) {
        while ( currDate.dayOfWeek() != realDay )
            currDate = currDate.addDays( 1 );
    } else if ( dayOfWeek < realDay ) {
        while ( currDate.dayOfWeek() < realDay )
            currDate = currDate.addDays( 1 );
    } else if ( dayOfWeek > realDay ) {
        while ( currDate.dayOfWeek() > realDay )
            currDate = currDate.addDays( -1 );
    }
    // update the date...
    setDate( currDate.year(), currDate.month(), currDate.day() );
}
