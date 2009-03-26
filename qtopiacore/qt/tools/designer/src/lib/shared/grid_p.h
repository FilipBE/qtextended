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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNER_GRID_H
#define QDESIGNER_GRID_H

#include "shared_global_p.h"

#include <QtCore/QVariantMap>

QT_BEGIN_NAMESPACE

class QWidget;
class QPaintEvent;
class QPainter;

namespace qdesigner_internal {

// Designer grid which is able to serialize to QVariantMap
class QDESIGNER_SHARED_EXPORT Grid
{
public:
    Grid();

    bool fromVariantMap(const QVariantMap& vm);

    void addToVariantMap(QVariantMap& vm, bool forceKeys = false) const;
    QVariantMap toVariantMap(bool forceKeys = false) const;

    inline bool visible() const   { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

    inline bool snapX() const     { return m_snapX; }
    void setSnapX(bool snap)      { m_snapX = snap; }

    inline bool snapY() const     { return m_snapY; }
    void setSnapY(bool snap)      { m_snapY = snap; }

    inline int deltaX() const     { return m_deltaX; }
    void setDeltaX(int dx)        { m_deltaX = dx; }

    inline int deltaY() const     { return m_deltaY; }
    void setDeltaY(int dy)        { m_deltaY = dy; }

    void paint(QWidget *widget, QPaintEvent *e) const;
    void paint(QPainter &p, const QWidget *widget, QPaintEvent *e) const;

    QPoint snapPoint(const QPoint &p) const;

    int widgetHandleAdjustX(int x) const;
    int widgetHandleAdjustY(int y) const;

private:
    int snapValue(int value, int grid) const;
    bool m_visible;
    bool m_snapX;
    bool m_snapY;
    int m_deltaX;
    int m_deltaY;
};
} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QDESIGNER_GRID_H
