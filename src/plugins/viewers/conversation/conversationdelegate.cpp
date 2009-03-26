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

#include "conversationdelegate.h"

#include <qtopianamespace.h>

#include <QMailMessage>
#include <QMailMessageListModel>
#include <QPainter>
#include <QPainterPath>
#include <QPixmapCache>
#include <QtopiaApplication>

#include <limits.h>


static int horizontalBubbleMargin = 10;
static int verticalBubbleMargin = 2;
static int itemMargin = 4;
static int lineSpacing = 1;


// Find a font that can display this text content
static QFont displayFont(const QString &text, const QFont &defaultFont)
{
    if (!text.isEmpty()) {
        QFontMetrics fm(defaultFont);

        foreach (const QChar& c, text) {
            if (!fm.inFont(c)) {
                // We need to find another font
                QVariant var = Qtopia::findDisplayFont(text);
                if (!var.isNull())
                    return var.value<QFont>();

                return defaultFont;
            }
        }
    }

    return defaultFont;
}


ConversationDelegate::ConversationDelegate(QObject* parent) 
    : QtopiaItemDelegate(parent)
{
}

ConversationDelegate::~ConversationDelegate()
{
}

// Allowing for alignment to one side, return the rect inside the bubble
static QRect maximumBubbleRect(const QRect &bounds, ConversationDelegate::Alignment alignment)
{
    // Do not use the full width, in order to bind the bubble to one side
    const int indentSpacing = bounds.width() / 8;

    QRect rect(bounds);
    rect.adjust(alignment == ConversationDelegate::Left ? horizontalBubbleMargin : indentSpacing, 
                verticalBubbleMargin, 
                alignment == ConversationDelegate::Left ? -indentSpacing : -horizontalBubbleMargin, 
                -verticalBubbleMargin);

    return rect;
}

void ConversationDelegate::paintBubble(QPainter *painter, const QRect &r, int msgWidth, Alignment align) const
{
    QRegion clip(r);

    QRect bubbleRect(maximumBubbleRect(r, align));

    // Constrain the bubble to wrap the text, if the text is short
    const int fullWidth = msgWidth + 2 * itemMargin;
    if (fullWidth < bubbleRect.width()) {
        if (align == Left) {
            bubbleRect.setRight(bubbleRect.left() + fullWidth);
        } else {
            bubbleRect.setLeft(bubbleRect.right() - fullWidth);
        }
    }

    qreal x = bubbleRect.x();
    qreal y = bubbleRect.y();
    qreal w = bubbleRect.width();
    qreal h = bubbleRect.height();

    qreal xRadius = 100 * qMin((qreal)10.0, w/2) / (w/2);
    qreal yRadius = 100 * qMin((qreal)10.0, h/2) / (h/2);

    qreal rxx2 = w*xRadius/100;
    qreal ryy2 = h*yRadius/100;

    painter->setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    if (align == Left) {
        path.arcMoveTo(x, y+h-ryy2, rxx2, ryy2, 2*90);
        path.arcTo(x, y+h-ryy2, rxx2, ryy2, 2*90, 90);
        path.arcTo(x+w-rxx2, y+h-ryy2, rxx2, ryy2, 3*90, 90);
        path.arcTo(x+w-rxx2, y, rxx2, ryy2, 0, 90);
        path.arcTo(x, y, rxx2, ryy2, 90, 90);
        path.lineTo(r.left(), y+ryy2/2);
        path.lineTo(x, y+ryy2/2+8);
    } else {
        path.arcMoveTo(x, y, rxx2, ryy2, 90);
        path.arcTo(x, y, rxx2, ryy2, 90, 90);
        path.arcTo(x, y+h-ryy2, rxx2, ryy2, 2*90, 90);
        path.arcTo(x+w-rxx2, y+h-ryy2, rxx2, ryy2, 3*90, 90);
        path.lineTo(x+w, y+ryy2/2+8);
        path.lineTo(x+w+horizontalBubbleMargin, y+ryy2/2);
        path.lineTo(x+w, y+ryy2/2);
        path.arcTo(x+w-rxx2, y, rxx2, ryy2/2, 0, 90);
    }
    path.closeSubpath();

    painter->drawPath(path);
}

void ConversationDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    static const bool rightToLeftMode(QtopiaApplication::layoutDirection() == Qt::RightToLeft);

    QMailMessageId id(qvariant_cast<QMailMessageId>(index.data(QMailMessageListModel::MessageIdRole)));
    QString detailText(qvariant_cast<QString>(index.data(QMailMessageListModel::MessageBodyTextRole)));

    // Fallback to the subject text
    if (detailText.isEmpty())
        detailText = qvariant_cast<QString>(index.data(QMailMessageListModel::MessageSubjectTextRole));

    QMailMessageMetaData metaData(id);
    const bool incomingMessage(metaData.status() & QMailMessage::Incoming);

    const ConversationDelegate::Alignment itemAlignment(((incomingMessage && !rightToLeftMode) || (!incomingMessage && rightToLeftMode)) ? Left : Right);

    QDateTime timeStamp(metaData.date().toLocalTime());
    QString timeStampText(timeStamp.toString("h:mmap dddd, MMMM d"));
    if (timeStamp.date().daysTo(QDate::currentDate()) > 365)
        timeStampText += timeStamp.toString(" yyyy");
    timeStampText += ':';

    QFont textFont(displayFont(detailText, option.font));
    QFontMetrics textMetrics(textFont);

    QFont timeStampFont(textFont);
    timeStampFont.setPointSize(textFont.pointSize() - 2);
    timeStampFont.setWeight(QFont::Bold);
    QFontMetrics timeStampMetrics(timeStampFont);

    // Find the maximum allowable rectangle within the speech bubble
    QRect bubbleRect(maximumBubbleRect(option.rect, itemAlignment));

    QRect textRect(bubbleRect);
    textRect.adjust(itemMargin, itemMargin, -itemMargin, -itemMargin);

    QRect timeStampRect(textRect);
    timeStampRect.setHeight(timeStampMetrics.lineSpacing());

    QRect subjectRect(textRect);
    subjectRect.setTop(timeStampRect.bottom() + lineSpacing);

    int detailFlags = Qt::AlignLeading | Qt::TextWordWrap;

    QRect subjectBounding(textMetrics.boundingRect(subjectRect, detailFlags, detailText));
    if (subjectBounding.width() > subjectRect.width()) {
        detailFlags = Qt::AlignLeading | Qt::TextWrapAnywhere;
        subjectBounding = textMetrics.boundingRect(subjectRect, detailFlags, detailText);
    }

    QFontMetrics timeTextMetrics(timeStampFont);
    QRect timeStampBounding(timeTextMetrics.boundingRect(timeStampRect, Qt::AlignLeading | Qt::TextWordWrap, timeStampText));

    if (!incomingMessage) {
        // This text will be drawn with usual alignment, but the entire text
        // block should be pushed against the opposite edge as much as possible
        if (subjectBounding.width() < subjectRect.width()) {
            int difference(subjectRect.width() - subjectBounding.width());

            if (rightToLeftMode)
                subjectRect.setRight(subjectRect.right() - difference);
            else
                subjectRect.setLeft(subjectRect.left() + difference);

            subjectRect.setWidth(subjectBounding.width());
        }
    }

    const bool isSelected((option.state & QStyle::State_Selected) == QStyle::State_Selected);

    QStyleOptionViewItemV3 opt = option;

    const QStyleOptionViewItemV2 *v2 = qstyleoption_cast<const QStyleOptionViewItemV2 *>(&option);
    opt.features = v2 ? v2->features : QStyleOptionViewItemV2::ViewItemFeatures(QStyleOptionViewItemV2::None);

    const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option);
    opt.locale = v3 ? v3->locale : QLocale(); 
    opt.widget = v3 ? v3->widget : 0;

    painter->save();
    painter->setClipRect(option.rect);

    painter->setBrush(option.palette.window());
    drawBackground(painter, opt, index);

    QBrush baseBrush(isSelected ? option.palette.highlight() : option.palette.base());
    QBrush textBrush(isSelected ? option.palette.highlightedText() : option.palette.text());

    painter->setBrush(baseBrush);
    painter->setPen(textBrush.color());

    paintBubble(painter, option.rect, qMax(timeStampBounding.width(), subjectBounding.width()), itemAlignment);

    int timeStampAlign = (incomingMessage ? Qt::AlignLeading : Qt::AlignTrailing);
    painter->setFont(timeStampFont);
    painter->setPen(textBrush.color().lighter(200));
    painter->drawText(timeStampRect, timeStampAlign, timeStampText);

    painter->setFont(textFont);
    painter->setPen(textBrush.color());
    painter->drawText(subjectRect, detailFlags, detailText);

    painter->restore();
}

void ConversationDelegate::drawBackground(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex&) const
{
    QPen pen(painter->pen());
    painter->setPen(Qt::NoPen);
    painter->drawRect(opt.rect);
    painter->setPen(pen);
}

QSize ConversationDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QString detailText(qvariant_cast<QString>(index.data(QMailMessageListModel::MessageBodyTextRole)));

    // Fallback to the subject text
    if (detailText.isEmpty())
        detailText = qvariant_cast<QString>(index.data(QMailMessageListModel::MessageSubjectTextRole));

    QFont textFont(displayFont(detailText, option.font));
    QFontMetrics textMetrics(textFont);

    QFont timeStampFont(textFont);
    timeStampFont.setPointSize(textFont.pointSize() - 2);
    timeStampFont.setWeight(QFont::Bold);
    QFontMetrics timeStampMetrics(timeStampFont);

    const int indentSpacing = option.rect.width() / 8;

    const int horizontalMargins = indentSpacing + horizontalBubbleMargin + 2 * itemMargin;
    const int verticalMargins =  2 * (verticalBubbleMargin + itemMargin);

    int maxTextWidth = option.rect.width() - horizontalMargins;

    int detailFlags = Qt::AlignLeading | Qt::TextWordWrap;

    QRect bounds(textMetrics.boundingRect(0, 0, maxTextWidth, INT_MAX, detailFlags, detailText));
    if (bounds.width() > maxTextWidth) {
        detailFlags = Qt::AlignLeading | Qt::TextWrapAnywhere;
        bounds = textMetrics.boundingRect(0, 0, maxTextWidth, INT_MAX, detailFlags, detailText);
    }

    return QSize(option.rect.width(), (timeStampMetrics.lineSpacing() + lineSpacing + bounds.height() + verticalMargins));
}

