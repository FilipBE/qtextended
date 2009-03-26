/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
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
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.3, included in
** the file GPL_EXCEPTION.txt in this package.
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

#include "grid_p.h"

#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui/QPainter>
#include <QtGui/QWidget>
#include <QtGui/qevent.h>

QT_BEGIN_NAMESPACE

static const bool defaultSnap = true;
static const bool defaultVisible = true;
static const int DEFAULT_GRID = 10;
static const char* KEY_VISIBLE = "gridVisible";
static const char* KEY_SNAPX =  "gridSnapX";
static const char* KEY_SNAPY =  "gridSnapY";
static const char* KEY_DELTAX =  "gridDeltaX";
static const char* KEY_DELTAY =  "gridDeltaY";

// Insert a value into the serialization map unless default
template <class T>
    static inline void valueToVariantMap(T value, T defaultValue, const QString &key, QVariantMap &v, bool forceKey) {
        if (forceKey || value != defaultValue)
            v.insert(key, QVariant(value));
    }

// Obtain a value form QVariantMap
template <class T>
    static inline bool valueFromVariantMap(const QVariantMap &v, const QString &key, T &value) {
        const QVariantMap::const_iterator it = v.constFind(key);
        const bool found = it != v.constEnd();
        if (found)
            value = qVariantValue<T>(it.value());
        return found;
    }

namespace qdesigner_internal
{

Grid::Grid() :
    m_visible(defaultVisible),
    m_snapX(defaultSnap),
    m_snapY(defaultSnap),
    m_deltaX(DEFAULT_GRID),
    m_deltaY(DEFAULT_GRID)
{
}

bool Grid::fromVariantMap(const QVariantMap& vm)
{
    *this = Grid();
    valueFromVariantMap(vm, QLatin1String(KEY_VISIBLE), m_visible);
    valueFromVariantMap(vm, QLatin1String(KEY_SNAPX), m_snapX);
    valueFromVariantMap(vm, QLatin1String(KEY_SNAPY), m_snapY);
    valueFromVariantMap(vm, QLatin1String(KEY_DELTAX), m_deltaX);
    return valueFromVariantMap(vm, QLatin1String(KEY_DELTAY), m_deltaY);
}

QVariantMap Grid::toVariantMap(bool forceKeys) const
{
    QVariantMap rc;
    addToVariantMap(rc, forceKeys);
    return rc;
}

void  Grid::addToVariantMap(QVariantMap& vm, bool forceKeys) const
{
    valueToVariantMap(m_visible, defaultVisible, QLatin1String(KEY_VISIBLE), vm, forceKeys);
    valueToVariantMap(m_snapX, defaultSnap, QLatin1String(KEY_SNAPX), vm, forceKeys);
    valueToVariantMap(m_snapY, defaultSnap, QLatin1String(KEY_SNAPY), vm, forceKeys);
    valueToVariantMap(m_deltaX, DEFAULT_GRID, QLatin1String(KEY_DELTAX), vm, forceKeys);
    valueToVariantMap(m_deltaY, DEFAULT_GRID, QLatin1String(KEY_DELTAY), vm, forceKeys);
}

void Grid::paint(QWidget *widget, QPaintEvent *e) const
{
    QPainter p(widget);
    paint(p, widget, e);
}

void Grid::paint(QPainter &p, const QWidget *widget, QPaintEvent *e) const
{
    p.setPen(widget->palette().dark().color());

    if (m_visible) {
        const int xstart = (e->rect().x() / m_deltaX) * m_deltaX;
        const int ystart = (e->rect().y() / m_deltaY) * m_deltaY;

        const int xend = e->rect().right();
        const int yend = e->rect().bottom();

        typedef QVector<QPointF> Points;
        static Points points;
        points.clear();

        for (int x = xstart; x <= xend; x += m_deltaX) {
            points.reserve((yend - ystart) / m_deltaY + 1);
            for (int y = ystart; y <= yend; y += m_deltaY)
                points.push_back(QPointF(x, y));
            p.drawPoints( &(*points.begin()), points.count());
            points.clear();
        }
    }
}

int Grid::snapValue(int value, int grid) const
{
    const int rest = value % grid;
    const int absRest = (rest < 0) ? -rest : rest;
    int offset = 0;
    if (2 * absRest > grid)
        offset = 1;
    if (rest < 0)
        offset *= -1;
    return (value / grid + offset) * grid;
}

QPoint Grid::snapPoint(const QPoint &p) const
{
    const int sx = m_snapX ? snapValue(p.x(), m_deltaX) : p.x();
    const int sy = m_snapY ? snapValue(p.y(), m_deltaY) : p.y();
    return QPoint(sx, sy);
}

int Grid::widgetHandleAdjustX(int x) const
{
    return m_snapX ? (x / m_deltaX) * m_deltaX + 1 : x;
}

int Grid::widgetHandleAdjustY(int y) const
{
    return m_snapY ? (y / m_deltaY) * m_deltaY + 1 : y;
}

}

QT_END_NAMESPACE
