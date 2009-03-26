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

#include "slideshowui.h"
#include "imageviewer.h"

#include <qthumbnail.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qfontmetrics.h>
#include <QStyleOption>

#include <QKeyEvent>

SlideShowUI::SlideShowUI(ImageScaler *scaler, QWidget* parent, Qt::WFlags f )
    : QWidget( parent, f ), m_scaler(scaler), display_name( false )
{
    setAttribute(Qt::WA_NoSystemBackground);

    connect(scaler, SIGNAL(imageChanged()), this, SLOT(imageChanged()));
}

void SlideShowUI::imageChanged()
{
    image = m_scaler->content();

    update();
}

void SlideShowUI::paintEvent( QPaintEvent * )
{
#define NAME_POSX 2
#define NAME_POSY 2
#define NAME_COLOR QColor( 162, 190, 0 )
#define SHADOW_OFFSET 1

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    painter.fillRect( rect(), Qt::black );

    QImage image = m_scaler->image(size());

    if (!image.isNull()) {
        QSize imageSize = image.size();

        imageSize.scale(size(), Qt::KeepAspectRatio);

        QRect destination(QPoint(0, 0), imageSize);

        destination.moveCenter(geometry().center());

        painter.drawImage(destination, image);
    }

    // If display name, draw fitted name onto widget
    if( display_name ) {
        QStyleOption style;
        style.initFrom( this );

        QString name = m_scaler->content().name();

        int x = style.direction == Qt::LeftToRight
                ? NAME_POSX + SHADOW_OFFSET
                : 0;
        int y = NAME_POSY + SHADOW_OFFSET;
        int w = width() - NAME_POSX - SHADOW_OFFSET;
        int h = height() - y;

        QRect rect( x, y, w, h );

        QString elidedName = style.fontMetrics.elidedText( name, Qt::ElideRight, w );

        // Draw shadow
        painter.setPen( Qt::black );
        painter.drawText( rect, elidedName );
        // Draw fitted name
        painter.setPen( NAME_COLOR );
        painter.drawText( rect.translated( -SHADOW_OFFSET, -SHADOW_OFFSET ), elidedName );
    }
}

void SlideShowUI::keyPressEvent( QKeyEvent* e )
{
    if( e->key() == Qt::Key_Back ) {
        emit pressed();
        e->accept();
    }
}

void SlideShowUI::mousePressEvent( QMouseEvent* )
{
    emit pressed();
}
