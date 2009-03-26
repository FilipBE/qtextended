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

#include "qmailmessagedelegate.h"
#include "qmailmessagelistmodel.h"
#include <QtopiaApplication>
#include <QPainter>
#include <QPixmapCache>
#include <QStylePainter>

#ifdef QTOPIA_HOMEUI
#include "qmailmessage.h"
#include <QSortFilterProxyModel>
#include <private/homewidgets_p.h>
#endif

static int scrollbarSize = 4;

static QStyleOptionViewItemV3 getStyleOptions(const QStyleOptionViewItem &option)
{
    QStyleOptionViewItemV3 opt = option;

    const QStyleOptionViewItemV2 *v2 = qstyleoption_cast<const QStyleOptionViewItemV2 *>(&option);
    opt.features = v2 ? v2->features : QStyleOptionViewItemV2::ViewItemFeatures(QStyleOptionViewItemV2::None);

    const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option);
    opt.locale = v3 ? v3->locale : QLocale(); 
    opt.widget = v3 ? v3->widget : 0;

    return opt;
}

static QPoint iconPosition(const QRect &boundingRect, const QSize &iconSize)
{
    return QPoint(boundingRect.left() + ((boundingRect.width() - iconSize.width()) / 2), 
                  boundingRect.top() + ((boundingRect.height() - iconSize.height()) / 2));
}

#ifdef QTOPIA_HOMEUI
// Copied from qtopiatheming/qthemetextitem:
void drawTextOutline(QPainter *painter, const QRect &rect, int flags, const QString &text, const QColor &color)
{
    // Cheaper to paint into pixmap and blit that four times than
    // to draw text four times.

    QImage img(rect.size(), QImage::Format_ARGB32_Premultiplied);
    img.fill(qRgba(0, 0, 0, 0));

    QPainter ppm(&img);
    ppm.setFont(painter->font());
    ppm.setPen(color);
    ppm.translate(QPoint(0, 0) - rect.topLeft());
    ppm.drawText(rect, flags, text);

    QPoint pos(rect.topLeft());
    pos += QPoint(-1, 0);
    painter->drawImage(pos, img);
    pos += QPoint(2, 0);
    painter->drawImage(pos, img);
    pos += QPoint(-1, -1);
    painter->drawImage(pos, img);
    pos += QPoint(0, 2);
    painter->drawImage(pos, img);
}

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
#endif


class QMailMessageDelegatePrivate 
{

public:
    QMailMessageDelegatePrivate(QMailMessageDelegate::DisplayMode mode)
    : displayMode(mode), displaySelectionState(false) {}

    ~QMailMessageDelegatePrivate(){};

public:
    QMailMessageDelegate::DisplayMode displayMode;
    bool displaySelectionState;
};

/*!
  \class QMailMessageDelegate 
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

  \preliminary
  \ingroup messaginglibrary 
  \brief The QMailMessageDelegate class provides a visual representation of a message suitable for 
  display in a list of messages.

  The QMailMessageDelegate provides a common visual representation of a message suitable for 
  display in a list of messages. This class inherits from QAbstractItemDelegate and is designed to be used
  in conjunction with the Qt view classes such as QListView.

  QMailMessageDelegate supports more than one painting mode used to display message elements in 
  different configurations.

  \sa QListView, QAbstractItemDelegate
*/

/*!
  \enum QMailMessageDelegate::DisplayMode

  Represents the display modes of the delegate. The DisplayMode represents different message painting 
  configurations of the delegate.

  \value AddressbookMode 
    The message entry is painted in the style used by the Addressbook application. The message entry 
    is painted with an icon representing the message direction, the text of the message address, 
    the message type icon and the text of the message subject underneath.
  \value QtmailMode 
    The message entry is painted in the style used by the Qt Extended messaging application. The message entry 
    is painted with an icon representing the message status, the message type icon, the text of the 
    message address and the text of the message subject underneath.
*/

/*!
  Creates a QMailMessageDelegate with parent \a parent that paints the contents of a message 
  in the DisplayMode \a mode.
*/

QMailMessageDelegate::QMailMessageDelegate(DisplayMode mode, QWidget* parent)
:
QtopiaItemDelegate(parent),
d(new QMailMessageDelegatePrivate(mode))
{
}

/*!
    Deletes the QMailMessageDelegate.
*/

QMailMessageDelegate::~QMailMessageDelegate()
{
    delete d; d = 0;
}

/*!
  Returns the display mode the delegate is painting in.
*/

QMailMessageDelegate::DisplayMode QMailMessageDelegate::displayMode() const
{
    return d->displayMode;
}

/*!
  Sets the display mode the delegate is painting to \a mode.
*/

void QMailMessageDelegate::setDisplayMode(DisplayMode mode)
{
    d->displayMode = mode;
}

/*!
  Returns true if the delegate is displaying the selection state of messages.
*/

bool QMailMessageDelegate::displaySelectionState() const
{
    return d->displaySelectionState;
}

/*!
  Sets the delegate to display message selection states if \a set is true.
*/

void QMailMessageDelegate::setDisplaySelectionState(bool set)
{
    d->displaySelectionState = set;
}

/*!
  \reimp
*/

void QMailMessageDelegate::paint(QPainter* painter,
                                 const QStyleOptionViewItem& option,
                                 const QModelIndex& index) const
{
    static const bool rightToLeftMode(QtopiaApplication::layoutDirection() == Qt::RightToLeft);
    const int borderSpacing = 1;

    QIcon statusIcon;
    QIcon typeIcon(qvariant_cast<QIcon>(index.data(QMailMessageListModel::MessageTypeIconRole)));

    QString addressText(qvariant_cast<QString>(index.data(QMailMessageListModel::MessageAddressTextRole)));
    QString detailText(qvariant_cast<QString>(index.data(QMailMessageListModel::MessageSubjectTextRole)));

    Qt::CheckState checkState(static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt()));

    QFont mainFont(option.font);
    QFont subtextFont(mainFont);
    subtextFont.setPointSize(mainFont.pointSize() - 2);

    QFontMetrics mainMetrics(mainFont);
    QFontMetrics subtextMetrics(subtextFont);

    int widthReduction = scrollbarSize + (2 * borderSpacing);

    QRect itemRect(option.rect);
    itemRect.setWidth(itemRect.width() - widthReduction);
    itemRect.translate((rightToLeftMode ? (borderSpacing + scrollbarSize) : borderSpacing), 0);

    QRect checkRect(itemRect);
    QRect textRect(itemRect);

    if (d->displaySelectionState) {
        const int checkReduction = 4;
        const int checkSpacing = 2;

        const int checkSize = mainMetrics.lineSpacing() - checkReduction;

        checkRect.setTop(checkRect.top() + ((checkRect.height() - checkSize) / 2));
        checkRect.setHeight(checkSize);

        if (rightToLeftMode) {
            checkRect.setLeft(checkRect.right() - checkSize);
            textRect.setRight(checkRect.left() - checkSpacing);
        } else {
            checkRect.setRight(checkRect.left() + checkSize);
            textRect.setLeft(checkRect.right() + checkSpacing);
        }
    }

    QRect statusIconRect(textRect);
    QRect typeIconRect(textRect);

    QSize statusIconSize;
    QSize typeIconSize;

    const int listIconSpan(QtopiaApplication::style()->pixelMetric(QStyle::PM_ListViewIconSize));
    const int smallIconSpan(QtopiaApplication::style()->pixelMetric(QStyle::PM_SmallIconSize));

    if(d->displayMode == QtmailMode)
    {
        statusIcon = qvariant_cast<QIcon>(index.data(QMailMessageListModel::MessageStatusIconRole));

        QSize maxSize(smallIconSpan,smallIconSpan);
        statusIconSize = statusIcon.actualSize(maxSize);
        typeIconSize = typeIcon.actualSize(maxSize);

        static int iconSpacing = 2;
        const int textWidthReduction = statusIconSize.width() + typeIconSize.width() + (2 * iconSpacing);

        if(rightToLeftMode) {
            textRect.setWidth(textRect.width() - textWidthReduction);
            statusIconRect.setLeft(statusIconRect.right() - statusIconSize.width());
            typeIconRect.setLeft(textRect.right() + iconSpacing);
            typeIconRect.setRight(statusIconRect.left() - iconSpacing);
        } else {
            statusIconRect.setWidth(statusIconSize.width());
            textRect.setLeft(textRect.right() - (textRect.width() - textWidthReduction));
            typeIconRect.setLeft(statusIconRect.right() + iconSpacing);
            typeIconRect.setRight(textRect.left() - iconSpacing);
        }
    }
    else //AddressbookMode
    {
        statusIcon = qvariant_cast<QIcon>(index.data(QMailMessageListModel::MessageDirectionIconRole));
        statusIconSize = statusIcon.actualSize(QSize(listIconSpan,listIconSpan));
        typeIconSize = typeIcon.actualSize(QSize(smallIconSpan,smallIconSpan));

        mainFont.setWeight(QFont::Bold);
        subtextFont.setWeight(QFont::Bold);
 
        static int iconSpacing = 4;

        if(rightToLeftMode) {
            typeIconRect.setWidth(typeIconSize.width());
            statusIconRect.setLeft(statusIconRect.right() - statusIconSize.width());
            textRect.setLeft(typeIconRect.right() + iconSpacing);
            textRect.setRight(statusIconRect.left() - iconSpacing);
        } else {
            statusIconRect.setWidth(statusIconSize.width());
            typeIconRect.setLeft(typeIconRect.right() - typeIconSize.width());
            textRect.setLeft(statusIconRect.right() + iconSpacing);
            textRect.setRight(typeIconRect.left() - iconSpacing);
        }
    }

    QRect headerRect(textRect);
    headerRect.setTop(headerRect.top() + borderSpacing);
    headerRect.setHeight(mainMetrics.lineSpacing());

    QRect subRect(textRect);
    subRect.setTop(subRect.bottom() - subtextMetrics.lineSpacing() + borderSpacing);
    subRect.setHeight(subtextMetrics.lineSpacing());

    const bool isSelected((option.state & QStyle::State_Selected) == QStyle::State_Selected);

    QBrush baseBrush(isSelected ? option.palette.brush(QPalette::Highlight) : option.palette.brush(QPalette::Base));
    QColor textColor(isSelected ? option.palette.color(QPalette::HighlightedText) : option.palette.color(QPalette::Text));

    QStyleOptionViewItemV3 opt = getStyleOptions(option);

    painter->save();
    painter->setClipRect(option.rect);

    QtopiaItemDelegate::drawBackground(painter, opt, index);

    if (d->displaySelectionState)
        QtopiaItemDelegate::drawCheck(painter, opt, checkRect, checkState);

    painter->drawPixmap(iconPosition(statusIconRect, statusIconSize), statusIcon.pixmap(statusIconSize));

    painter->drawPixmap(iconPosition(typeIconRect, typeIconSize), typeIcon.pixmap(typeIconSize));

    painter->setBrush(baseBrush);
    painter->setPen(textColor);

    painter->setFont(mainFont);
    painter->drawText(headerRect, Qt::AlignLeading, mainMetrics.elidedText(addressText, option.textElideMode, headerRect.width()));

    painter->setFont(subtextFont);
    painter->drawText(subRect, Qt::AlignLeading, subtextMetrics.elidedText(detailText, option.textElideMode, subRect.width()));

    painter->restore();
}

/*!
  \reimp
*/

QSize QMailMessageDelegate::sizeHint(const QStyleOptionViewItem& option,
                                     const QModelIndex& index) const
{
    Q_UNUSED(index)

    static const int listIconSpan(qApp->style()->pixelMetric(QStyle::PM_ListViewIconSize));
    const int borderSpacing = 1;

    QFont mainFont(option.font);
    mainFont.setWeight(QFont::Bold);

    QFont subtextFont(mainFont);
    subtextFont.setPointSize(mainFont.pointSize() - 2);

    QFontMetrics mainMetrics(mainFont);
    QFontMetrics subtextMetrics(subtextFont);

    return QSize(option.rect.width(), qMax((mainMetrics.lineSpacing() + borderSpacing + subtextMetrics.lineSpacing()), listIconSpan) + 2);
}


#ifdef QTOPIA_HOMEUI

class QtopiaHomeMailMessageDelegatePrivate 
{

public:
    QtopiaHomeMailMessageDelegatePrivate(QtopiaHomeMailMessageDelegate::DisplayMode mode, QWidget *parent)
    : displayMode(mode), displaySelectionState(false), mParent(parent), imageCache(0) {}

    ~QtopiaHomeMailMessageDelegatePrivate(){};

public:
    QtopiaHomeMailMessageDelegate::DisplayMode displayMode;
    bool displaySelectionState;

    QWidget *mParent;
    QImage *imageCache;
};

/*!
  \class QtopiaHomeMailMessageDelegate 
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtPimModule

  \preliminary
  \ingroup messaginglibrary 
  \brief The QtopiaHomeMailMessageDelegate class provides a visual representation of a message suitable for 
  display in a list of messages.

  The QtopiaHomeMailMessageDelegate provides a common visual representation of a message suitable for 
  display in a list of messages. This class inherits from QAbstractItemDelegate and is designed to be used
  in conjunction with the Qt view classes such as QListView.

  QtopiaHomeMailMessageDelegate supports more than one painting mode used to display message elements in 
  different configurations.

  \sa QListView, QAbstractItemDelegate
*/

/*!
  \enum QtopiaHomeMailMessageDelegate::DisplayMode

  Represents the display modes of the delegate. The DisplayMode represents different message painting 
  configurations of the delegate.

  \value QtmailMode 
    The message entry is painted in the style used by the Qt Extended messaging application.
  \value QtmailUnifiedMode 
    The message entry is painted in the style used by the Qt Extended messaging application, when 
    a unified presentation of incoming and outgoing messages is displayed.
  \value AddressbookMode 
    The message entry is painted in the style used by the Addressbook application.
*/

/*!
  Creates a delegate object with parent \a parent that paints the contents of a message 
  in the DisplayMode \a mode.
*/

QtopiaHomeMailMessageDelegate::QtopiaHomeMailMessageDelegate(DisplayMode mode, QWidget* parent)
:
QtopiaItemDelegate(parent),
d(new QtopiaHomeMailMessageDelegatePrivate(mode, parent))
{
}

/*!
    Deletes the delegate.
*/

QtopiaHomeMailMessageDelegate::~QtopiaHomeMailMessageDelegate()
{
    delete d; d = 0;
}

/*!
  Returns the display mode the delegate is painting in.
*/

QtopiaHomeMailMessageDelegate::DisplayMode QtopiaHomeMailMessageDelegate::displayMode() const
{
    return d->displayMode;
}

/*!
  Sets the display mode the delegate is painting to \a mode.
*/

void QtopiaHomeMailMessageDelegate::setDisplayMode(DisplayMode mode)
{
    d->displayMode = mode;
}

/*!
  Returns true if the delegate is displaying the selection state of messages.
*/

bool QtopiaHomeMailMessageDelegate::displaySelectionState() const
{
    return d->displaySelectionState;
}

/*!
  Sets the delegate to display message selection states if \a set is true.
*/

void QtopiaHomeMailMessageDelegate::setDisplaySelectionState(bool set)
{
    d->displaySelectionState = set;
}

QFont QtopiaHomeMailMessageDelegate::titleFont(const QStyleOptionViewItem &option) const
{
    QFont fmain = option.font;
    fmain.setWeight(QFont::Bold);
    return fmain;
}

QRect QtopiaHomeMailMessageDelegate::replyButtonRect(const QRect &rect) const 
{
    if (!d->imageCache)
        return QRect();

    static const bool rightToLeftMode(QtopiaApplication::layoutDirection() == Qt::RightToLeft);

    const int width = d->imageCache->rect().width();
    if (rightToLeftMode) {
        return QRect(rect.left(), rect.y(), width, rect.height());
    } else {
        return QRect(rect.right() - width, rect.y(), width, rect.height());
    }
}

/*!
  \reimp
*/

void QtopiaHomeMailMessageDelegate::paint(QPainter* painter, 
                                          const QStyleOptionViewItem& option, 
                                          const QModelIndex& index) const
{
    static const bool rightToLeftMode(QtopiaApplication::layoutDirection() == Qt::RightToLeft);

    static const int smallIconSpan(QtopiaApplication::style()->pixelMetric(QStyle::PM_SmallIconSize));
    static const int lowerMargin = 4;

    const bool showCheckbox(d->displaySelectionState);
    const bool showReply((d->displayMode == AddressbookMode) && !d->displaySelectionState);

    const bool showTypeIcon(true);
    const bool showPresenceIcon(d->displayMode != AddressbookMode);
    const bool showAddressText(d->displayMode != AddressbookMode);
    const bool showDetailText(true);
    const bool showTimeStamp(true);
    const bool showDirectionIcon(d->displayMode == QtmailUnifiedMode);

    // Assemble our data elements
    QMailMessageId mailId(qvariant_cast<QMailMessageId>(index.data(QMailMessageListModel::MessageIdRole)));
    QMailMessageMetaData metaData(mailId);

    const bool unread((metaData.status() & (QMailMessage::Read | QMailMessage::ReadElsewhere)) == 0);

    Qt::CheckState checkState(Qt::Unchecked);
    if (showCheckbox) {
        checkState = static_cast<Qt::CheckState>(index.data(Qt::CheckStateRole).toInt());
    }

    QIcon typeIcon;
    if (showTypeIcon) {
        typeIcon = qvariant_cast<QIcon>(index.data(QMailMessageListModel::MessageTypeIconRole));
    }

    QIcon presenceIcon;
    if (showPresenceIcon) {
        presenceIcon = qvariant_cast<QIcon>(index.data(QMailMessageListModel::MessagePresenceIconRole));
    }

    QString addressText;
    if (showAddressText) {
        addressText = qvariant_cast<QString>(index.data(QMailMessageListModel::MessageAddressTextRole));
    }
    QString detailText;
    if (showDetailText) {
        detailText += qvariant_cast<QString>(index.data(QMailMessageListModel::MessageSubjectTextRole));

        // Ensure that we display any detailed text as a single line
        detailText.replace('\n', ' ');
    }

    QString timeStamp;
    if (showTimeStamp) {
        QDateTime dateTime = metaData.date().toLocalTime();

        if (dateTime.date() == QDate::currentDate()) {
            timeStamp = QTimeString::localHM(dateTime.time(), QTimeString::Medium);
        } else {
            timeStamp = QTimeString::localMD(dateTime.date(), QTimeString::Medium);
        }
    }

    QIcon directionIcon;
    if (showDirectionIcon) {
        directionIcon = qvariant_cast<QIcon>(index.data(QMailMessageListModel::MessageDirectionIconRole));
    }

    QFont addressFont(displayFont(addressText, option.font));
    QFont detailFont(displayFont(detailText, option.font));

    addressFont.setBold(unread);
    detailFont.setBold(unread);

    QFontMetrics addressMetrics(addressFont);
    QFontMetrics detailMetrics(detailFont);
    QFontMetrics metrics(detailFont);

    if (showReply) {
        // If we don't have a button image that matches our item size, create one
        if (!d->imageCache || d->imageCache->rect().height() != option.rect.height()) {
            QString replyStr(tr("Reply"));
            const int replyWidth = metrics.boundingRect(option.rect, Qt::AlignLeading | Qt::AlignBottom, replyStr).width();

            const int replyMargin = 8;
            d->imageCache = new QImage(replyWidth + 2 * replyMargin,
                                       option.rect.height(),
                                       QImage::Format_ARGB32_Premultiplied);
            d->imageCache->fill(0);

            QStyleOptionButton buttonOption;
            buttonOption.initFrom(d->mParent);
            buttonOption.rect = d->imageCache->rect();
            HomeActionButton::setPaletteFromColor(&buttonOption.palette, QtopiaHome::Green);

            QStylePainter p(d->imageCache, d->mParent);
            p.setFont(option.font);

            HomeActionButton::paintButton(&buttonOption, &p, buttonOption.rect, QString());

            buttonOption.rect.setHeight(buttonOption.rect.height() - lowerMargin);
            p.drawText(buttonOption.rect, Qt::AlignHCenter | Qt::AlignBottom, replyStr);
        }
    }

    // Partition the drawing area into checkbox/data/reply-button
    QRect checkRect(option.rect);
    QRect dataRect(option.rect);
    QRect replyRect(option.rect);

    if (showCheckbox) {
        const int checkReduction = 4;
        const int checkSpacing = 2;

        const int checkSize = qMax(addressMetrics.lineSpacing(), detailMetrics.lineSpacing()) - checkReduction;

        checkRect.setTop(checkRect.top() + ((checkRect.height() - checkSize) / 2));
        checkRect.setHeight(checkSize);

        if (rightToLeftMode) {
            checkRect.setRight(checkRect.right() - checkSpacing);
            checkRect.setLeft(checkRect.right() - checkSize);
            dataRect.setRight(checkRect.left() - checkSpacing);
        } else {
            checkRect.setLeft(checkRect.left() + checkSpacing);
            checkRect.setRight(checkRect.left() + checkSize);
            dataRect.setLeft(checkRect.right() + checkSpacing);
        }
    }

    if (showReply) {
        const int replySpacing = 2;

        const int replyWidth = d->imageCache->width();

        if (rightToLeftMode) {
            replyRect.setRight(replyRect.left() + replyWidth);
            dataRect.setLeft(replyRect.right() + replySpacing);
        } else {
            replyRect.setLeft(replyRect.right() - replyWidth);
            dataRect.setRight(replyRect.left() - replySpacing);
        }
    }

    static int dataSpacing = 2;
    static int presenceSizeReduction = 8;

    const QSize maxIconSize(smallIconSpan, smallIconSpan);

    const QSize typeIconSize = typeIcon.actualSize(maxIconSize);
    const QSize presenceIconSize = QSize(maxIconSize.width() - presenceSizeReduction, maxIconSize.height() - presenceSizeReduction);
    const QSize directionIconSize = maxIconSize;

    const int timeWidth = metrics.boundingRect(option.rect, Qt::AlignCenter, timeStamp).width();

    QRect typeIconRect(dataRect);
    QRect presenceIconRect(dataRect);
    QRect textRect(dataRect);
    QRect timeStampRect(dataRect);
    QRect directionIconRect(dataRect);

    if (showTypeIcon) {
        QRect &successorRect(showPresenceIcon ? presenceIconRect : textRect);

        if (rightToLeftMode) {
            typeIconRect.setLeft(typeIconRect.right() - typeIconSize.width());
            successorRect.setRight(typeIconRect.left() - dataSpacing);
        } else {
            typeIconRect.setRight(typeIconRect.left() + typeIconSize.width());
            successorRect.setLeft(typeIconRect.right() + dataSpacing);
        }
    }

    if (showPresenceIcon) {
        if (rightToLeftMode) {
            presenceIconRect.setLeft(presenceIconRect.right() - presenceIconSize.width());
            textRect.setRight(presenceIconRect.left() - dataSpacing);
        } else {
            presenceIconRect.setRight(presenceIconRect.left() + presenceIconSize.width());
            textRect.setLeft(presenceIconRect.right() + dataSpacing);
        }
    }

    if (showAddressText || showDetailText) {
        // Size the text rectangle so we can align text items on their baseline
        textRect.setHeight(textRect.height() - lowerMargin);
    }

    if (showDirectionIcon) {
        QRect &predecessorRect(showTimeStamp ? timeStampRect : textRect);

        if (rightToLeftMode) {
            directionIconRect.setRight(directionIconRect.left() + directionIconSize.width());
            predecessorRect.setLeft(directionIconRect.right() + dataSpacing);
        } else {
            directionIconRect.setLeft(directionIconRect.right() - directionIconSize.width());
            predecessorRect.setRight(directionIconRect.left() - dataSpacing);
        }
    }

    if (showTimeStamp) {
        timeStampRect.setHeight(timeStampRect.height() - lowerMargin);

        if (rightToLeftMode) {
            timeStampRect.setRight(timeStampRect.left() + timeWidth);
            textRect.setLeft(timeStampRect.right() + dataSpacing);
        } else {
            timeStampRect.setLeft(timeStampRect.right() - timeWidth);
            textRect.setRight(timeStampRect.left() - dataSpacing);
        }
    }

    QBrush baseBrush(option.palette.brush(QPalette::Base));
    QColor textColor(option.palette.color(QPalette::Text));

    QStyleOptionViewItemV3 opt = getStyleOptions(option);

    painter->save();
    painter->setClipRect(option.rect);

    painter->setRenderHint(QPainter::Antialiasing);

    //don't display selections for home edition
    opt.state &= ~QStyle::State_Selected;

    QtopiaItemDelegate::drawBackground(painter, opt, index);

    if (showCheckbox) {
        QtopiaItemDelegate::drawCheck(painter, opt, checkRect, checkState);
    }

    if (showTypeIcon) {
        painter->drawPixmap(iconPosition(typeIconRect, typeIconSize), typeIcon.pixmap(typeIconSize));
    }

    if (showPresenceIcon) {
        painter->drawPixmap(iconPosition(presenceIconRect, presenceIconSize), presenceIcon.pixmap(presenceIconSize));
    }

    if (showAddressText || showDetailText | showTimeStamp) {
        painter->setBrush(baseBrush);
        painter->setPen(textColor);
    }

    if (showAddressText) {
        QRect addressRect;
        QString elided = addressMetrics.elidedText(addressText, option.textElideMode, textRect.width()/2);
        if (showDetailText)
            elided += QLatin1String(": ");

        painter->setFont(addressFont);

        painter->drawText(textRect, Qt::AlignLeading | Qt::AlignBottom, elided, &addressRect);

        textRect.setLeft(addressRect.right());
    }
    if (showDetailText) {
        painter->setFont(detailFont);

        QString elided = detailMetrics.elidedText(detailText, option.textElideMode, textRect.width());

        painter->drawText(textRect, Qt::AlignLeading | Qt::AlignBottom, elided);
    }

    if (showTimeStamp) {
        painter->setFont(detailFont);
        painter->drawText(timeStampRect, Qt::AlignLeading | Qt::AlignBottom, timeStamp);
    }

    if (showReply) {
        painter->drawImage(replyRect, *d->imageCache);
    }

    if (showDirectionIcon) {
        painter->drawPixmap(iconPosition(directionIconRect, directionIconSize), directionIcon.pixmap(directionIconSize));
    }

    painter->restore();
}

/*!
  \reimp
*/

QSize QtopiaHomeMailMessageDelegate::sizeHint(const QStyleOptionViewItem &option, 
                                              const QModelIndex &index) const
{
    static const int smallIconSpan(QtopiaApplication::style()->pixelMetric(QStyle::PM_SmallIconSize));
    static const int upperMargin = 8;
    static const int lowerMargin = 4;

    const bool showAddressText(d->displayMode != AddressbookMode);
    const bool showDetailText(true);

    QString addressText;
    if (showAddressText) {
        addressText = qvariant_cast<QString>(index.data(QMailMessageListModel::MessageAddressTextRole));
    }
    QString detailText;
    if (showDetailText) {
        detailText += qvariant_cast<QString>(index.data(QMailMessageListModel::MessageSubjectTextRole));
    }

    QFont addressFont(displayFont(addressText, option.font));
    QFont detailFont(displayFont(detailText, option.font));
    addressFont.setBold(true);
    detailFont.setBold(true);

    QFontMetrics addressMetrics(addressFont);
    QFontMetrics detailMetrics(detailFont);

    return QSize(option.rect.width(), qMax(qMax(addressMetrics.lineSpacing(), detailMetrics.lineSpacing()), smallIconSpan) + upperMargin + lowerMargin);
}

#endif // QTOPIA_HOMEUI

