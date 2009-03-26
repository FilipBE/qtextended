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

#include "qglobal.h"

#ifndef QT_NO_GRAPHICSVIEW

#include "qgraphicslayout_p.h"
#include "qgraphicslayout.h"
#include "qgraphicswidget.h"

QT_BEGIN_NAMESPACE

/*!
    \internal

    Returns the parent widget of this layout, or 0 if this layout is
    not installed on any widget.

    If the layout is a sub-layout, this function returns the parent
    widget of the parent layout.

    \sa parent()
*/
QGraphicsWidget *QGraphicsLayoutPrivate::parentWidget() const
{
    Q_Q(const QGraphicsLayout);

    const QGraphicsLayoutItem *parent = q;
    while (parent && parent->isLayout()) {
        parent = parent->parentLayoutItem();
    }
    return static_cast<QGraphicsWidget *>(const_cast<QGraphicsLayoutItem *>(parent));
}

/*!
    \internal

    \a mw is the new parent. all items in the layout will be a child of \a mw.
 */
void QGraphicsLayoutPrivate::reparentChildWidgets(QGraphicsWidget *mw)
{
    Q_Q(QGraphicsLayout);
    int n =  q->count();
    //bool mwVisible = mw && mw->isVisible();
    for (int i = 0; i < n; ++i) {
        QGraphicsLayoutItem *item = q->itemAt(i);
        if (!item) {
            // Skip stretch items
            continue;
        }
        if (item->isLayout()) {
            QGraphicsLayout *l = static_cast<QGraphicsLayout*>(item);
            l->d_func()->reparentChildWidgets(mw);
        } else {
            QGraphicsWidget *w = static_cast<QGraphicsWidget*>(item);
            QGraphicsWidget *pw = w->parentWidget();
#ifdef QT_DEBUG
            if (pw && pw != mw && qt_graphicsLayoutDebug()) {
                qWarning("QGraphicsLayout::addChildLayout: widget %s \"%s\" in wrong parent; moved to correct parent",
                         w->metaObject()->className(), w->objectName().toLocal8Bit().constData());
            }
#endif
            //bool needShow = mwVisible && !(w->isHidden() && w->testAttribute(Qt::WA_WState_ExplicitShowHide));
            if (pw != mw)
                w->setParentItem(mw);
            //if (needShow)
            //    QMetaObject::invokeMethod(w, "_q_showIfNotHidden", Qt::QueuedConnection); //show later
        }
    }
}

void QGraphicsLayoutPrivate::getMargin(qreal *result, qreal userMargin, QStyle::PixelMetric pm) const
{
    if (!result)
        return;
    Q_Q(const QGraphicsLayout);

    QGraphicsLayoutItem *parent = q->parentLayoutItem();
    if (userMargin >= 0.0) {
        *result = userMargin;
    } else if (!parent) {
        *result = 0.0;
    } else if (parent->isLayout()) {    // sublayouts have 0 margin by default
        *result = 0.0;
    } else {
        *result = (qreal)static_cast<QGraphicsWidget*>(parent)->style()->pixelMetric(pm, 0);
    }
}


/*!
    \internal

    This function is called from subclasses to add layout \a l as a
    sub-layout.

    The only scenario in which you need to call it directly is if you
    implement a custom layout that supports nested layouts.
*/
void QGraphicsLayoutPrivate::addChildLayout(QGraphicsLayout *l)
{
    Q_Q(QGraphicsLayout);
    if (l->parentLayoutItem()) {
        qWarning("QGraphicsLayout::addChildLayout: layout already has a parent");
        return;
    }

    l->setParentLayoutItem(q);
    if (QGraphicsWidget *mw = parentWidget()) {
        l->d_func()->reparentChildWidgets(mw);
    }
}

static bool removeWidgetFromLayout(QGraphicsLayout *lay, QGraphicsWidget *wid)
{
    if (!lay)
        return false;

    QGraphicsLayoutItem *child;
    for (int i = 0; (child = lay->itemAt(i)); ++i) {
        if (child && child->isLayout()) {
            if (removeWidgetFromLayout(static_cast<QGraphicsLayout*>(child), wid))
                return true;
        } else if (child == wid) {
            lay->removeAt(i);
            return true;
        }
    }
    return false;
}

/*!
    \internal

    This function is called from subclasses to add graphics widget \a w to
    a layout. If the layout has a parentWidget, it will set that parent to be the parent of \a w.
    If \a w is already in a layout, it will remove \a w from that layout.
*/
void QGraphicsLayoutPrivate::addChildWidget(QGraphicsWidget *w)
{
    Q_Q(QGraphicsLayout);

    QGraphicsWidget *lw = parentWidget();
    QGraphicsWidget *pw = w->parentWidget();
    if (pw == lw || !lw)
        return;
    if (pw) {
        QGraphicsLayout *pl = pw->layout();
        if (pl) {
            removeWidgetFromLayout(pl, w);
        }
    }

#ifdef QT_DEBUG
    if (pw) {
        qWarning("QGraphicsLayout::addChildWidget: %s \"%s\" in wrong parent; moved to correct parent",
            w->metaObject()->className(), w->objectName().toLocal8Bit().constData());
    }
#endif

    w->setParentLayoutItem(q);
    w->setParentItem(lw);
}

QT_END_NAMESPACE
        
#endif //QT_NO_GRAPHICSVIEW
