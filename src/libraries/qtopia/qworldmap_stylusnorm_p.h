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

#ifndef QWORLDMAP_STYLUSNORM_P_H
#define QWORLDMAP_STYLUSNORM_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt Extended API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// Qt includes
#include <QWidget>
#include <QPoint>
#include <QTime>

// ============================================================================
//
// _StylusEvent
//
// ============================================================================

class _StylusEvent
{
public:
    _StylusEvent( const QPoint &pt = QPoint( 0, 0 ) );
    ~_StylusEvent();
    QPoint point( void ) const { return _pt; };
    QTime time( void ) const { return _t; };
    void setPoint( int x, int y) { _pt.setX( x ); _pt.setY( y ); };
    void setPoint( const QPoint &newPt ) { _pt = newPt; };
    void setTime( QTime newTime ) { _t = newTime; };

private:
    QPoint _pt;
    QTime _t;
};

// ============================================================================
//
// StylusNormalizer
//
// ============================================================================

class StylusNormalizer : public QWidget
{
    Q_OBJECT
public:
    explicit StylusNormalizer( QWidget *parent = 0 );
    ~StylusNormalizer();
    void start();
    void stop();
    void addEvent( const QPoint &pt );   // add a new point in

signals:
    void signalNewPoint( const QPoint &p );

private slots:
    void slotAveragePoint( void );  // return an averaged point

private:
    enum {SAMPLES = 10};
    _StylusEvent _ptList[SAMPLES];
    int _next;
    QTimer *_tExpire;
    bool bFirst;    // the first item added in...
};

#endif
