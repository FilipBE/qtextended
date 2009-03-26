/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include <qcursor.h>
#include <private/qcursor_p.h>
#include <qbitmap.h>
#include <qwsdisplay_qws.h>

QT_BEGIN_NAMESPACE

QT_USE_NAMESPACE

#ifndef QT_NO_CURSOR

static int nextCursorId = Qt::BitmapCursor;

/*****************************************************************************
  Internal QCursorData class
 *****************************************************************************/

QCursorData::QCursorData(Qt::CursorShape s)
    : cshape(s), bm(0), bmm(0), hx(0), hy(0), id(s)
{
    ref = 1;
}

QCursorData::~QCursorData()
{
    delete bm;
    delete bmm;
    QPaintDevice::qwsDisplay()->destroyCursor(id);
}


/*****************************************************************************
  Global cursors
 *****************************************************************************/

extern QCursorData *qt_cursorTable[Qt::LastCursor + 1]; // qcursor.cpp

int QCursor::handle() const
{
    return d->id;
}


QCursorData *QCursorData::setBitmap(const QBitmap &bitmap, const QBitmap &mask, int hotX, int hotY)
{
    if (!QCursorData::initialized)
        QCursorData::initialize();
    if (bitmap.depth() != 1 || mask.depth() != 1 || bitmap.size() != mask.size()) {
        qWarning("QCursor: Cannot create bitmap cursor; invalid bitmap(s)");
        QCursorData *c = qt_cursorTable[0];
        c->ref.ref();
        return c;
    }
    QCursorData *d = new QCursorData;
    d->bm  = new QBitmap(bitmap);
    d->bmm = new QBitmap(mask);
    d->cshape = Qt::BitmapCursor;
    d->id = ++nextCursorId;
    d->hx = hotX >= 0 ? hotX : bitmap.width() / 2;
    d->hy = hotY >= 0 ? hotY : bitmap.height() / 2;

    QPaintDevice::qwsDisplay()->defineCursor(d->id, *d->bm, *d->bmm, d->hx, d->hy);
    return d;
}

void QCursorData::update()
{
}

#endif //QT_NO_CURSOR

extern int *qt_last_x,*qt_last_y;

QPoint QCursor::pos()
{
    // This doesn't know about hotspots yet so we disable it
    //qt_accel_update_cursor();
    if (qt_last_x)
        return QPoint(*qt_last_x, *qt_last_y);
    else
        return QPoint();
}

void QCursor::setPos(int x, int y)
{
    // Need to check, since some X servers generate null mouse move
    // events, causing looping in applications which call setPos() on
    // every mouse move event.
    //
    if (pos() == QPoint(x, y))
        return;
    QPaintDevice::qwsDisplay()->setCursorPosition(x, y);
}

QT_END_NAMESPACE
