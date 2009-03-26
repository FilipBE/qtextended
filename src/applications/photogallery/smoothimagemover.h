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
#ifndef SMOOTHIMAGEMOVER_H
#define SMOOTHIMAGEMOVER_H

#include <QObject>
#include <QTime>
#include <QRect>
#include <QPoint>

class SmoothImageMover : public QObject {
    Q_OBJECT
public:
    enum Direction { Horizontal, Vertical };

    SmoothImageMover( QObject *parent = 0 );
    ~SmoothImageMover();

    void fixMovements( Direction direction );
    void allowMovements( Direction direction );
    bool movementsFixed( Direction direction ) const;

    QPoint position() const { return m_pos; }

public slots:
    void setAllowedRect( const QRect& );

    void setPosition( const QPoint& );

    void startMoving( const QPoint& );
    void moveTo( const QPoint& );
    void endMoving(  const QPoint& );

signals:
    void positionChanged( const QPoint& );

protected:
    virtual void timerEvent(QTimerEvent *);

private:
    void continueMoving();

    bool m_horizontalFixed;
    bool m_verticalFixed;

    bool m_isMoving;

    QPoint m_pos;
    QRect m_allowedRect;

    QPoint m_startMousePos;
    QPoint m_startPos;

    QPoint m_lastPos;
    QPointF m_velocity;
    QTime m_timer;
    QTime m_lastReleaseTimer;

    bool m_haveTargetPos;
    QPoint m_targetPos;

    int m_moveTimerId;
};

#endif

