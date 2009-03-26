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

#ifndef QPIMDELEGATE_H
#define QPIMDELEGATE_H

#include <qtopiaglobal.h>

#include <QAbstractItemDelegate>
#include <QFont>
#include <QMetaType>
#include <QPair>
#include <QString>
#include <QPixmap>
#include <QHash>

class QPimDelegateData;
typedef QPair<QString, QString> StringPair;
class QTOPIAPIM_EXPORT QPimDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    QPimDelegate(QObject * = 0);
    ~QPimDelegate();

    // base delegate functionality
    void paint(QPainter *p, const QStyleOptionViewItem& option, const QModelIndex& index) const;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;

protected:
    enum SubTextAlignment {Independent, CuddledPerItem};
    enum BackgroundStyle {None, SelectedOnly, Gradient};

    // customizability (these should be overridden as needed)

    // Background/foreground
    virtual BackgroundStyle backgroundStyle(const QStyleOptionViewItem &option, const QModelIndex& index) const;
    virtual void drawBackground(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex& index) const;
    virtual void drawForeground(QPainter *p, const QStyleOptionViewItem &option, const QModelIndex& index) const;

    // Decorations
    virtual void drawDecorations(QPainter* p, bool rtl, const QStyleOptionViewItem &option, const QModelIndex& index, QList<QRect>& leadingFloats, QList<QRect>& trailingFloats) const;
    virtual QSize decorationsSizeHint(const QStyleOptionViewItem& option, const QModelIndex& index, const QSize& textSizeHint) const;

    // Main line of text
    virtual QString mainText(const QStyleOptionViewItem &option, const QModelIndex& index) const;

    // Subsidiary lines of text
    virtual QList<StringPair> subTexts(const QStyleOptionViewItem &option, const QModelIndex& index) const;
    virtual int subTextsCountHint(const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual SubTextAlignment subTextAlignment(const QStyleOptionViewItem &option, const QModelIndex& index) const;

    // text line placement
    virtual QRect textRectangle(const QRect& entireRect, const QList<QRect>& leftFloats, const QList<QRect>& rightFloats, int top, int height) const;

    // Font selection
    virtual QFont mainFont(const QStyleOptionViewItem &option, const QModelIndex& index) const;
    virtual QFont secondaryHeaderFont(const QStyleOptionViewItem &option, const QModelIndex& index) const;
    virtual QFont secondaryFont(const QStyleOptionViewItem &option, const QModelIndex& index) const;

    // Helper function
    QFont differentFont(const QFont& start, int step) const;

private:
    // Data
    QPimDelegateData *d;
};

#endif
