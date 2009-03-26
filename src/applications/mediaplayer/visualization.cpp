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

#include "visualization.h"

#include <stdlib.h>
#include <QtopiaApplication>

class Flake
{
public:
    Flake()
        : m_step( 1 ), m_isActive( true )
    { }

    bool isActive() const { return m_isActive; }
    void retire() { m_isActive = false; }

    void setStep( int step ) { m_step = step; }
    void advance() { m_pos.ry() += m_step; }

    void setPos( const QPoint& pos ) { m_pos = pos; }
    QPoint pos() const { return m_pos; }

private:
    QPoint m_pos;
    int m_step;

    bool m_isActive;
};

FlakeFactory::FlakeFactory( int maxpool )
    : m_count( maxpool )
{
    m_pool = new Flake[m_count];

    // Initialize pool
    Flake *flake = m_pool;
    Flake *end = flake + m_count;
    while( flake != end ) {
        flake->retire();
        ++flake;
    }
}

FlakeFactory::~FlakeFactory()
{
    delete[] m_pool;
}

Flake* FlakeFactory::create()
{
    Flake *flake = m_pool;
    Flake *end = flake + m_count;
    while( flake != end ) {
        if( !flake->isActive() ) {
            *flake = Flake();
            return flake;
        }
        ++flake;
    }

    return 0;
}

void FlakeFactory::retireAll()
{
    // Initialize pool
    Flake *flake = m_pool;
    Flake *end = flake + m_count;
    while( flake != end ) {
        flake->retire();
        ++flake;
    }
}

static const int MAX_POOL = 15;

VisualizationWidget::VisualizationWidget(QWidget* parent):
    QWidget(parent),
    m_init(false),
    m_isactive(false),
    m_generate(0),
    m_update(0),
    m_flakefactory(MAX_POOL)
{
}

void VisualizationWidget::setActive( bool active )
{
    if (m_isactive == active)
        return;

    m_isactive = active;

    init();

    if (m_isactive)
    {
        m_generate = startTimer(1500);
        m_update = startTimer(50);
    }
    else
    {
        if (m_generate)
        {
            killTimer(m_generate);
            m_generate = 0;
        }

        if (m_update)
        {
            killTimer(m_update);
            m_update = 0;
        }

        m_flakefactory.retireAll();
        update();
    }
}

void VisualizationWidget::paintEvent( QPaintEvent* e )
{
#ifdef NO_VISUALIZATION
    return;
#endif

    if (!m_isactive)
        return;

    QPainter painter( this );

    Flake *flake = m_flakefactory.flakes();
    Flake *end = flake + m_flakefactory.count();
    while( flake != end ) {
        if( flake->isActive() ) {
            QPoint pos = flake->pos();
            if( QRect( pos, m_flakepixmap.size() ).intersects( e->rect() ) ) {
                painter.drawPixmap( flake->pos(), m_flakepixmap );
            }
        }
        ++flake;
    }
}

void VisualizationWidget::timerEvent( QTimerEvent* e )
{
    static int step = 0;

    if( e->timerId() == m_update ) {
        // Advance flakes and paint
        Flake *flake = m_flakefactory.flakes();
        Flake *end = flake + m_flakefactory.count();
        while( flake != end ) {
            if( flake->isActive() ) {
                flake->advance();
                if( flake->pos().y() > height() ) {
                    flake->retire();
                }
            }
            ++flake;
        }
        update();
    } else if( e->timerId() == m_generate ) {
        // Generate random number of flakes
        int count = rand() % 3;
        for( int i = 0; i < count; ++i ) {
            Flake *flake = m_flakefactory.create();
            if( flake ) {
                flake->setPos( QPoint( rand() % width() - m_halfwidth,
                    -m_flakepixmap.height() - rand() % m_flakepixmap.height() ) );
                flake->setStep( (step = (step + 1) % 2) + 1 );
            }
        }
    }
}

void VisualizationWidget::init()
{
    static const QString FLAKE_PATH = ":icon/mediaplayer/visualization/qtopiaflake";

    if (m_init)
        return;

    int flakeside = QtopiaApplication::style()->pixelMetric(QStyle::PM_SmallIconSize)*3;

    m_flakepixmap = QIcon( FLAKE_PATH ).pixmap( QSize( flakeside, flakeside ) );
    m_halfwidth = m_flakepixmap.width() / 2;

    m_init = true;
}


