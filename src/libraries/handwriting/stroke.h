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

#ifndef STROKE_H
#define STROKE_H

#include <qobject.h>
#include <qlist.h>
#include <qiterator.h>
#include <qpoint.h>
#include <qrect.h>

#include <qtopiaglobal.h>

#include "signature.h"

class QTOPIAHW_EXPORT QIMPenStroke
{
public:
    QIMPenStroke();
    QIMPenStroke( const QIMPenStroke & );
    ~QIMPenStroke(){}

    void clear();
    bool isEmpty() const { return links.isEmpty(); }
    unsigned int length() const { return links.count(); }
    unsigned int match( QIMPenStroke *st );
    const QVector<QIMPenGlyphLink> &chain() const { return links; }
    QPoint startingPoint() const { return startPoint; }
    void setStartingPoint( const QPoint &p ) { startPoint = p; }
    QRect boundingRect() const;
    QPoint center() const;

    int canvasHeight() const { return cheight; }
    void setCanvasHeight(int h) { cheight = h; }

    static void setUseCanvasPosition(bool b) { useVertPos = b; }
    static bool useCanvasPosition() { return useVertPos; }

    QIMPenStroke &operator=( const QIMPenStroke &s );

    void beginInput( QPoint p );
    bool addPoint( QPoint p );
    void endInput();

    QVector<int> tansig() { createSignatures(); return tsig; } // for debugging
    QVector<int> angnsig() { createSignatures(); return asig; } // for debugging
    QVector<int> dstsig() { createSignatures(); return dsig; } // for debugging

protected:
    void createSignatures();
    void internalAddPoint( QPoint p );

protected:
    QPoint startPoint;
    QPoint lastPoint;
    QVector<QIMPenGlyphLink> links;
    TanSignature tsig;
    AngleSignature asig;
    DistSignature dsig;
    mutable QRect bounding;
    int cheight;
    static bool useVertPos;

    friend QDataStream &operator<< (QDataStream &, const QIMPenStroke &);
    friend QDataStream &operator>> (QDataStream &, QIMPenStroke &);
};

typedef QList<QIMPenStroke *> QIMPenStrokeList;

typedef QList<QIMPenStroke *>::iterator QIMPenStrokeIterator;
typedef QList<QIMPenStroke *>::const_iterator QIMPenStrokeConstIterator;

QTOPIAHW_EXPORT QDataStream & operator<< (QDataStream & s, const QIMPenStroke &ws);
QTOPIAHW_EXPORT QDataStream & operator>> (QDataStream & s, const QIMPenStroke &ws);

#endif

