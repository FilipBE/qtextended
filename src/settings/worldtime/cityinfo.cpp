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

#include "cityinfo.h"

#include <qtimestring.h>
#include <qtimezone.h>
#include <qtopialog.h>

#include <QStyleOption>
#include <QPalette>
#include <QPainter>

CityInfo::CityInfo(QWidget *parent, Qt::WFlags f)
    : QFrame(parent,f)
{
    mUtc = QTimeZone::utcDateTime();
}

void CityInfo::setZone(const QString &zone)
{
    mZone = zone;
        mZone.toLocal8Bit().constData();
    repaint();
}

void CityInfo::setUtcTime(const QDateTime &dt)
{
    mUtc = dt;
    repaint();
}

QString CityInfo::text() const
{
    QString line;
    QDateTime cityTime;

    if ( !mZone.isNull() ) {
        QTimeZone curZone( mZone.toLocal8Bit().constData() );
        if ( curZone.isValid() )
            cityTime = curZone.fromUtc(mUtc);
        line = QTimeString::localHMDayOfWeek(cityTime).simplified();
        return line;
    }
    else {
        return QString();
    }
}

void CityInfo::paintEvent( QPaintEvent * )
{
    QPainter p(this);
    QStyleOptionHeader opt;

    opt.palette = palette();
    opt.state = QStyle::State_Enabled;
    opt.state |= QStyle::State_Horizontal;

    QRect cr = contentsRect();
    style()->drawItemText( &p, cr, Qt::AlignHCenter, opt.palette,
            opt.state, text(), QPalette::ButtonText);

    drawFrame(&p);
}


QSize CityInfo::sizeHint() const
{
    QSize res;
    QFontMetrics fm( font() );
    res.setWidth(fm.width(text()) + 2 );
    res.setHeight(fm.height() );
    return res;
}
