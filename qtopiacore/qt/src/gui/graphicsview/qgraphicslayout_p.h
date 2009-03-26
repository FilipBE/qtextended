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

#ifndef QGRAPHICSLAYOUT_P_H
#define QGRAPHICSLAYOUT_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qglobal.h>

#if !defined(QT_NO_GRAPHICSVIEW) || (QT_EDITION & QT_MODULE_GRAPHICSVIEW) != QT_MODULE_GRAPHICSVIEW

#include "qgraphicslayout.h"
#include "qgraphicslayoutitem_p.h"
#include <QtGui/qstyle.h>

QT_BEGIN_NAMESPACE

class QGraphicsLayoutItem;
class QGraphicsWidget;

#ifdef QT_DEBUG
inline bool qt_graphicsLayoutDebug()
{
    static int checked_env = -1;
    if(checked_env == -1)
        checked_env = !!qgetenv("QT_GRAPHICSLAYOUT_DEBUG").toInt();
    return checked_env;
}
#endif

class Q_AUTOTEST_EXPORT QGraphicsLayoutPrivate : public QGraphicsLayoutItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsLayout)

public:
    QGraphicsLayoutPrivate() : QGraphicsLayoutItemPrivate(0, true), left(-1.0), top(-1.0), right(-1.0), bottom(-1.0), 
        activated(false) { }

    QGraphicsWidget *parentWidget() const;
    void reparentChildWidgets(QGraphicsWidget *mw);
    void getMargin(qreal *result, qreal userMargin, QStyle::PixelMetric pm) const;

    void addChildLayout(QGraphicsLayout *l);
    void addChildWidget(QGraphicsWidget *w);

    qreal left, top, right, bottom;
    bool activated;
};


QT_END_NAMESPACE
        
#endif //QT_NO_GRAPHICSVIEW

#endif
