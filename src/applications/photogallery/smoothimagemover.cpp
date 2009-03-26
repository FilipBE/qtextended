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

#include "smoothimagemover.h"
#include <QTimerEvent>
#include <QDebug>


SmoothImageMover::SmoothImageMover( QObject *parent )
:QObject(parent), m_horizontalFixed(false), m_verticalFixed(false), m_isMoving(false), m_moveTimerId(-1)
{
    m_lastReleaseTimer.start();
}

SmoothImageMover::~SmoothImageMover()
{
}

void SmoothImageMover::fixMovements( SmoothImageMover::Direction direction )
{
    if ( direction == SmoothImageMover::Horizontal )
        m_horizontalFixed = true;
    else
        m_verticalFixed = true;
}

void SmoothImageMover::allowMovements( SmoothImageMover::Direction direction )
{
    if ( direction == SmoothImageMover::Horizontal )
        m_horizontalFixed = false;
    else
        m_verticalFixed = false;
}

bool SmoothImageMover::movementsFixed( SmoothImageMover::Direction direction ) const
{
    return direction == SmoothImageMover::Horizontal ? m_horizontalFixed : m_verticalFixed;
}

void SmoothImageMover::setAllowedRect( const QRect &rect )
{
    m_allowedRect = rect;
}


void SmoothImageMover::setPosition( const QPoint &pos )
{
    m_pos = pos;

    if ( m_horizontalFixed )
        m_pos.setX( qBound( m_allowedRect.left(), m_pos.x(), m_allowedRect.right() ) );

    if ( m_verticalFixed )
        m_pos.setY( qBound( m_allowedRect.top(), m_pos.y(), m_allowedRect.bottom() ) );

    emit positionChanged(m_pos);

    m_velocity = QPointF(0,0);
    if ( !m_isMoving ) {
        continueMoving();
    } else {
    }
}

void SmoothImageMover::startMoving( const QPoint &pos )
{
    m_isMoving = true;

    if ( m_moveTimerId != -1 ) {
        killTimer(m_moveTimerId);
        m_moveTimerId = -1;
    }

    m_startMousePos = pos;
    m_startPos = m_pos;

    //don't stop the movement because of non ideal touchscreen
    if ( m_lastReleaseTimer.elapsed() > 200 )
        m_velocity = QPointF(0,0);

    m_lastPos = m_pos;
    m_timer.start();
}

void SmoothImageMover::moveTo( const QPoint &pos )
{
    QPoint newPos = pos - m_startMousePos + m_startPos;

    if ( m_pos != newPos ) {
        if ( m_verticalFixed )
            newPos.setY( m_pos.y() );
        if ( m_horizontalFixed )
            newPos.setX( m_pos.x() );

        m_pos = newPos;
        emit positionChanged(m_pos);
    }

    if ( m_timer.elapsed() > 60 ) {
        m_velocity = QPointF( m_pos - m_lastPos )/float( m_timer.elapsed() );

        m_lastPos = m_pos;
        m_timer.start();
    }
}

void SmoothImageMover::endMoving( const QPoint &pos )
{
    m_isMoving = false;
    QPoint newPos = pos - m_startMousePos + m_startPos;

    if ( m_pos != newPos ) {
        if ( m_verticalFixed )
            newPos.setY( m_pos.y() );
        if ( m_horizontalFixed )
            newPos.setX( m_pos.x() );

        m_pos = newPos;
        emit positionChanged(m_pos);
    }

    if ( m_timer.elapsed() > 60 ) {
        m_velocity = QPointF( m_pos - m_lastPos )/float( m_timer.elapsed() );
    }

    m_lastReleaseTimer.start();
    continueMoving();
}

void SmoothImageMover::continueMoving()
{
    m_timer.start();
    m_moveTimerId = startTimer(50);

    m_targetPos = QPoint();
    m_haveTargetPos = !m_allowedRect.contains(m_pos);
    if (m_haveTargetPos) {
        m_targetPos = m_pos;
        m_targetPos.setX( qBound( m_allowedRect.left(), m_pos.x(), m_allowedRect.right() ) );
        m_targetPos.setY( qBound( m_allowedRect.top(), m_pos.y(), m_allowedRect.bottom() ) );
    }
}

void SmoothImageMover::timerEvent(QTimerEvent * ev)
{
    if ( ev->timerId() == m_moveTimerId ) {
        int dt = m_timer.elapsed();
        m_velocity *= 0.8;
        m_timer.start();
        QPoint delta = (m_velocity*dt).toPoint();

        if ( !m_haveTargetPos && !m_allowedRect.contains(m_pos) ) { //get out of allowed area
            m_haveTargetPos = true;
            m_targetPos = m_pos;
            m_targetPos.setX( qBound( m_allowedRect.left(), m_pos.x(), m_allowedRect.right() ) );
            m_targetPos.setY( qBound( m_allowedRect.top(), m_pos.y(), m_allowedRect.bottom() ) );
        }

        if ( !m_allowedRect.contains(m_pos) ) {
            QPoint distance = m_targetPos - m_pos;
            if ( !distance.isNull() ) {
                QPointF gravitation = QPointF(distance)/distance.manhattanLength()/50;
                m_velocity += gravitation*dt;

                delta = (m_velocity * dt).toPoint();
                if ( delta.manhattanLength() > distance.manhattanLength() ) {
                    delta = distance/2;
                    m_velocity = QPointF(distance/2)/dt;
                }
            } else {
                delta = QPoint();
                m_velocity = QPointF();
            }
        }

        m_pos += delta;
        bool finished = false;

        if ( delta.manhattanLength() < 4 ) {
            if ( !m_haveTargetPos )
                finished = true;
            else {
                if ( (m_pos-m_targetPos).manhattanLength() < 16 ) {
                    m_pos = m_targetPos;
                    finished = true;
                }
            }
        }

        if ( finished ) {
            m_velocity = QPointF(0,0);
            killTimer(m_moveTimerId);
            m_moveTimerId = -1;
        }

        emit positionChanged( m_pos );
    }
}

