/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** WARNING: Use of this file may require additional third party patent licensing.
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

#include "wordpicker.h"
#include <qtopiaipcenvelope.h>
#include <qsettings.h>
#include <qtopiaapplication.h>
#include <qapplication.h>
#include <qtimer.h>
#include <qstyle.h>
#include <QPainter>

WordPicker::WordPicker(QWidget *parent)
    : Picker(parent)
{
    connect(this, SIGNAL(cellClicked(int,int)), this, SLOT(sendWordChoice(int,int)));
}

WordPicker::~WordPicker()
{
}

void WordPicker::setChoices(const QStringList &list)
{
    choices = list;
    if (choices.count() < 1 && isVisible())
	hide();
    setNumRows(choices.count());
    // work out column width.
    int cw=1;
    QFontMetrics fm(font());
    for ( QStringList::Iterator it = choices.begin(); it != choices.end(); ++it ) {
	cw = qMax(fm.width(*it), cw);
    }
    setCellWidth(cw+4);
    setCellHeight( fm.lineSpacing() + 3 );
}

void WordPicker::sendWordChoice(int row, int)
{
    emit wordChosen(choices[row]);
}

void WordPicker::drawCell(QPainter *p, int row, int, bool selected)
{
    if ( selected ) {
	p->setPen( palette().highlightedText().color() );
	p->fillRect( 0, 0, cellWidth(), cellHeight(), palette().highlight() );
    } else {
	p->setPen( palette().text().color() );
	p->fillRect( 0, 0, cellWidth(), cellHeight(), palette().base() );
    }
    p->drawText(2, 0, cellWidth()-4, cellHeight(), Qt::AlignCenter, choices[row]);
}

