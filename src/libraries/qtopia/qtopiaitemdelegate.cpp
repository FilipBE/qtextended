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

#include "qtopiaitemdelegate.h"
#include "qtopiastyle.h"

#include <qabstractitemmodel.h>
#include <qapplication.h>
#include <qbrush.h>
#include <qpainter.h>
#include <qpalette.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstyle.h>
#include <qdatetime.h>
#include <qstyleoption.h>
#include <qevent.h>
#include <qpixmap.h>
#include <qpixmapcache.h>
#include <qmetaobject.h>
#include <qtextlayout.h>
#include <private/qtextengine_p.h>
#include <qdebug.h>
#include <qlocale.h>

#include <limits.h>

// I curse QItemDelegate, especially non-virtual drawBackground().
// Unfortunately this class is basically a copy of QItemDelegate.

class QtopiaItemDelegatePrivate
{
public:
    QtopiaItemDelegatePrivate() {}

    inline QIcon::Mode iconMode(QStyle::State state) const
        {
            if (!(state & QStyle::State_Enabled)) return QIcon::Disabled;
            return QIcon::Normal;
        }

    inline QIcon::State iconState(QStyle::State state) const
        { return state & QStyle::State_Open ? QIcon::On : QIcon::Off; }

    inline static QString replaceNewLine(QString text)
        {
            const QChar nl = QLatin1Char('\n');
            for (int i = 0; i < text.count(); ++i)
                if (text.at(i) == nl)
                    text[i] = QChar::LineSeparator;
            return text;
        }

    static QString valueToText(const QVariant &value, const QStyleOptionViewItemV3 &option);

    QRect textLayoutBounds(const QStyleOptionViewItemV2 &options) const;
    QSizeF doTextLayout(int lineWidth) const;
    mutable QTextLayout textLayout;
    mutable QTextOption textOption;

    const QWidget *widget(const QStyleOptionViewItem &option) const
    {
        if (const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option))
            return v3->widget;

        return 0;
    }
};

QRect QtopiaItemDelegatePrivate::textLayoutBounds(const QStyleOptionViewItemV2 &option) const
{
    QRect rect = option.rect;
    const bool wrapText = option.features & QStyleOptionViewItemV2::WrapText;
    switch (option.decorationPosition) {
    case QStyleOptionViewItem::Left:
    case QStyleOptionViewItem::Right:
        rect.setWidth(wrapText && rect.isValid() ? rect.width() : (QFIXED_MAX));
        break;
    case QStyleOptionViewItem::Top:
    case QStyleOptionViewItem::Bottom:
        rect.setWidth(wrapText ? option.decorationSize.width() : (QFIXED_MAX));
        break;
    }

    return rect;
}

QSizeF QtopiaItemDelegatePrivate::doTextLayout(int lineWidth) const
{
    qreal height = 0;
    qreal widthUsed = 0;
    textLayout.beginLayout();
    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;
        line.setLineWidth(lineWidth);
        line.setPosition(QPointF(0, height));
        height += line.height();
        widthUsed = qMax(widthUsed, line.naturalTextWidth());
    }
    textLayout.endLayout();
    return QSizeF(widthUsed, height);
}

/*!
    \class QtopiaItemDelegate
    \inpublicgroup QtBaseModule

    \brief The QtopiaItemDelegate class provides display and editing facilities for
    data items from a model.

    \ingroup model-view


    QtopiaItemDelegate can be used to provide custom display features and editor
    widgets for item views based on QAbstractItemView subclasses. Using a
    delegate for this purpose allows the display and editing mechanisms to be
    customized and developed independently from the model and view.

    The QtopiaItemDelegate class is one of the \l{Model/View Classes}.

    When displaying items from a custom model in a standard view, it is
    often sufficient to simply ensure that the model returns appropriate
    data for each of the \l{Qt::ItemDataRole}{roles} that determine the
    appearance of items in views. The default delegate used by Qt's
    standard views uses this role information to display items in most
    of the common forms expected by users. However, it is sometimes
    necessary to have even more control over the appearance of items than
    the default delegate can provide.

    This class provides default implementations of the functions for
    painting item data in a view, and editing data obtained from a model
    with the Qt Extended look and feel.

    Default implementations of the paint() and sizeHint() virtual functions,
    defined in QAbstractItemDelegate, are provided to ensure that the
    delegate implements the correct basic behavior expected by views. You
    can reimplement these functions in subclasses to customize the
    appearance of items.

    Delegates can be used to manipulate item data in two complementary ways:
    by processing events in the normal manner, or by implementing a
    custom editor widget. The item delegate takes the approach of providing
    a widget for editing purposes that can be supplied to
    QAbstractItemView::setDelegate() or the equivalent function in
    subclasses of QAbstractItemView.

    Only the standard editing functions for widget-based delegates are
    reimplemented here: editor() returns the widget used to change data
    from the model; setEditorData() provides the widget with data to
    manipulate; updateEditorGeometry() ensures that the editor is displayed
    correctly with respect to the item view; setModelData() returns the
    updated data to the model; releaseEditor() indicates that the user has
    completed editing the data, and that the editor widget can be destroyed.

    \section1 Standard Roles and Data Types

    The default delegate used by the standard views supplied with Qt
    associates each standard role (defined by Qt::ItemDataRole and
    Qtopia::ItemDataRole) with certain
    data types. Models that return data in these types can influence the
    appearance of the delegate as described in the following table.

    \table
    \header \o Role \o Accepted Types
    \omit
    \row    \o \l Qt::AccessibleDescriptionRole \o QString
    \row    \o \l Qt::AccessibleTextRole \o QString
    \endomit
    \row    \o \l Qt::BackgroundRole \o QBrush
    \row    \o \l Qt::BackgroundColorRole \o QColor (obsolete; use Qt::BackgroundRole instead)
    \row    \o \l Qt::CheckStateRole \o Qt::CheckState
    \row    \o \l Qt::DecorationRole \o QIcon and QColor
    \row    \o \l Qt::DisplayRole \o QString and types with a string representation
    \row    \o \l Qt::EditRole \o See QItemEditorFactory for details
    \row    \o \l Qt::FontRole \o QFont
    \row    \o \l Qt::SizeHintRole \o QSize
    \omit
    \row    \o \l Qt::StatusTipRole \o
    \endomit
    \row    \o \l Qt::TextAlignmentRole \o Qt::Alignment
    \row    \o \l Qt::ForegroundRole \o QBrush
    \row    \o \l Qt::TextColorRole \o QColor (obsolete; use Qt::ForegroundRole instead)
    \omit
    \row    \o \l Qt::ToolTipRole
    \row    \o \l Qt::WhatsThisRole
    \row    \o \l Qtopia::AdditionalDecorationRole \o QIcon
    \endomit
    \endtable

    If the default delegate does not allow the level of customization that
    you need, either for display purposes or for editing data, it is possible to
    subclass QtopiaItemDelegate to implement the desired behavior.

    \section1 Subclassing

    When subclassing QtopiaItemDelegate to create a delegate that displays items
    using a custom renderer, it is important to ensure that the delegate can
    render items suitably for all the required states; e.g. selected,
    disabled, checked. The documentation for the paint() function contains
    some hints to show how this can be achieved.

    Custom editing features for can be added by subclassing QtopiaItemDelegate and
    reimplementing createEditor(), setEditorData(), setModelData(), and
    updateEditorGeometry(). This process is described in the
    \l{Spin Box Delegate example}.

    \sa {Delegate Classes}, QItemDelegate, QAbstractItemDelegate, {Spin Box Delegate Example},
        {Settings Editor Example}, {Icons Example}
*/

/*!
    Constructs an item delegate with the given \a parent.
*/

QtopiaItemDelegate::QtopiaItemDelegate(QObject *parent)
    : QItemDelegate(parent), d(new QtopiaItemDelegatePrivate)
{

}

/*!
    Destroys the item delegate.
*/

QtopiaItemDelegate::~QtopiaItemDelegate()
{
    delete d;
}

QString QtopiaItemDelegatePrivate::valueToText(const QVariant &value, const QStyleOptionViewItemV3 &option)
{
    QString text;
    switch (value.type()) {
        case QVariant::Double:
            text = option.locale.toString(value.toDouble());
            break;
        case QVariant::Int:
        case QVariant::LongLong:
            text = option.locale.toString(value.toLongLong());
            break;
        case QVariant::UInt:
        case QVariant::ULongLong:
            text = option.locale.toString(value.toULongLong());
            break;
        case QVariant::Date:
            text = option.locale.toString(value.toDate(), QLocale::ShortFormat);
            break;
        case QVariant::Time:
            text = option.locale.toString(value.toTime(), QLocale::ShortFormat);
            break;
        case QVariant::DateTime:
            text = option.locale.toString(value.toDateTime().date(), QLocale::ShortFormat);
            text += QLatin1Char(' ');
            text += option.locale.toString(value.toDateTime().time(), QLocale::ShortFormat);
            break;
        default:
            text = replaceNewLine(value.toString());
            break;
    }
    return text;
}

/*!
    Renders the delegate using the given \a painter and style \a option for
    the item specified by \a index.

    When reimplementing this function in a subclass, you should update the area
    held by the option's \l{QStyleOption::rect}{rect} variable, using the
    option's \l{QStyleOption::state}{state} variable to determine the state of
    the item to be displayed, and adjust the way it is painted accordingly.
    For example, a selected item may need to be displayed differently to
    unselected items.

    After painting, you should ensure that the painter is returned to its
    the state it was supplied in when this function was called. For example,
    it may be useful to call QPainter::save() before painting and
    QPainter::restore() afterwards.

    \sa QStyle::State
*/
void QtopiaItemDelegate::paint(QPainter *painter,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    Q_ASSERT(index.isValid());

    QStyleOptionViewItemV3 opt = setOptions(index, option);

    const QStyleOptionViewItemV2 *v2 = qstyleoption_cast<const QStyleOptionViewItemV2 *>(&option);
    opt.features = v2 ? v2->features
                    : QStyleOptionViewItemV2::ViewItemFeatures(QStyleOptionViewItemV2::None);
    const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option);
    opt.locale = v3 ? v3->locale : QLocale();
    opt.widget = v3 ? v3->widget : 0;

    // prepare
    painter->save();
    if (hasClipping())
        painter->setClipRect(opt.rect);

    // get the data and the rectangles

    QRect decorationRect;
    QVariant decoration = index.data(Qt::DecorationRole);
    if (decoration.isValid()) {
        switch (decoration.type()) {
        case QVariant::Icon: {
            QIcon icon = qvariant_cast<QIcon>(decoration);
            QIcon::Mode mode = d->iconMode(option.state);
            QIcon::State state = d->iconState(option.state);
#ifdef QTOPIA_HOMEUI_WIDE
            const QSize size = option.decorationSize;
#else
            const QSize size = icon.actualSize(option.decorationSize, mode, state);
#endif
            decorationRect = QRect(QPoint(0, 0), size);
            break; }
        case QVariant::Pixmap: {
            QPixmap pixmap = qvariant_cast<QPixmap>(decoration);
            decorationRect = QRect(QPoint(0, 0), pixmap.size());
            break; }
        default:
            decorationRect = QRect(QPoint(0, 0), option.decorationSize);
            break;
        }
    }

    QRect addDecorationRect;
    QVariant addDecoration = index.data(Qtopia::AdditionalDecorationRole);
    if (addDecoration.isValid()) {
        int addDecorationSize = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
        QSize size(addDecorationSize, addDecorationSize);
        switch (addDecoration.type()) {
        case QVariant::Icon: {
            QIcon icon = qvariant_cast<QIcon>(addDecoration);
            QIcon::Mode mode = d->iconMode(option.state);
            QIcon::State state = d->iconState(option.state);
            size = icon.actualSize(size, mode, state);
            break; }
        case QVariant::Pixmap: {
            QPixmap pixmap = qvariant_cast<QPixmap>(addDecoration);
            size = pixmap.size();
            break; }
        default:
            break;
        }
        addDecorationRect = QRect(QPoint(0, 0), size);
    }

    QString text;
    QRect displayRect;
    QVariant value = index.data(Qt::DisplayRole);
    if (value.isValid()) {
        text = QtopiaItemDelegatePrivate::valueToText(value, opt);
        displayRect = textRectangle(painter, d->textLayoutBounds(opt), opt.font, text);
    }

    QRect checkRect;
    Qt::CheckState checkState = Qt::Unchecked;
    value = index.data(Qt::CheckStateRole);
    if (value.isValid()) {
        checkState = static_cast<Qt::CheckState>(value.toInt());
        checkRect = check(opt, opt.rect, value);
    }

    // do the layout

    doLayout(opt, &checkRect, &decorationRect, &displayRect, &addDecorationRect, false);

    // draw the item

    drawBackground(painter, opt, index);
    drawCheck(painter, opt, checkRect, checkState);
    drawDecoration(painter, opt, decorationRect, decoration);
    drawDisplay(painter, opt, displayRect, text);
    drawAdditionalDecoration(painter, opt, addDecorationRect, addDecoration);
    drawFocus(painter, opt, displayRect);

    // done
    painter->restore();
}

/*!
    Returns the size needed by the delegate to display the item
    specified by \a index, taking into account the style information
    provided by \a option.

    When reimplementing this function, note that in case of text
    items, QtopiaItemDelegate adds a margin (i.e. 2 *
    QStyle::PM_FocusFrameHMargin) to the length of the text.
*/

QSize QtopiaItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    QVariant value = index.data(Qt::SizeHintRole);
    if (value.isValid())
        return qvariant_cast<QSize>(value);
    QRect decorationRect = rect(option, index, Qt::DecorationRole);
    QRect displayRect = rect(option, index, Qt::DisplayRole);
    QRect checkRect = rect(option, index, Qt::CheckStateRole);
    QRect addDecorationRect = rect(option, index, Qtopia::AdditionalDecorationRole);

    doLayout(option, &checkRect, &decorationRect, &displayRect, &addDecorationRect, true);

    return (decorationRect|displayRect|checkRect|addDecorationRect).size();
}

/*!
   Renders the item view \a text within the rectangle specified by \a rect
   using the given \a painter and style \a option.
*/

void QtopiaItemDelegate::drawDisplay(QPainter *painter, const QStyleOptionViewItem &option,
                                const QRect &rect, const QString &text) const
{
    if (text.isEmpty())
        return;

    QPen pen = painter->pen();
    QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                              ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
        cg = QPalette::Inactive;
    if (option.state & QStyle::State_Selected) {
        painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
    } else {
        painter->setPen(option.palette.color(cg, QPalette::Text));
    }

    if (option.state & QStyle::State_Editing) {
        painter->save();
        painter->setPen(option.palette.color(cg, QPalette::Text));
        painter->drawRect(rect.adjusted(0, 0, -1, -1));
        painter->restore();
    }

    const QStyleOptionViewItemV3 opt = option;

    const QWidget *widget = d->widget(option);
    QStyle *style = widget ? widget->style() : QApplication::style();
    const int textMargin = style->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, widget) + 1;
    QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0); // remove width padding
    const bool wrapText = opt.features & QStyleOptionViewItemV2::WrapText;
    d->textOption.setWrapMode(wrapText ? QTextOption::WordWrap : QTextOption::ManualWrap);
    d->textOption.setTextDirection(option.direction);
    d->textOption.setAlignment(QStyle::visualAlignment(option.direction, option.displayAlignment));
    d->textLayout.setTextOption(d->textOption);
    d->textLayout.setFont(option.font);
    d->textLayout.setText(QtopiaItemDelegatePrivate::replaceNewLine(text));

    QSizeF textLayoutSize = d->doTextLayout(textRect.width());

    if (textRect.width() < textLayoutSize.width()
        || textRect.height() < textLayoutSize.height()) {
        QString elided;
        int start = 0;
        int end = text.indexOf(QChar::LineSeparator, start);
        if (end == -1) {
            elided += option.fontMetrics.elidedText(text, option.textElideMode, textRect.width());
        } else {
            while (end != -1) {
                elided += option.fontMetrics.elidedText(text.mid(start, end - start),
                                                        option.textElideMode, textRect.width());
                elided += QChar::LineSeparator;
                start = end + 1;
                end = text.indexOf(QChar::LineSeparator, start);
            }
            //let's add the last line (after the last QChar::LineSeparator)
            elided += option.fontMetrics.elidedText(text.mid(start),
                                                    option.textElideMode, textRect.width());
            if (end != -1)
                elided += QChar::LineSeparator;
        }
        d->textLayout.setText(elided);
        textLayoutSize = d->doTextLayout(textRect.width());
    }

    const QSize layoutSize(textRect.width(), int(textLayoutSize.height()));
    const QRect layoutRect = QStyle::alignedRect(option.direction, option.displayAlignment,
                                                  layoutSize, textRect);
    // if we still overflow even after eliding the text, enable clipping
    if (!hasClipping() && (textRect.width() < textLayoutSize.width()
                           || textRect.height() < textLayoutSize.height())) {
        painter->save();
        painter->setClipRect(layoutRect);
        d->textLayout.draw(painter, layoutRect.topLeft(), QVector<QTextLayout::FormatRange>(), layoutRect);
        painter->restore();
    } else {
        d->textLayout.draw(painter, layoutRect.topLeft(), QVector<QTextLayout::FormatRange>(), layoutRect);
    }
}

/*!
    Renders the \a pixmap within the rectangle specified by
    \a rect using the given \a painter and style \a option.
*/
void QtopiaItemDelegate::drawDecoration(QPainter *painter, const QStyleOptionViewItem &option,
                            const QRect &rect, const QPixmap &pixmap) const
{
    QPoint p = QStyle::alignedRect(option.direction, option.decorationAlignment,
            pixmap.size(), rect).topLeft();
    painter->drawPixmap(p, pixmap);
}

/*!
    Renders the \a decoration within the rectangle specified by
    \a rect using the given \a painter and style \a option.
*/
void QtopiaItemDelegate::drawDecoration(QPainter *painter, const QStyleOptionViewItem &option,
                                   const QRect &rect, const QVariant &decoration) const
{
    if (!rect.isValid())
        return;

    // if we have an icon, we ignore the pixmap
    switch (decoration.type()) {
    case QVariant::Icon: {
        QIcon icon = qvariant_cast<QIcon>(decoration);
        QIcon::Mode mode = d->iconMode(option.state);
        QIcon::State state = d->iconState(option.state);
        icon.paint(painter, rect, option.decorationAlignment, mode, state);
        break; }
    case QVariant::Pixmap: {
        QPixmap pixmap = qvariant_cast<QPixmap>(decoration);
        drawDecoration(painter, option, rect, pixmap);
        break; }
    case QVariant::Color: {
        painter->fillRect(rect, qvariant_cast<QColor>(decoration));
        break; }
    default:
        break;
    }
}

/*!
    Renders the additional \a decoration within the rectangle specified by
    \a rect using the given \a painter and style \a option.
*/
void QtopiaItemDelegate::drawAdditionalDecoration(QPainter *painter, const QStyleOptionViewItem &option,
                                   const QRect &rect, const QVariant &decoration) const
{
    if (!rect.isValid())
        return;

    // if we have an icon, we ignore the pixmap
    switch (decoration.type()) {
    case QVariant::Icon: {
        QIcon icon = qvariant_cast<QIcon>(decoration);
        QIcon::Mode mode = d->iconMode(option.state);
        QIcon::State state = d->iconState(option.state);
        icon.paint(painter, rect, option.decorationAlignment, mode, state);
        break; }
    case QVariant::Pixmap: {
        QPixmap pixmap = qvariant_cast<QPixmap>(decoration);
        QPoint p = QStyle::alignedRect(option.direction, option.decorationAlignment,
                                    pixmap.size(), rect).topLeft();
        painter->drawPixmap(p, pixmap);
        break; }
    case QVariant::Color: {
        painter->fillRect(rect, qvariant_cast<QColor>(decoration));
        break; }
    default:
        break;
    }
}

/*!
    Renders the region within the rectangle specified by \a rect, indicating
    that it has the focus, using the given \a painter and style \a option.
*/

void QtopiaItemDelegate::drawFocus(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QRect &rect) const
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(rect);
    /* Focus should not be necessary in Qtopia.
    if ((option.state & QStyle::State_HasFocus) == 0 || !rect.isValid())
        return;
    QStyleOptionFocusRect o;
    o.QStyleOption::operator=(option);
    o.rect = rect;
    o.state |= QStyle::State_KeyboardFocusChange;
    QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
                              ? QPalette::Normal : QPalette::Disabled;
    o.backgroundColor = option.palette.color(cg, (option.state & QStyle::State_Selected)
                                             ? QPalette::Highlight : QPalette::Window);
    const QWidget *widget = d->widget(option);
//    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter, widget);
    */
}

/*!
    Renders a check indicator within the rectangle specified by \a
    rect, using the given \a painter and style \a option, using the
    given \a state.
*/

void QtopiaItemDelegate::drawCheck(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QRect &rect, Qt::CheckState state) const
{
    if (!rect.isValid())
        return;

    QStyleOptionViewItem opt(option);
    opt.rect = rect;
    opt.state = opt.state & ~QStyle::State_HasFocus;

    switch (state) {
    case Qt::Unchecked:
        opt.state |= QStyle::State_Off;
        break;
    case Qt::PartiallyChecked:
        opt.state |= QStyle::State_NoChange;
        break;
    case Qt::Checked:
        opt.state |= QStyle::State_On;
        break;
    }

    const QWidget *widget = d->widget(option);
    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_IndicatorViewItemCheck, &opt, painter, widget);
}

/*!
    Renders the item background for the given \a index,
    using the given \a painter and style \a option.
*/

void QtopiaItemDelegate::drawBackground(QPainter *painter,
                                   const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    if (option.rect.width() <= 0 || option.rect.height() <= 0)
        return;

    if (option.state & QStyle::State_Selected) {
        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
        if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
            cg = QPalette::Inactive;

        QRect rect(option.rect);

        QString key = QLatin1String("_QID_");
        key += QString::number(rect.width());
        key += QString::number(rect.height());
        key += QString::number(int(option.palette.color(cg, QPalette::Highlight).rgba()));

        QPixmap pm;
        if (!QPixmapCache::find(key, pm)) {
            QSize size = rect.size();
            QImage img(size, QImage::Format_ARGB32_Premultiplied);
            img.fill(0x00000000);
            QPainter p(&img);
            p.setRenderHint(QPainter::Antialiasing);
            QColor color = option.palette.color(cg, QPalette::Highlight);
            p.setPen(color);
            QLinearGradient bgg(QPoint(0,0), QPoint(0, size.height()));
            bgg.setColorAt(0.0f, color.lighter(175));
            bgg.setColorAt(0.49f, color.lighter(105));
            bgg.setColorAt(0.5f, color);
            p.setBrush(bgg);
            QtopiaStyle::drawRoundRect(&p, QRect(QPoint(0,0),size), 8, 8);
            pm = QPixmap::fromImage(img);
            QPixmapCache::insert(key, pm);
        }
        painter->drawPixmap(rect.topLeft(), pm);
    } else {
        QVariant value = index.data(Qt::BackgroundRole);
        if (qVariantCanConvert<QBrush>(value)) {
            QPointF oldBO = painter->brushOrigin();
            painter->setBrushOrigin(option.rect.topLeft());
            painter->fillRect(option.rect, qvariant_cast<QBrush>(value));
            painter->setBrushOrigin(oldBO);
        }
#ifdef QTOPIA_HOMEUI
        else {
            QColor bg = option.palette.color(QPalette::Base);
            QLinearGradient bgg(option.rect.x(), option.rect.y(),
                                option.rect.x(), option.rect.bottom());
            bgg.setColorAt(0.0f, bg);
            bgg.setColorAt(1.0f, bg.darker(160));
            painter->fillRect(option.rect, bgg);
        }
#endif
    }
}


/*!
    \internal
*/

void QtopiaItemDelegate::doLayout(const QStyleOptionViewItem &option,
                             QRect *checkRect, QRect *pixmapRect, QRect *textRect,
                             QRect *addDecorationRect, bool hint) const
{
    Q_ASSERT(checkRect && pixmapRect && textRect);
    const QWidget *widget = d->widget(option);
    QStyle *style = widget ? widget->style() : QApplication::style();
    const bool hasCheck = checkRect->isValid();
    const bool hasPixmap = pixmapRect->isValid();
    const bool hasText = textRect->isValid();
    const bool hasAddDec = addDecorationRect->isValid();
    const int textMargin = hasText ? style->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, widget) + 1 : 0;
    const int pixmapMargin = hasPixmap ? style->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, widget) + 1 : 0;
    int x = option.rect.left();
    int y = option.rect.top();
    int w, h;

    textRect->adjust(-textMargin, 0, textMargin, 0); // add width padding
    if (textRect->height() == 0 && !hasPixmap)
        textRect->setHeight(option.fontMetrics.height());

    int addDecorationMargin = 0;
    const QStyleOptionViewItemV3 *v3 = qstyleoption_cast<const QStyleOptionViewItemV3 *>(&option);
    if (v3 && hasAddDec)
        addDecorationMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, v3->widget) + 1;
    QSize addDecSize = addDecorationRect->size();

    QSize pm(0, 0);
    if (hasPixmap) {
        pm = pixmapRect->size();
        pm.rwidth() += 2 * pixmapMargin;
    }
    if (hint) {
        h = qMax(checkRect->height(), qMax(textRect->height(), pm.height()+2));
        if (option.decorationPosition == QStyleOptionViewItem::Left
            || option.decorationPosition == QStyleOptionViewItem::Right) {
            w = textRect->width() + pm.width() + addDecSize.width() + addDecorationMargin;
        } else {
            w = qMax(textRect->width() + addDecSize.width() + addDecorationMargin, pm.width());
        }
    } else {
        w = option.rect.width();
        h = option.rect.height();
    }

    int cw = 0;
    QRect check;
    if (hasCheck) {
        cw = checkRect->width() + 2 * textMargin;
        if (hint) w += cw;
        if (option.direction == Qt::RightToLeft) {
            check.setRect(x + w - cw, y, cw, h);
        } else {
            check.setRect(x, y, cw, h);
        }
    }

    // at this point w should be the *total* width

    QRect display;
    QRect decoration;
    QRect addDec;
    switch (option.decorationPosition) {
    case QStyleOptionViewItem::Top: {
        if (hasPixmap)
            pm.setHeight(pm.height() + pixmapMargin); // add space
        h = hint ? textRect->height() : h - pm.height();

        if (option.direction == Qt::RightToLeft) {
            decoration.setRect(x, y, w - cw, pm.height());
            addDec.setRect(x, y + pm.height(), addDecSize.width(), h);
            display.setRect(addDec.right() + addDecorationMargin, y + pm.height(), w - cw - addDecSize.width(), h);
        } else {
            decoration.setRect(x + cw, y, w - cw, pm.height());
            display.setRect(x + cw, y + pm.height(), w - cw - addDecSize.width() - addDecorationMargin, h);
            addDec.setRect(display.right() + addDecorationMargin, y + pm.height(), addDecSize.width(), h);
        }
        break; }
    case QStyleOptionViewItem::Bottom: {
        if (hasText)
            textRect->setHeight(textRect->height() + textMargin); // add space
        h = hint ? textRect->height() + pm.height() : h;

        if (option.direction == Qt::RightToLeft) {
            addDec.setRect(x, y, addDecSize.width(), textRect->height());
            display.setRect(addDec.right() + addDecorationMargin, y, w - cw - addDecSize.width() - addDecorationMargin, textRect->height());
            decoration.setRect(x, y + textRect->height(), w - cw, h - textRect->height());
        } else {
            display.setRect(x + cw, y, w - cw - addDecSize.width() - addDecorationMargin, textRect->height());
            addDec.setRect(display.right() + addDecorationMargin, y, addDecSize.width(), textRect->height());
            decoration.setRect(x + cw, y + textRect->height(), w - cw, h - textRect->height());
        }
        break; }
    case QStyleOptionViewItem::Left: {
        if (option.direction == Qt::LeftToRight) {
            decoration.setRect(x + cw, y, pm.width(), h);
            display.setRect(decoration.right() + 1, y, w - pm.width() - cw, h);
            addDec.setRect(display.right() - addDecSize.width() - addDecorationMargin, y, addDecSize.width(), h);
            display.setRight(addDec.left());
        } else {
            addDec.setRect(x + addDecorationMargin, y, addDecSize.width(), h);
            display.setRect(addDec.right(), y, w - pm.width() - addDecSize.width() - cw, h);
            decoration.setRect(display.right() + 1, y, pm.width(), h);
        }
        break; }
    case QStyleOptionViewItem::Right: {
        if (option.direction == Qt::LeftToRight) {
            display.setRect(x + cw, y, w - pm.width() - cw - addDecSize.width() - addDecorationMargin, h);
            decoration.setRect(display.right() + 1, y, pm.width(), h);
            addDec.setRect(decoration.right() + addDecorationMargin, y , addDecSize.width(), h);
        } else {
            addDec.setRect(x, y, addDecSize.width(), h);
            decoration.setRect(addDec.right() + addDecorationMargin, y, pm.width(), h);
            display.setRect(decoration.right() + 1, y, w - pm.width() - cw, h);
        }
        break; }
    default:
        qWarning("doLayout: decoration position is invalid");
        decoration = *pixmapRect;
        break;
    }

    if (!hint) { // we only need to do the internal layout if we are going to paint
        *checkRect = QStyle::alignedRect(option.direction, Qt::AlignCenter,
                                         checkRect->size(), check);
        *pixmapRect = QStyle::alignedRect(option.direction, option.decorationAlignment,
                                          pixmapRect->size(), decoration);
        *addDecorationRect = QStyle::alignedRect(option.direction, option.decorationAlignment,
                                          addDecSize, addDec);
        // the text takes up all available space, unless the decoration is not shown as selected
        if (option.showDecorationSelected)
            *textRect = display;
        else
            *textRect = QStyle::alignedRect(option.direction, option.displayAlignment,
                                            textRect->size().boundedTo(display.size()), display);
    } else {
        *checkRect = check;
        *pixmapRect = decoration;
        *addDecorationRect = addDec;
        *textRect = display;
    }
}

// hacky but faster version of "QString::sprintf("%d-%d", i, enabled)"
static QString qPixmapSerial(quint64 i, bool enabled)
{
    ushort arr[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', '0' + enabled };
    ushort *ptr = &arr[16];

    while (i > 0) {
        // hey - it's our internal representation, so use the ascii character after '9'
        // instead of 'a' for hex
        *(--ptr) = '0' + i % 16;
        i >>= 4;
    }

    return QString::fromUtf16(ptr, int(&arr[sizeof(arr) / sizeof(ushort)] - ptr));
}

/*!
  \internal
  Returns the selected version of the given \a pixmap using the given \a palette.
  The \a enabled argument decides whether the normal or disabled highlight color of
  the palette is used.
*/
QPixmap *QtopiaItemDelegate::selected(const QPixmap &pixmap, const QPalette &palette, bool enabled) const
{
    QString key = qPixmapSerial(qt_pixmap_id(pixmap), enabled);
    QPixmap *pm = QPixmapCache::find(key);
    if (!pm) {
        QImage img = pixmap.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);

        QColor color = palette.color(enabled ? QPalette::Normal : QPalette::Disabled,
                                     QPalette::Highlight);
        color.setAlphaF(0.3);

        QPainter painter(&img);
        painter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        painter.fillRect(0, 0, img.width(), img.height(), color);
        painter.end();

        QPixmap selected = QPixmap(QPixmap::fromImage(img));
        QPixmapCache::insert(key, selected);
        pm = QPixmapCache::find(key);
    }
    return pm;
}

/*!
  \internal
*/

QRect QtopiaItemDelegate::rect(const QStyleOptionViewItem &option,
                          const QModelIndex &index, int role) const
{
    QVariant value = index.data(role);
    if (role == Qt::CheckStateRole)
        return check(option, option.rect, value);
    if (value.isValid()) {
        switch (value.type()) {
        case QVariant::Invalid:
            break;
        case QVariant::Pixmap:
            return QRect(QPoint(0, 0), qvariant_cast<QPixmap>(value).size());
        case QVariant::Image:
            return QRect(QPoint(0, 0), qvariant_cast<QImage>(value).size());
        case QVariant::Icon: {
            QIcon::Mode mode = d->iconMode(option.state);
            QIcon::State state = d->iconState(option.state);
            QIcon icon = qvariant_cast<QIcon>(value);
            QSize size = icon.actualSize(option.decorationSize, mode, state);
            return QRect(QPoint(0, 0), size); }
        case QVariant::Color:
            return QRect(QPoint(0, 0), option.decorationSize);
        case QVariant::String:
        default: {
            QString text = QtopiaItemDelegatePrivate::valueToText(value, option);
            value = index.data(Qt::FontRole);
            QFont fnt = qvariant_cast<QFont>(value).resolve(option.font);
            return textRectangle(0, d->textLayoutBounds(option), fnt, text); }
        }
    }
    return QRect();
}

/*!
  \internal

  Note that on Mac, if /usr/include/AssertMacros.h is included prior to QtopiaItemDelegate,
  and the application is building in debug mode, the check(assertion) will conflict
  with QtopiaItemDelegate::check.

  To avoid this problem, add 

  #ifdef check
	#undef check
  #endif

  after including AssertMacros.h
*/
QRect QtopiaItemDelegate::check(const QStyleOptionViewItem &option,
                           const QRect &bounding, const QVariant &value) const
{
    if (value.isValid()) {
        QStyleOptionButton opt;
        opt.QStyleOption::operator=(option);
        opt.rect = bounding;
        const QWidget *widget = d->widget(option); // cast
        QStyle *style = widget ? widget->style() : QApplication::style();
        return style->subElementRect(QStyle::SE_ViewItemCheckIndicator, &opt, widget);
    }
    return QRect();
}

/*!
  \internal
*/
QRect QtopiaItemDelegate::textRectangle(QPainter * /*painter*/, const QRect &rect,
                                   const QFont &font, const QString &text) const
{
    d->textOption.setWrapMode(QTextOption::WordWrap);
    d->textLayout.setTextOption(d->textOption);
    d->textLayout.setFont(font);
    d->textLayout.setText(QtopiaItemDelegatePrivate::replaceNewLine(text));
    const QSize size = d->doTextLayout(rect.width()).toSize();
    // ###: textRectangle should take style option as argument
    const int textMargin = QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1;
    return QRect(0, 0, size.width() + 2 * textMargin, size.height());
}

/*!
    \fn bool QtopiaItemDelegate::eventFilter(QObject *editor, QEvent *event)

    Returns true if the given \a editor is a valid QWidget and the
    given \a event is handled; otherwise returns false. The following
    key press events are handled by default:

    \list
        \o \gui Tab
        \o \gui Backtab
        \o \gui Enter
        \o \gui Return
        \o \gui Esc
    \endlist

    In the case of \gui Tab, \gui Backtab, \gui Enter and \gui Return
    key press events, the \a editor's data is comitted to the model
    and the editor is closed. If the \a event is a \gui Tab key press
    the view will open an editor on the next item in the
    view. Likewise, if the \a event is a \gui Backtab key press the
    view will open an editor on the \i previous item in the view.

    If the event is a \gui Esc key press event, the \a editor is
    closed \i without committing its data.

    \sa commitData(), closeEditor()
*/

bool QtopiaItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    return QItemDelegate::eventFilter(object, event);
}


/*!
  \internal
*/

QStyleOptionViewItem QtopiaItemDelegate::setOptions(const QModelIndex &index,
                                               const QStyleOptionViewItem &option) const
{
    QStyleOptionViewItem opt = option;

    // set font
    QVariant value = index.data(Qt::FontRole);
    if (value.isValid()){
        opt.font = qvariant_cast<QFont>(value).resolve(opt.font);
        opt.fontMetrics = QFontMetrics(opt.font);
    }

    // set text alignment
    value = index.data(Qt::TextAlignmentRole);
    if (value.isValid())
        opt.displayAlignment = (Qt::Alignment)value.toInt();

    // set foreground brush
    value = index.data(Qt::ForegroundRole);
    if (qVariantCanConvert<QBrush>(value))
        opt.palette.setBrush(QPalette::Text, qvariant_cast<QBrush>(value));

    return opt;
}

