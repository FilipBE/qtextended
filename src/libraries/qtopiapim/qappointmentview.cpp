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
#include <qappointmentmodel.h>
#include <QPainter>
#include <QMap>
#include <QList>
#include <QSet>
#include <QPixmap>
#include <QTextDocument>
#include <QTimer>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QStyle>
#include <qappointmentview.h>
#include <QDebug>

/*!
  \class QAppointmentDelegate
    \inpublicgroup QtUiModule
    \inpublicgroup QtMessagingModule
    \inpublicgroup QtTelephonyModule
    \inpublicgroup QtPimModule

  \ingroup pim
  \brief The QAppointmentDelegate class provides drawing of QAppointmentModel items (\l {QAppointment}{QAppointments}).

  By using QAppointmentDelegate, applications dealing with QAppointments can achieve a consistent
  look and feel.

  QAppointments are drawn with two sections per item.  There are optional icons on
  the right side of the rendered item, and text on the left.  The text and icons are rendered
  vertically centred within the item if there is enough space.  The icons are
  rendered starting at the top right, in a top-to-bottom, right-to-left fashion.

  The following image illustrates a collection of \l{QAppointment}{QAppointments} being displayed
  by an application.

  \image qappointmentdelegate.png "View of QAppointments"

  It is assumed that the model is a QAppointmentModel.  The roles used to draw the items include:

  \table 80 %
   \header
    \o Role
    \o Data Type
    \o Description
   \row
    \o Qt::DisplayRole
    \o QString
    \o Plain unformatted text drawn on the left of the icon, wrapped to any icons.
   \row
    \o Qt::DecorationRole
    \o QList<QIcon>
    \o Optional. Drawn on the right side of the item, top-to-bottom, right-to-left
  \endtable

  The four appointments shown in the picture above have the following data in the model:
  \table 80 %
   \header
    \o DisplayRole
    \o DecorationRole
   \row
    \o Status meeting
    \o List with a single icon for recurrence
   \row
    \o Lunch at Moe's
    \o List with a single icon for a reminder
   \row
    \o School play
    \o <empty list>
   \row
    \o Oslo conf.
    \o List with two icons:
       \list
        \o a recurrence icon
        \o a timezone icon
       \endlist
  \endtable

  Selected appointments are rendered with the current palette's \c Highlight color),
  while unselected appointments are rendered with the \c Button color.

  \sa QAppointment, QAppointmentModel, {Pim Library}
*/

/*!
  Constructs a QAppointmentDelegate with the given \a parent.
*/
QAppointmentDelegate::QAppointmentDelegate( QObject * parent )
    : QAbstractItemDelegate(parent)
{
    iconSize = QApplication::style()->pixelMetric(QStyle::PM_ListViewIconSize);
}

/*!
  Destroys a QAppointmentDelegate.
*/
QAppointmentDelegate::~QAppointmentDelegate() {}

/*!
  \internal
  Provides an alternate font based of the \a start font.  Reduces the size of the returned font
  by at least step point sizes.  Will attempt a total of six point sizes beyond the request
  point size until a valid font size that differs from the starting font size is found.
*/
QFont QAppointmentDelegate::differentFont(const QFont& start, int step) const
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
  \internal
  Returns the font to use for painting the main label text of the item.
  Due to the nature of rich text painting in Qt 4.0 attributes such as bold and italic will be
  ignored.  These attributes can be set by the text returned for
  QAbstractItemModel::data() where role is QAppointmentModel::LabelRole.

  By default returns the font of the style option \a o.
*/
QFont QAppointmentDelegate::mainFont(const QStyleOptionViewItem &o) const
{
    return o.font;
}

/*!
  \internal
  Returns the font to use for painting the sub label text of the item.
  Due to the nature of rich text painting in Qt 4.0 attributes such as bold and italic will be
  ignored.  These attributes can be set by the text returned for
  QAbstractItemModel::data() where role is QAppointmentModel::SubLabelRole.

  By default returns a font at least two point sizes smaller of the font of the
  style option \a o.
*/
QFont QAppointmentDelegate::secondaryFont(const QStyleOptionViewItem &o) const
{
    return differentFont(o.font, -2);
}

/*!
  \reimp
*/
void QAppointmentDelegate::paint(QPainter *painter, const QStyleOptionViewItem & option,
        const QModelIndex & index) const
{
    //  Prepare brush + pen and draw in background rectangle

    QRect border;
    QPen pen(option.palette.color(QPalette::Mid));
    if (option.state & QStyle::State_Selected) {
        painter->setBrush(option.palette.highlight());
        border = option.rect.adjusted(1, 1, -1, -1);
        pen.setWidth(2);
    } else {
        painter->setBrush(option.palette.button());
        border = option.rect.adjusted(0, 0, -1, -1);
    }

    painter->setPen(pen);
    painter->drawRect(border);

    QRect contentRect = option.rect.adjusted(2, 2, -2, -2);
    QRect textRect = contentRect;

    painter->save();
    painter->setClipRect(contentRect);

    // We try to align the text and the icons so that they are vertically
    // centered if that does not result in too much whitespace (being
    // defined as more than one font linespacing high)

    // In addition, we try to wrap the text to the leading boundary of the icons
    bool rtl = option.direction == Qt::RightToLeft;

    QString appText = index.model()->data(index, Qt::DisplayRole).toString();
    int fontHeight = QFontMetrics(option.font).lineSpacing();

    //  Draw in the relevant event icons (vertically centered), with a 2px margin
    // If the text is not vertically centered, we don't center the icons either.
    QList<QVariant> icons = index.model()->data( index, Qt::DecorationRole ).toList();
    if (icons.count() > 0) {
        // We use the height of the text as the desired icon size (-2 for the padding)
        int drawnIconSize = fontHeight - 2;
        if (drawnIconSize <= 0)
            drawnIconSize = 1;

        // number of rows of icons drawn
        int numRows = (contentRect.height() + 2) / (drawnIconSize + 2);
        if (numRows > icons.count())
            numRows = icons.count();

        // vertical offset, if any (if more than one line spare, we top align)
        int iconVOffset = (contentRect.height() + 2 - (numRows * (drawnIconSize + 2)) + 1) / 2;
        if (iconVOffset < 0 || iconVOffset >= contentRect.height() || iconVOffset > (drawnIconSize / 2))
            iconVOffset = 0;

        // Icon drawing position
        int iconY = iconVOffset;
        int iconX = rtl ? contentRect.left() : contentRect.right() - drawnIconSize;
        int iconDX = rtl ? (drawnIconSize + 2) : -(drawnIconSize + 2);

        // Now draw the icons, starting from top right, moving downwards and then leftwards
        for (QList<QVariant>::Iterator it = icons.begin(); it != icons.end(); ) {
            QIcon icon = qvariant_cast<QIcon>(*it);
            icon.paint(painter, iconX, contentRect.top() + iconY, drawnIconSize, drawnIconSize);

            // Calculate where the next icon should go
            ++it;
            if (it !=icons.end()) {
                if(contentRect.height() >= iconY + drawnIconSize + 2 + drawnIconSize)
                    iconY += drawnIconSize + 2;
                else {
                    iconY = iconVOffset;
                    iconX += iconDX;
                }
            }
        }

        if (rtl)
            textRect.setLeft(iconX + drawnIconSize + 2);
        else
            textRect.setRight(iconX - 2);
    }

    //  Prepare pen and draw in text
    if (option.state & QStyle::State_Selected)
        painter->setPen(option.palette.color(QPalette::HighlightedText));
    else
        painter->setPen(option.palette.color(QPalette::ButtonText));

    // Now see if we can fit our text in.
    int vertPos = 0;
    QRect textMetrics = painter->boundingRect(textRect, Qt::AlignLeft | Qt::TextWrapAnywhere, appText);

    if (textMetrics.height() < contentRect.height() && textMetrics.height() > (contentRect.height() - fontHeight))
        vertPos = (contentRect.height() - textMetrics.height() + 1) / 2;
    textRect.adjust(0, vertPos, 0, 0);
    painter->drawText(textRect, Qt::AlignLeft | Qt::TextWrapAnywhere, appText);
    painter->restore();
}

/*!
   \reimp
*/
QSize QAppointmentDelegate::sizeHint(const QStyleOptionViewItem & option,
        const QModelIndex &index) const
{
    Q_UNUSED(index);

    QFontMetrics fm(mainFont(option));

    return QSize(fm.width("M") * 10, fm.height() + 4);  //  Make Qtopia phone more compact
}
