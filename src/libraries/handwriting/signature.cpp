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

#include "signature.h"
#include "stroke.h"

#include <limits.h>

#define QIMPEN_CORRELATION_POINTS   25
int arcTan( int dy, int dx )
{
    if ( dx == 0 ) {
        if ( dy >= 0 )
            return 64;
        else
            return 192;
    }

    if ( dy == 0 ) {
        if ( dx >= 0 )
            return 0;
        else
            return 128;
    }

    static int table[5][5] = {
        { 32, 19, 13, 10, 8  },
        { 45, 32, 24, 19, 16 },
        { 51, 40, 32, 26, 22 },
        { 54, 45, 37, 32, 27 },
        { 56, 49, 42, 37, 32 } };

    if ( dy > 0 ) {
        if ( dx > 0 )
            return table[dy-1][dx-1];
        else
            return 128 - table[dy-1][qAbs(dx)-1];
    } else {
        if ( dx > 0 )
            return 256 - table[qAbs(dy)-1][dx-1];
        else
            return 128 + table[qAbs(dy)-1][qAbs(dx)-1];
    }

    return 0;
}

/*!
    \class QIMPenSignature
    \inpublicgroup QtInputMethodsModule
    \internal
    \brief The QIMPenSignature class allows correlations of strokes.

    Strokes are compared using a variety of "signatures" that describe
    features of the stroke.
*/
QIMPenSignature::QIMPenSignature() : QVector<int>()
{
}

QIMPenSignature::QIMPenSignature(const QVector<int> &o)
    : QVector<int>(o)
{
}

QIMPenSignature::QIMPenSignature(const QIMPenSignature &o)
    : QVector<int>(o)
{
}

void QIMPenSignature::setStroke(const QIMPenStroke &stroke)
{
    resize(0);
    calcSignature(stroke);
    if (count() != QIMPEN_CORRELATION_POINTS)
        scale( QIMPEN_CORRELATION_POINTS, loops() );
}

/*!
  Silly name.  Create an array from \a a that has \a e points extra at the start and
  end to enable a sliding correlation to be performed.
*/
QVector<int> QIMPenSignature::createBase( const QVector<int> &a, int e )
{
    QVector<int> ra( a.count() + 2*e );
    int i;
    for ( i = 0; i < e; i++ ) {
        ra[i] = a[0];
        ra[(int)a.count() + e + i - 1] = a[(int)a.count() - 1];
    }
    for ( i = 0; i < (int)a.count(); i++ ) {
        ra[i+e] = a[i];
    }

    return ra;
}

/*!
  Scale the points in array \a s to \a count points.
  This is braindead at the moment (no smooth scaling) and fixing this is
  probably one of the simpler ways to improve performance.

  The \a circular parameter is if it loops.
*/
void QIMPenSignature::scale( unsigned int dcount, bool circular )
{
    uint scount = count();

    QIMPenSignature &me = *this;
    int si = 0;
    if ( scount > dcount ) {
        int next = 0;
        for ( uint i = 0; i < dcount; i++ ) {
            next = (i+1) * scount / dcount;
            int maxval = 0;
            if ( circular ) {
                for ( int j = si; j < next; j++ ) {
                    maxval = at(j) > maxval ? at(j) : maxval;
                }
            }
            int sum = 0;
            for ( int j = si; j < next; j++ ) {
                if ( circular && maxval - at(j) > 128 )
                    sum += 256;
                sum += at(j);
            }
            me[i] = sum / (next-si);
            if ( circular && me[i] > 256 )
                me[i] %= 256;
            si = next;
        }
        resize(dcount);
    } else {
        resize(dcount);
        // could be better
        for ( int i = int(dcount)-1; i >= 0; i-- ) {
            si = i * scount / dcount;
            me[i] = me[si];
        }
    }
}

int QIMPenSignature::calcError(const QIMPenSignature &other, const QVector<int> &weight) const
{
    if (slides()) {
        QVector<int> base = createBase( *this, 2 );
        int err = INT_MAX;
        for ( int i = 0; i < 4; i++ ) {
            int e = calcError( base, other, i, loops(), weight );
            if ( e < err )
                err = e;
        }
        return err;
    } else {
        return calcError(*this, other, 0, loops(), weight);
    }

}

/*!
  Perform a correlation of the supplied arrays.  \a base should have
  win.count() + 2 * \a off points to enable sliding \a win over the
  \a base data.  If \a circular is true, the comparison takes into account
  the circular nature of the angular data.  If \a weight is not empty, will modify the error
  values for each point by the value in weight for the same index as a scale from 0 to 256.
  Returns the best (lowest error) match.
*/
int QIMPenSignature::calcError(const QVector<int> &base,
        const QVector<int> &win, int off, bool circular, const QVector<int> &weight)
{
    int err = 0;

    for ( int i = 0; i < win.count(); i++ ) {
        int d = qAbs( base[(int)i+off] - win[(int)i] );
        if ( circular && d > 128 )
            d -= 256;

        if (weight.count())
            err += (qAbs( d ) * weight[i]) >> 8;
        else
            err += qAbs( d );
    }

    return err / win.count();
}

/*
    Creates a signature of the tangent to the points on the stroke.
*/
TanSignature::TanSignature(const QIMPenStroke &stroke)
{
    setStroke(stroke);
}

TanSignature::~TanSignature() {}

void TanSignature::calcSignature(const QIMPenStroke &stroke)
{
    int dist = 5; // number of points to include in calculation
    TanSignature &me = *this;

    if ( stroke.chain().count() <= dist ) {
        resize(1);
        int dx = 0;
        int dy = 0;
        for ( int j = 0; j < stroke.chain().count(); j++ ) {
            dx += stroke.chain()[j].dx;
            dy += stroke.chain()[j].dy;
        }
        me[0] = arcTan( dy, dx );
    } else {
        resize( (stroke.chain().count()-dist+1) / 2 );
        int idx = 0;
        for ( int i = 0; i < stroke.chain().count() - dist; i += 2 ) {
            int dx = 0;
            int dy = 0;
            for ( int j = 0; j < dist; j++ ) {
                dx += stroke.chain()[i+j].dx;
                dy += stroke.chain()[i+j].dy;
            }
            me[idx++] = arcTan( dy, dx );
        }
    }
}

/*
    Creates a signature of the tan of the angle from the centre of gravity
    of the stroke to the points on the stroke.
*/
AngleSignature::AngleSignature(const QIMPenStroke &stroke)
{
    setStroke(stroke);
}

AngleSignature::~AngleSignature() {}


void AngleSignature::calcSignature(const QIMPenStroke &stroke)
{
    QPoint c = stroke.center();

    AngleSignature &me = *this;
    int dist = 3; // number of points to include in calculation
    if ( stroke.chain().count() <= dist ) {
        resize(1);
        me[0] = 1;
    } else {
        resize( stroke.chain().count() );
        QPoint current(0, 0);
        int idx = 0;
        for ( int i = 0; i < stroke.chain().count(); i++ ) {
            int dx = c.x() - current.x();
            int dy = c.y() - current.y();
            int md = qMax( qAbs(dx), qAbs(dy) );
            if ( md > 5 ) {
                dx = dx * 5 / md;
                dy = dy * 5 / md;
            }
            me[idx++] = arcTan( dy, dx );
            current += QPoint( stroke.chain()[i].dx, stroke.chain()[i].dy );
        }
    }
}

/*
    Creates a signature of the distance from the centre of gravity
    of the stroke to the points on the stroke.
*/
DistSignature::DistSignature(const QIMPenStroke &stroke)
{
    setStroke(stroke);
}

DistSignature::~DistSignature()
{
}

void DistSignature::calcSignature(const QIMPenStroke &stroke)
{
    resize( (stroke.chain().count()+1)/2 );
    QPoint c = stroke.center();
    QPoint pt( 0, 0 );

    DistSignature &me = *this;
    int minval = INT_MAX;
    int maxval = 0;
    int idx = 0, i;
    for ( i = 0; i < stroke.chain().count(); i += 2 ) {
        int dx = c.x() - pt.x();
        int dy = c.y() - pt.y();
        if ( dx == 0 && dy == 0 )
            me[idx] = 0;
        else
            me[idx] = dx*dx + dy*dy;

        if ( me[idx] > maxval )
            maxval = me[idx];
        if ( me[idx] < minval )
            minval = me[idx];
        pt.rx() += stroke.chain()[i].dx;
        pt.ry() += stroke.chain()[i].dy;
        idx++;
    }

    // normalise 0-255
    int div = maxval - minval;
    if ( div == 0 ) div = 1;
    for ( i = 0; i < count(); i++ ) {
        me[i] = (me[i] - minval ) * 255 / div;
    }

}

