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

#include "slider.h"

#include <qsoftmenubar.h>
#include <qtopianamespace.h>
#include <QKeyEvent>

Slider::Slider( int min, int max, int step, int value, QWidget* parent )
    : QSlider( Qt::Horizontal, parent )
{
    setMinimum( min );
    setMaximum( max );
    setPageStep( step );
    setValue( value );
    // Disable context menu
    QSoftMenuBar::menuFor( this );
}

void Slider::keyPressEvent(QKeyEvent* e)
{
    switch (e->key()) {
    case Qt::Key_Select:
    case Qt::Key_Back:
        QSlider::keyPressEvent( e );

        emit selected();
    case Qt::Key_Up:
    case Qt::Key_Down:
        e->accept();
        break;
    default:
        QSlider::keyPressEvent( e );
    }
}

void Slider::focusOutEvent(QFocusEvent *event)
{
    switch (event->reason()) {
    case Qt::MouseFocusReason:
    case Qt::TabFocusReason:
        hide();
    default:
        QSlider::focusOutEvent(event);
    }
}
