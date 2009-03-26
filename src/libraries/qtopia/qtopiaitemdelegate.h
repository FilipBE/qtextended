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

#ifndef QTOPIAITEMDELEGATE_H
#define QTOPIAITEMDELEGATE_H

#include <qtopiaglobal.h>
#include <qtopianamespace.h>
#include <QItemDelegate>
#include <QString>
#include <QPixmap>
#include <QVariant>


class QtopiaItemDelegatePrivate;

class QTOPIA_EXPORT QtopiaItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    explicit QtopiaItemDelegate(QObject *parent = 0);
    ~QtopiaItemDelegate();

    // painting
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;

protected:
    virtual void drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
                             const QRect &rect, const QString &text) const;
    virtual void drawDecoration(QPainter *painter, const QStyleOptionViewItem &option,
                                const QRect &rect, const QPixmap &pm) const;
    virtual void drawDecoration(QPainter *painter, const QStyleOptionViewItem &option,
                                const QRect &rect, const QVariant &decoration) const;
    virtual void drawAdditionalDecoration(QPainter *painter, const QStyleOptionViewItem &option,
                                const QRect &rect, const QVariant &decoration) const;
    virtual void drawFocus(QPainter *painter, const QStyleOptionViewItem &option,
                           const QRect &rect) const;
    virtual void drawCheck(QPainter *painter, const QStyleOptionViewItem &option,
                           const QRect &rect, Qt::CheckState state) const;
    virtual void drawBackground(QPainter *painter, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const;

    virtual void doLayout(const QStyleOptionViewItem &option,
                  QRect *checkRect, QRect *iconRect, QRect *textRect, QRect *addDecorationRect, bool hint) const;

    QRect rect(const QStyleOptionViewItem &option, const QModelIndex &index, int role) const;

    bool eventFilter(QObject *object, QEvent *event);

    QStyleOptionViewItem setOptions(const QModelIndex &index,
                                    const QStyleOptionViewItem &option) const;

    QPixmap *selected(const QPixmap &pixmap, const QPalette &palette, bool enabled) const;

    QRect check(const QStyleOptionViewItem &option, const QRect &bounding,
                const QVariant &variant) const;
    QRect textRectangle(QPainter *painter, const QRect &rect,
                        const QFont &font, const QString &text) const;

private:
    QtopiaItemDelegatePrivate *d;
    Q_DISABLE_COPY(QtopiaItemDelegate)
};

#endif
