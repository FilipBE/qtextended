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

#ifndef QGRAPHICSLAYOUTITEM_H
#define QGRAPHICSLAYOUTITEM_H

#include <QtGui/qsizepolicy.h>
#include <QtGui/qevent.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#if !defined(QT_NO_GRAPHICSVIEW) || (QT_EDITION & QT_MODULE_GRAPHICSVIEW) != QT_MODULE_GRAPHICSVIEW

class QGraphicsLayoutItemPrivate;
class Q_GUI_EXPORT QGraphicsLayoutItem
{
public:
    QGraphicsLayoutItem(QGraphicsLayoutItem *parent = 0, bool isLayout = false);
    virtual ~QGraphicsLayoutItem();

    void setSizePolicy(const QSizePolicy &policy);
    void setSizePolicy(QSizePolicy::Policy hPolicy, QSizePolicy::Policy vPolicy, QSizePolicy::ControlType controlType = QSizePolicy::DefaultType);
    QSizePolicy sizePolicy() const;

    void setMinimumSize(const QSizeF &size);
    inline void setMinimumSize(qreal w, qreal h);
    QSizeF minimumSize() const;
    void setMinimumWidth(qreal width);
    inline qreal minimumWidth() const;
    void setMinimumHeight(qreal height);
    inline qreal minimumHeight() const;

    void setPreferredSize(const QSizeF &size);
    inline void setPreferredSize(qreal w, qreal h);
    QSizeF preferredSize() const;
    void setPreferredWidth(qreal width);
    inline qreal preferredWidth() const;
    void setPreferredHeight(qreal height);
    inline qreal preferredHeight() const;

    void setMaximumSize(const QSizeF &size);
    inline void setMaximumSize(qreal w, qreal h);
    QSizeF maximumSize() const;
    void setMaximumWidth(qreal width);
    inline qreal maximumWidth() const;
    void setMaximumHeight(qreal height);
    inline qreal maximumHeight() const;

    virtual void setGeometry(const QRectF &rect);
    QRectF geometry() const;
    virtual void getContentsMargins(qreal *left, qreal *top, qreal *right, qreal *bottom) const;
    QRectF contentsRect() const;

    QSizeF effectiveSizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

    virtual void updateGeometry();  //### rename to sizeHintChanged()

    QGraphicsLayoutItem *parentLayoutItem() const;
    void setParentLayoutItem(QGraphicsLayoutItem *parent);

    bool isLayout() const;

protected:
    QGraphicsLayoutItem(QGraphicsLayoutItemPrivate &dd);

    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const = 0;
    QGraphicsLayoutItemPrivate *d_ptr;

private:
    QSizeF *effectiveSizeHints(const QSizeF &constraint) const;
    Q_DECLARE_PRIVATE(QGraphicsLayoutItem)
};

inline void QGraphicsLayoutItem::setMinimumSize(qreal aw, qreal ah)
{ setMinimumSize(QSizeF(aw, ah)); }
inline void QGraphicsLayoutItem::setPreferredSize(qreal aw, qreal ah)
{ setPreferredSize(QSizeF(aw, ah)); }
inline void QGraphicsLayoutItem::setMaximumSize(qreal aw, qreal ah)
{ setMaximumSize(QSizeF(aw, ah)); }

inline qreal QGraphicsLayoutItem::minimumWidth() const
{ return effectiveSizeHint(Qt::MinimumSize).width(); }
inline qreal QGraphicsLayoutItem::minimumHeight() const
{ return effectiveSizeHint(Qt::MinimumSize).height(); }

inline qreal QGraphicsLayoutItem::preferredWidth() const
{ return effectiveSizeHint(Qt::PreferredSize).width(); }
inline qreal QGraphicsLayoutItem::preferredHeight() const
{ return effectiveSizeHint(Qt::PreferredSize).height(); }

inline qreal QGraphicsLayoutItem::maximumWidth() const
{ return effectiveSizeHint(Qt::MaximumSize).width(); }
inline qreal QGraphicsLayoutItem::maximumHeight() const
{ return effectiveSizeHint(Qt::MaximumSize).height(); }

#endif

QT_END_NAMESPACE

QT_END_HEADER

#endif
