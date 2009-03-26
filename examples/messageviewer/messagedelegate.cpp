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

#include "qtopialog.h"

#include "messagedelegate.h"
#include <QFont>
#include <QFontMetrics>
#include <QIcon>
#include <QModelIndex>
#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>
#include <QtopiaApplication>


MessageDelegate::MessageDelegate(QObject* parent)
    : QAbstractItemDelegate(parent)
{
}

MessageDelegate::~MessageDelegate()
{
}

QSize MessageDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(index)

    static const int iconSize(qApp->style()->pixelMetric(QStyle::PM_ListViewIconSize));
    static const int smallIconSize(qApp->style()->pixelMetric(QStyle::PM_SmallIconSize));

    QFont main(option.font);
    main.setWeight(QFont::Bold);

    QFont sub(main);
    sub.setPointSize(main.pointSize() - 2);

    QFontMetrics fm(main);
    QFontMetrics sfm(sub);

    return QSize(iconSize + 8 + smallIconSize, qMax((fm.lineSpacing() + 1 + sfm.lineSpacing()), iconSize) + 2);
}

void MessageDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    static const bool rtl(qApp->layoutDirection() == Qt::RightToLeft);
    static const int iconSize(qApp->style()->pixelMetric(QStyle::PM_ListViewIconSize));
    static const int smallIconSize(qApp->style()->pixelMetric(QStyle::PM_SmallIconSize));

    // Find the paintable elements of the item
    QIcon icon(qvariant_cast<QIcon>(index.model()->data(index, Qt::DecorationRole)));
    QIcon typeIcon(qvariant_cast<QIcon>(index.model()->data(index, SecondaryDecorationRole)));
    QString headerText(qvariant_cast<QString>(index.model()->data(index, Qt::DisplayRole)));
    QString subText(qvariant_cast<QString>(index.model()->data(index, SubLabelRole)));

    // Find the painting properties we need
    const bool sel((option.state & QStyle::State_Selected) == QStyle::State_Selected);
    QBrush baseBrush(sel ? option.palette.highlight() : option.palette.base());
    QBrush textBrush(sel ? option.palette.highlightedText() : option.palette.text());

    QFont main(option.font);
    main.setWeight(QFont::Bold);

    QFont sub(main);
    sub.setPointSize(main.pointSize() - 2);

    painter->save();
    painter->setClipRect(option.rect);

    // Draw the background gradient if selected
    if (sel) 
    {
        QPalette::ColorGroup cg((option.state & QStyle::State_Enabled) ? QPalette::Normal : QPalette::Disabled);

        QString key = QLatin1String("_MSGD_");
        key += QString::number(option.rect.width());
        key += QString::number(option.rect.height());
        key += QString::number(int(option.palette.color(cg, QPalette::Highlight).rgba()));

        QPixmap pm;
        if (!QPixmapCache::find(key, pm)) {
            QSize size = option.rect.size();
            QImage img(size, QImage::Format_ARGB32_Premultiplied);
            img.fill(0x00000000);
            QPainter pp(&img);
            pp.setRenderHint(QPainter::Antialiasing);
            QColor color = option.palette.color(cg, QPalette::Highlight);
            pp.setPen(color);

            QLinearGradient bgg(QPoint(0,0), QPoint(0, size.height()));
            bgg.setColorAt(0.0f, color.lighter(175));
            bgg.setColorAt(0.49f, color.lighter(105));
            bgg.setColorAt(0.5f, color);
            pp.setBrush(bgg);
            pp.drawRoundRect(QRect(QPoint(0,0),size), 800/size.width(),800/size.height());
            pm = QPixmap::fromImage(img);
            QPixmapCache::insert(key, pm);
        }
        painter->drawPixmap(option.rect.topLeft(), pm);
    }

    // Find the icon rectangles for this item
    QRect textRect(option.rect);
    QRect iconRect(option.rect);
    QRect secondaryRect(option.rect);

    if (rtl)
    {
        iconRect.setLeft(iconRect.right() - iconSize - 8);
        secondaryRect.setRight(smallIconSize);

        textRect.setRight(iconRect.left());
        textRect.setLeft(secondaryRect.right());
    }
    else
    {
        iconRect.setRight(iconSize + 8);
        secondaryRect.setLeft(secondaryRect.right() - smallIconSize - 8);

        textRect.setLeft(iconRect.right());
        textRect.setRight(secondaryRect.left());
    }

    // Find the text rectangles
    QFontMetrics fm(main);
    QRect headerRect(textRect);
    headerRect.setTop(headerRect.top() + 1);
    headerRect.setHeight(fm.lineSpacing());

    QFontMetrics sfm(sub);
    QRect subRect(textRect);
    subRect.setTop(subRect.bottom() - sfm.lineSpacing() + 1);
    subRect.setHeight(sfm.lineSpacing());

    // Paint the icons
    QPoint drawOffset(iconRect.left() + ((iconRect.width() - iconSize)/2), iconRect.top() + ((iconRect.height() - iconSize) / 2));
    painter->drawPixmap(drawOffset, icon.pixmap(QSize(iconSize, iconSize)));

    drawOffset = QPoint(secondaryRect.left() + ((secondaryRect.width() - smallIconSize)/2), secondaryRect.top() + ((secondaryRect.height() - smallIconSize) / 2));
    painter->drawPixmap(drawOffset, typeIcon.pixmap(QSize(smallIconSize, smallIconSize)));

    // Paint the text elements
    painter->setBrush(baseBrush);
    painter->setPen(textBrush.color());

    painter->setFont(main);
    painter->drawText(headerRect, Qt::AlignLeading, fm.elidedText(headerText, option.textElideMode, headerRect.width()));

    painter->setFont(sub);
    painter->drawText(subRect, Qt::AlignLeading, sfm.elidedText(subText, option.textElideMode, subRect.width()));

    painter->restore();
}

