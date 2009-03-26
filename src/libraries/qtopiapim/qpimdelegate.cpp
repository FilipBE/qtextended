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

#include "qpimdelegate.h"
#include <QPainter>
#include <QAbstractItemModel>
#include <QDebug>
#include "qtopiaapplication.h"
#include <qpixmapcache.h>

class QPimDelegateData
{
public:
    int ignored; // Padding.
};


/*!
  \class QPimDelegate
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule
  \preliminary

  \ingroup pim
  \brief The QPimDelegate class provides an abstract delegate for
         rendering multiple lines of text for a PIM record.

  QPimDelegate draws an item with the following features.
  \list
  \o A background.
  \o An optional number of decorations.
  \o A main line of text, displayed in bold at the top.
  \o A variable number of subsidiary lines of text, each consisting of a
    'header' string rendered in bold, and a 'value' string rendered in a
    normal weight font.  The subsidiary lines of text are rendered slightly
    smaller than the main line of text.  Each subsidiary line can be rendered
    independently, or all subsidiary lines in a given item can have the header
    and value strings lined up consistently.
  \o An optional foreground.
  \endlist

  All text lines will be elided if they are too wide.

  Much like QAbstractItemDelegate, this class is not intended to be used
  directly.  Subclasses are expected to override some or all of the
  customization functions providing the above pieces of information
  or style settings.

  \image qpimdelegate-subtexts.png "Sample image of QPimDelegate"

  \sa QAbstractItemDelegate, QContactDelegate, QPimRecord, {Pim Library}
*/

/*!
  Constructs a QPimDelegate, with the given \a parent.
*/
QPimDelegate::QPimDelegate(QObject *parent)
    : QAbstractItemDelegate(parent), d(new QPimDelegateData)
{

}

/*!
 Destroys a \c QPimDelegate.
*/
QPimDelegate::~QPimDelegate()
{
    delete d;
}

/*!
  Returns the string to be displayed as the main text element of the
  delegate.  The default implementation returns the \c DisplayRole of the
  supplied \a index, and ignores \a option.
*/
QString QPimDelegate::mainText(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    Q_UNUSED(option);

    return index.data(Qt::DisplayRole).toString();
}

/*!
  Returns the size hint for the whole delegate, which will include the
  space required for decorations, if any. The returned value is calculated
  by adding any space required for decorations to the given \a textSizeHint
  parameter.

  The default implementation returns the supplied \a textSizeHint, and
  ignores the supplied \a index and \a option parameters.
 */
QSize QPimDelegate::decorationsSizeHint(
        const QStyleOptionViewItem& option, const QModelIndex& index,
        const QSize& textSizeHint) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return textSizeHint;
}


/*!
  Returns a hint for the number of subsidiary lines of text
  to render for an item, which is used to calculate the sizeHint
  of this delegate.

  The default implementation obtains the actual list of subsidiary
  lines of text to render with the supplied \a option and \a index,
  and returns the size of this list.

  This method should be overridden if it can be slow to retrieve the list of
  subsidiary lines but fast to estimate the number of lines, for example,
  if all items are rendered with two subsidiary lines of text, but
  each subsidiary line of text requires a database lookup.

  \sa subTexts()
 */
int QPimDelegate::subTextsCountHint(
        const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return subTexts(option, index).count();
}

/*!
  Returns the list of subsidiary lines to render.  This is
  stored in a list of pairs of strings, where the first member
  of the pair is the "header" string, and the second member
  of the pair is the "value" string.

  If either member of the pair is a null QString (\c QString()),
  then the subsidiary line is considered to consist of a single
  string that will take up the entire line.  You can specify an
  empty QString (\c QString("")) if you wish to have blank space.

  The default implementation returns an empty list, and
  ignores the supplied \a index and \a option parameters.
 */
QList<StringPair> QPimDelegate::subTexts(
        const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QList<StringPair>();
}

/*!
  \enum QPimDelegate::BackgroundStyle

  Enumerates the ways that the background of an item can be rendered by the
  \l {QPimDelegate::}{drawBackground()} function.

  \value None
    No background will be drawn by this delegate.  The container view may
    still have applied a background.
  \value SelectedOnly
    Only the selected item will have its background drawn by the delegate.
    This uses the selected palette brush.
  \value Gradient
    The delegate draws a linear gradient from top to
    bottom in slightly different shades of either the style's
    highlight brush color (if the item is selected) or the base
    brush color.

  \image qpimdelegate-bgstyle.png "BackgroundStyle variations"

  \sa backgroundStyle()
 */

/*!
  Returns a value that indicates how the background of this item should be rendered.

  The default implementation returns QPimDelegate::SelectedOnly, and
  ignores the supplied \a index and \a option parameters.

  \sa drawBackground()
 */
QPimDelegate::BackgroundStyle QPimDelegate::backgroundStyle(
        const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QPimDelegate::SelectedOnly;
}

/*!
  \enum QPimDelegate::SubTextAlignment

  Enumerates the ways that the header and value pairs of the
  subsidiary lines can be rendered.

  \value Independent
    Each header and value pair is rendered independently, leading aligned, with
    the value string being rendered immediately after the header.
  \value CuddledPerItem
    All of the subsidiary lines of an item will be rendered with the same width
    for the header strings, and the header strings will be rendered trailing
    aligned within this space.  The value strings will be rendered leading
    aligned, so that the header and value strings for a subsidiary line will be
    cuddled together.

  \image qpimdelegate-subtextalignment.png "Examples of SubTextAlignment"

  \sa subTextAlignment()
*/

/*!
  Returns the alignment of the header and value pairs of the
  subsidiary lines of text.

  The default implementation returns QPimDelegate::Independent, and
  ignores the supplied \a index and \a option parameters.
 */
QPimDelegate::SubTextAlignment QPimDelegate::subTextAlignment(
        const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QPimDelegate::Independent;
}

/*!
  Paints any decorations, and assigns to the given lists of rectangles, which
  the caller can then use to align painted text, if required. The \a rtl argument
  is a convenience parameter that is true if the decorations should be painted
  right-to-left, or false if they should be painted left-to-right.
  This may affect which side of the rectangle a decoration is painted on.

  The rectangle to paint in (using \a p) should be obtained from
  \a option (\a {option}.rect) and the \a index of the item to paint.

  This function should return (in the \a leadingFloats and \a trailingFloats
  parameters) lists of rectangles that the rendered text will be wrapped
  around.  Rectangles on the leading side of the text should be returned in
  \a leadingFloats, and rectangles on the trailing side should be returned
  in \a trailingFloats.  This allows some flexibility in deciding whether
  decorations should be drawn behind or beside the text.

  The default implementation does not draw anything, and returns two empty
  lists.

  \sa textRectangle()
 */
void QPimDelegate::drawDecorations(QPainter* p, bool rtl, const QStyleOptionViewItem &option,
                                   const QModelIndex& index,
                                   QList<QRect>& leadingFloats,
                                   QList<QRect>& trailingFloats) const
{
    Q_UNUSED(p);
    Q_UNUSED(rtl);
    Q_UNUSED(option);
    Q_UNUSED(index);
    Q_UNUSED(leadingFloats);
    Q_UNUSED(trailingFloats);
}

/*!
  Returns the rectangle that a line of text should be rendered in, given the following parameters.
  \list
  \o \a top - The top pixel coordinate of a line of text.
  \o \a height - The height of a line of text, in pixels.
  \o \a entireRect - The area of the entire delegate.
  \o \a leftFloats - The areas for any decorations at the left of the delegate.
  \o \a rightFloats - The areas for any decorations at the right of the delegate.
  \endlist

  Note that the \l {QPimDelegate::}{drawDecorations()} function returns lists of
  rectangles that are RTL independent (e.g. leading and trailing instead of
  left and right).  The lists of rectangles passed to this function are in
  absolute terms (left and right) - for an LTR language, they are equivalent, but
  for an RTL language the two lists will be exchanged.

  This function is used while rendering each line of text, including any subsidiary lines.

  \sa drawDecorations()
 */
QRect QPimDelegate::textRectangle(const QRect& entireRect,
                                         const QList<QRect>& leftFloats,
                                         const QList<QRect>& rightFloats,
                                         int top, int height) const
{
    QRect r = QRect(entireRect.x(), top, entireRect.width(), height);
    foreach(QRect leftFloat, leftFloats) {
        if (leftFloat.right() > r.left() && r.intersects(leftFloat))
            r.setLeft(leftFloat.right());
    }
    foreach(QRect rightFloat, rightFloats) {
        if (rightFloat.left() < r.right() && r.intersects(rightFloat))
            r.setRight(rightFloat.left());
    }
    return r;
}


/*!
  Paints the background of the item.

  The rectangle to paint in (using \a p) should be obtained from
  \a option (\a {option}.rect) and the \a index of the item to paint.

  The default implementation fetches the background style to paint
  by calling \l {QPimDelegate::}{backgroundStyle()} for the given
  \a option and \a index.

 */
void QPimDelegate::drawBackground(QPainter *p,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex& index) const
{
    BackgroundStyle style = backgroundStyle(option, index);

    if (style == None)
        return;

    bool selected = (option.state & QStyle::State_Selected) == QStyle::State_Selected;
    QBrush baseBrush = selected ? option.palette.highlight() : option.palette.base();

    if (selected) {
        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
            ? QPalette::Normal : QPalette::Disabled;

        QString key = QLatin1String("_QPIMD_");
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
        p->drawPixmap(option.rect.topLeft(), pm);
    } else {
        if (selected && !option.rect.isEmpty()) {
            QPainter::RenderHints rh = p->renderHints();
            p->setRenderHint(QPainter::Antialiasing);
            QColor color = option.palette.color(QPalette::Highlight);
            p->setPen(color);
            p->setBrush(baseBrush);
            int adj = p->pen().width();
            QRect rr = option.rect.adjusted(adj/2, adj/2, -(adj - (adj/2)), -(adj - (adj/2)));
            if (!rr.isEmpty())
                p->drawRoundRect(rr, 800/rr.width(),800/rr.height());
            p->setRenderHints(rh);
        }
#ifdef QTOPIA_HOMEUI
        else {
            QColor bg = option.palette.color(QPalette::Base);
            QLinearGradient bgg(option.rect.x(), option.rect.y(),
                    option.rect.x(), option.rect.bottom());
            bgg.setColorAt(0.0f, bg.lighter(200));
            bgg.setColorAt(1.0f, bg);
            p->fillRect(option.rect, bgg);
        }
#endif
    }
}

/*!
  Paints the foreground of the item.

  The rectangle to paint in (using \a p) should be obtained from
  \a option (\a {option}.rect) and the \a index of the item to paint.

  This function is called after painting all other visual elements
  (background, decorations, text etc) and could be used to apply a
  transparent effect to the rendered items.

  The default implementation does not paint anything.
 */
void QPimDelegate::drawForeground(QPainter *p,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex& index) const
{
    Q_UNUSED(p);
    Q_UNUSED(option);
    Q_UNUSED(index);
}


/*!
  Paints the item specified by \a index, using the supplied \a painter and
  style option \a option.

  The default implementation will first draw the background and decorations,
  then the text items, and finally, any foreground items. All the
  drawing is accomplished using basic methods in this class.

  \sa drawBackground(), drawDecorations(), mainFont(),
      secondaryFont(), secondaryHeaderFont(),
      subTextAlignment(), textRectangle(), drawForeground()
 */
void QPimDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const
{
    bool rtl = QtopiaApplication::layoutDirection() == Qt::RightToLeft ;

    QList<StringPair> subTexts = this->subTexts(option, index);
    SubTextAlignment subAlign = subTextAlignment(option, index);

    painter->save();

    bool sel = (option.state & QStyle::State_Selected)==QStyle::State_Selected;
    QBrush baseBrush = sel ? option.palette.highlight() : option.palette.base();
    QBrush textBrush = sel ? option.palette.highlightedText() : option.palette.text();

    // Clip
    painter->setClipRect(option.rect);

    // Draw the background and icons first
    QList<QRect> leftFloats;
    QList<QRect> rightFloats;
    drawBackground(painter, option, index);
    drawDecorations(painter, rtl, option, index, leftFloats, rightFloats);

    // Swap here so we don't need to for textRectangle
    if (rtl)
        qSwap(leftFloats, rightFloats);

    // Make sure we have a valid pen/brush
    painter->setBrush(baseBrush);
    painter->setPen(textBrush.color());

    int y = option.rect.y() + 1;
    int height = option.rect.height() - 2;
    QRect space;

    // select the header font
    QFont fmain = mainFont(option, index);
    QFontMetrics fmainM(fmain);
    painter->setFont(fmain);

    if (subTexts.count() < 1) {
        // We vertically center align this text if there CuddledPerListis less
        // than one line of extra space
        if (height < 2 * fmainM.lineSpacing())
            y += (height - fmainM.lineSpacing()) / 2;

        space = textRectangle(option.rect, leftFloats, rightFloats, y, fmainM.lineSpacing());
        painter->drawText(space, Qt::AlignLeading, fmainM.elidedText(mainText(option, index), option.textElideMode, space.width()));
    } else {
        /* Sub labels are present */
        QFont fsubheader = secondaryHeaderFont(option, index);
        QFont fsubvalue = secondaryFont(option, index);

        QFontMetrics fsubvalueM(fsubvalue);
        QFontMetrics fsubheaderM(fsubheader);
        StringPair subLine;

        // We vertically center align all the text if there is less than one line of extra space [as measured by the big font]
        if (height < (2 * fmainM.lineSpacing()) + (subTexts.count() * (fsubheaderM.lineSpacing() + 1)))
            y += (height - (fmainM.lineSpacing() + (subTexts.count() * (fsubheaderM.lineSpacing() + 1)))) / 2;

        space = textRectangle(option.rect, leftFloats, rightFloats, y, fmainM.lineSpacing());

        painter->drawText(space, Qt::AlignLeading, fmainM.elidedText(mainText(option, index), option.textElideMode, space.width()));
        y += fmainM.lineSpacing();

        int headerWidth = 0;
        /* First, calculate the width of all the header sections, if the style is cuddly */
        if (subAlign == CuddledPerItem) {
            foreach(subLine, subTexts) {
                if (!subLine.first.isEmpty() && !subLine.second.isNull()) {
                    int w = fsubheaderM.boundingRect(subLine.first).width();
                    if (w > headerWidth)
                        headerWidth = w;
                }
            }
        }

        /* Now draw! */
        QList<StringPair>::const_iterator it = subTexts.begin();
        while (y + fsubheaderM.lineSpacing() <= option.rect.bottom() && it != subTexts.end()) {
            space = textRectangle(option.rect, leftFloats, rightFloats, y, fsubheaderM.lineSpacing() + 1);
            subLine = *it++;
            if (!subLine.first.isNull()) {
                if (!subLine.second.isEmpty()) {
                    QRect headerRect = space;
                    QRect valueRect = space;

                    QString subText = fsubheaderM.elidedText(subLine.first, Qt::ElideRight, headerRect.width());
                    if (subAlign == Independent)
                        headerWidth = fsubheaderM.width(subText);

                    if (rtl) {
                        headerRect.setLeft(headerRect.right() - headerWidth);
                        valueRect.setRight(headerRect.left());
                    } else {
                        headerRect.setWidth(headerWidth);
                        valueRect.setLeft(headerRect.right());
                    }
                    painter->setFont(fsubheader);
                    painter->drawText(headerRect, Qt::AlignTrailing, subText);
                    painter->setFont(fsubvalue);
                    painter->drawText(valueRect, Qt::AlignLeading, fsubvalueM.elidedText(subLine.second, option.textElideMode, valueRect.width()));
                } else {
                    painter->setFont(fsubheader);
                    painter->drawText(space, Qt::AlignLeading, fsubheaderM.elidedText(subLine.first, option.textElideMode, space.width()));
                }
            } else {
                if (!subLine.second.isEmpty()) {
                    painter->setFont(fsubvalue);
                    painter->drawText(space, Qt::AlignLeading, fsubvalueM.elidedText(subLine.second, option.textElideMode, space.width()));
                }
            }

            y += fsubheaderM.lineSpacing() + 1;
        }
    }

    drawForeground(painter, option, index);
    painter->restore();
}

/*!
  Returns the font to use for painting the main label text of the item
  for the given \a index and style option \a option.

  The default behavior is to return the font of the style option
  \a option, modified for bold rendering, and to ignore \a index.
 */
QFont QPimDelegate::mainFont(const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    QFont fmain = option.font;
    fmain.setWeight(QFont::Bold);

    return fmain;
}

/*!
  Returns the font to use for painting the subsidiary header texts of the item,
  for the given \a index and style option \a option.

  The default return value is a bold font that is at least two point sizes smaller
  than the font of the style option \a option. The supplied \a index is ignored
  in this case.

  \sa mainFont(), secondaryFont()
 */
QFont QPimDelegate::secondaryHeaderFont(const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    QFont fsubheader = differentFont(option.font, -1);
    fsubheader.setWeight(QFont::Bold);

    return fsubheader;
}

/*!
  Returns the font to use for painting the subsidiary value texts of the item,
  for the given \a index and style option \a option.

  The default return value is a font that is at least two point sizes smaller
  than the font of the style option \a option. The supplied \a index is ignored
  in this case.

  \sa mainFont(), secondaryHeaderFont()
 */
QFont QPimDelegate::secondaryFont(const QStyleOptionViewItem &option, const QModelIndex& index) const
{
    Q_UNUSED(index);

    return differentFont(option.font, -1);
}

/*!
  Attempts to return a font that is similar to \a start but has the
  given size difference of \a step. If no matching font for the given
  \a step value is found, it will try increasingly larger/smaller fonts
  (depending on whether \a step was originally positive or negative).  If
  no suitable font is found after trying different sizes, the original font
  \a start will be returned.
 */
QFont QPimDelegate::differentFont(const QFont& start, int step) const
{
    int osize = QFontMetrics(start).lineSpacing();
    QFont f = start;
    for (int t=1; t<6; t++) {
        int newSize = f.pointSize() + step;
        if ( newSize > 0 )
            f.setPointSize(f.pointSize()+step);
        else
            return start; // we cannot find a font -> return old one
        step += step < 0 ? -1 : +1;
        QFontMetrics fm(f);
        if ( fm.lineSpacing() != osize )
            break;
    }
    return f;
}

/*!
  Returns the delegate's size hint for a specific index \a index and item style \a option.

  \sa decorationsSizeHint()
 */
QSize QPimDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFont main = mainFont(option, index);
    QFont shfont = secondaryHeaderFont(option, index);
    QFont subfont = secondaryFont(option, index);

    QFontMetrics fm(main);
    QFontMetrics shfm(shfont);
    QFontMetrics sfm(subfont);

    // Get the text sizes..
    QSize mainSize(fm.width(mainText(option, index)), fm.lineSpacing());

    // Need to measure the subtexts properly
    QSize subSizes;

    QList<StringPair> subTexts = this->subTexts(option, index);
    SubTextAlignment subAlign = subTextAlignment(option, index);
    int headerWidth = 0;
    StringPair subLine;

    /* First, calculate the width of all the header sections, if the style is cuddly */
    if (subAlign == CuddledPerItem) {
        foreach(subLine, subTexts) {
            if (!subLine.first.isEmpty() && !subLine.second.isNull()) {
                int w = shfm.boundingRect(subLine.first).width();
                if (w > headerWidth)
                    headerWidth = w;
            }
        }
    }

    /* Now get the width of each item */
    QList<StringPair>::const_iterator it = subTexts.begin();
    while (it != subTexts.end()) {
        subLine = *it++;
        if (!subLine.first.isNull()) {
            if (!subLine.second.isEmpty()) {
                if (subAlign == Independent)
                    headerWidth = shfm.width(subLine.first);
                int lineWidth = headerWidth + sfm.width(subLine.second);
                subSizes.rwidth() = qMax(subSizes.width(), lineWidth);
            } else
                subSizes.rwidth() = qMax(subSizes.width(), shfm.width(subLine.first));
        } else {
            if (!subLine.second.isEmpty())
                subSizes.rwidth() = qMax(subSizes.width(), sfm.width(subLine.second));
        }

        subSizes.rheight() += shfm.lineSpacing();
    }
    QSize sh(qMax(mainSize.width(), subSizes.width()), 1 + mainSize.height() + 1 + subSizes.height() + 1);

    // allow the decoration size hint to be included
    return decorationsSizeHint(option, index, sh);
}
