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

#include "mediastyle_p.h"

static QPixmap generate_progress_bar( const QColor& start, const QColor& end, const QSize& size )
{
    QLinearGradient gradient( 0, 0, 0, size.height() );
    gradient.setColorAt( 0.3, start );
    gradient.setColorAt( 0.9, end );

    QPixmap buffer = QPixmap( size );
    buffer.fill( QColor( 0, 0, 0, 0 ) );

    QPainter painter( &buffer );
    painter.setPen( Qt::NoPen );
    painter.setBrush( gradient );
    painter.drawRect( buffer.rect() );

    return buffer;
}

/*!
    \class MediaStyle
    \inpublicgroup QtMediaModule
    \internal
*/

/*!
    \fn void MediaStyle::drawControl( ControlElement ce, const QStyleOption* opt, QPainter* p, const QWidget* widget ) const
    \internal
*/
void MediaStyle::drawControl( ControlElement ce, const QStyleOption *opt, QPainter *p, const QWidget *widget ) const
{
    switch( ce )
    {
    // Slimline progress bar
    case CE_ProgressBarGroove:
        {
        p->setPen( opt->palette.color( QPalette::Shadow ).light( 300 ) );
        p->setBrush( Qt::NoBrush );
        p->drawRect( opt->rect.adjusted( 0, 0, -1, -1 ) );
        }
        break;
    case CE_ProgressBarContents:
        {
        const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar*>(opt);
        if( pb ) {
            QRect rect = pb->rect.adjusted(1,1,-1,-1);
            QRect progress;

            if( m_groovebuffer.isNull() || rect.size() != m_groovebuffer.size() ) {
                QColor color = opt->palette.color( QPalette::Shadow );
                m_groovebuffer = generate_progress_bar( color, color.light( 300 ), rect.size() );
            }

            if( pb->maximum > pb->minimum ) {
                progress = rect.adjusted( 0, 0,
                    -rect.width() + rect.width()*(pb->progress - pb->minimum)/(pb->maximum - pb->minimum), 0 );
                progress.adjust( 1, 1, -1, -1 );
            }

            if( opt->state == QStyle::State_Sunken ) {
                if( progress.isValid() ) {
                    if( m_silhouettebuffer.isNull() || rect.size() != m_silhouettebuffer.size() ) {
                        QColor color = opt->palette.color( QPalette::Highlight );
                        m_silhouettebuffer = generate_progress_bar( color, color.dark( 150 ), rect.size() );
                    }

                    p->drawPixmap( progress, m_silhouettebuffer, progress );
                }
            } else {
                if( progress.isValid() ) {
                    if( m_barbuffer.isNull() || rect.size() != m_barbuffer.size() ) {
                        QColor color = opt->palette.color( QPalette::Highlight );
                        m_barbuffer = generate_progress_bar( color, color.dark( 150 ), rect.size() );
                    }

                    if( p->paintEngine()->hasFeature( QPaintEngine::PorterDuff ) ) {
                        p->setCompositionMode( QPainter::CompositionMode_Source ); // ### Cheat
                    }
                    p->drawPixmap( progress, m_barbuffer, progress );
                    QRect groovrect(QPoint( progress.right() + 1, rect.top() + 1 ),
                            QPoint( rect.right() - 1, rect.bottom() - 1 ) );
                    p->drawPixmap( groovrect, m_groovebuffer, groovrect );
                } else {
                    if( p->paintEngine()->hasFeature( QPaintEngine::PorterDuff ) ) {
                        p->setCompositionMode( QPainter::CompositionMode_Source ); // ### Cheat
                    }
                    p->drawPixmap( rect.adjusted( 1, 1, -1, -1 ), m_groovebuffer );
                }
            }
        }
        }
        break;
    default:
        QCommonStyle::drawControl( ce, opt, p, widget );
        break;
    }
}

/*!
    \fn int MediaStyle::pixelMetric( PixelMetric pm, const QStyleOption* opt, const QWidget* widget ) const
    \internal
*/
int MediaStyle::pixelMetric( PixelMetric pm, const QStyleOption *opt, const QWidget *widget ) const
{
    if( pm == PM_ProgressBarChunkWidth ) {
        return 1;
    }

    return QCommonStyle::pixelMetric( pm, opt, widget );
}
