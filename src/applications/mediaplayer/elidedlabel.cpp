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

#include "elidedlabel.h"

ElidedLabel::ElidedLabel( QWidget* parent )
    : QLabel( parent )
{
    setSizePolicy( QSizePolicy::Ignored, QSizePolicy::Minimum );
}

void ElidedLabel::resizeEvent( QResizeEvent* e )
{
    elideText();

    QLabel::resizeEvent( e );
}

void ElidedLabel::setText( const QString& text )
{
    m_text = text;
    elideText();
}

void ElidedLabel::elideText()
{
    QLabel::setText( fontMetrics().elidedText( m_text, Qt::ElideRight, width() ) );
}
