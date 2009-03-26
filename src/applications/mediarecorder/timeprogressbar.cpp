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

#include "timeprogressbar.h"


#include <qapplication.h>
#include <QStylePainter>
#include <QStyleOptionProgressBar>

TimeProgressBar::TimeProgressBar( QWidget *parent )
    : QProgressBar( parent ), prevValue( -1 )
{
    setAlignment( Qt::AlignCenter );
    recording = true;
    refreshPalettes();
}


TimeProgressBar::~TimeProgressBar()
{
}


void TimeProgressBar::setRecording()
{
    recording = true;
    setPalette( adjustedPalette );
}


void TimeProgressBar::setPlaying()
{
    recording = false;
    setPalette( origPalette );
}

void TimeProgressBar::paintEvent( QPaintEvent * )
{
    QStylePainter paint(this);
    QStyleOptionProgressBarV2 option;
    initStyleOption(&option);

    if( option.progress >= 0 )
    {
        if( option.progress > 60 * 60 )
            option.text = QString( "%1:%2:%3" )
                    .arg( option.progress / (60 * 60) )
                    .arg( (option.progress / 60) % 60, 2, 10, QLatin1Char( '0' ) )
                    .arg( option.progress % 60, 2, 10, QLatin1Char( '0' ) );
        else
            option.text = QString( "%1:%2" )
                    .arg( option.progress / 60 )
                    .arg( option.progress % 60, 2, 10, QLatin1Char( '0' ) );
    }
    else
        option.progress = 0;

    if( recording )
        option.maximum = -1;

    paint.drawControl(QStyle::CE_ProgressBar, option);
}

bool TimeProgressBar::event( QEvent *e )
{
    if ( e->type() == QEvent::ApplicationPaletteChange ) {
        refreshPalettes();
    }
    return QProgressBar::event( e );
}


void TimeProgressBar::refreshPalettes()
{
    origPalette = QApplication::palette( this );
    adjustedPalette = origPalette;
    adjustedPalette.setColor
            ( QPalette::Active,
              QPalette::Highlight,
              origPalette.color( QPalette::Active, QPalette::Base ) );
    adjustedPalette.setColor
            ( QPalette::Active,
              QPalette::HighlightedText,
              origPalette.color( QPalette::Active, QPalette::Text ) );
    adjustedPalette.setColor
            ( QPalette::Inactive,
              QPalette::Highlight,
              origPalette.color( QPalette::Active, QPalette::Base ) );
    adjustedPalette.setColor
            ( QPalette::Inactive,
              QPalette::HighlightedText,
              origPalette.color( QPalette::Active, QPalette::Text ) );
    if ( recording )
        setPalette( adjustedPalette );
    else
        setPalette( origPalette );
}

