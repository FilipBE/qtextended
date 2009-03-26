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

#include <QThemeLayoutItem>
#include "qthemelayoutitem_p.h"
#include <QPainter>
#include <QXmlStreamReader>
#include "qthemeitem_p.h"
#include <QFormLayout>

QThemeLayoutItemPrivate::QThemeLayoutItemPrivate()
{
    orientation = Qt::Horizontal;
    
}

/***************************************************************************/

/*!
  \class QThemeLayoutItem
    \inpublicgroup QtBaseModule
  \brief The QThemeLayoutItem class provides a layout item that you can add to a QThemedView to lay out items.
  \since 4.4

  \ingroup qtopiatheming
  \sa QThemeItem, QThemedView
*/

/*!
  Constructs a QThemeLayoutItem.
  The \a parent is passed to the base class constructor.
*/
QThemeLayoutItem::QThemeLayoutItem(QThemeItem *parent)
        : QThemeItem(parent), d(new QThemeLayoutItemPrivate)
{
    QThemeItem::registerType(Type);
}

/*!
  Destroys the QThemeLayoutItem.
*/
QThemeLayoutItem::~QThemeLayoutItem()
{
    delete d;
}

/*!
  \reimp
*/
void QThemeLayoutItem::loadAttributes(QXmlStreamReader &reader)
{
    QThemeItem::loadAttributes(reader);

    d->alignment.setValue(parseAlignment(reader.attributes().value("align").toString()));
    d->spacing.setValue(reader.attributes().value("spacing").toString().toInt());

    QStringRef o = reader.attributes().value("orientation");
    if (o == "horizontal")
        d->orientation = Qt::Horizontal;
    else if (o == "vertical")
        d->orientation = Qt::Vertical;
}
#ifdef THEME_EDITOR
/*!
  \reimp
*/
void QThemeLayoutItem::saveAttributes(QXmlStreamWriter &writer)
{
    QThemeItem::saveAttributes(writer);

    writer.writeAttribute("orientation", d->orientation == Qt::Horizontal ? "horizontal" : "vertical");
}

#endif

/*!
  \reimp
*/
int QThemeLayoutItem::type() const
{
    return Type;
}

/*!
  \reimp
*/
void QThemeLayoutItem::layout()
{
    QThemeItem::layout();
    qreal fixedSize = 0;
    int visCount = 0;
    int expandingCount = 0;
    qreal w, h;
    qreal spacing = d->spacing.value().toInt();
    int align = d->alignment.value().toInt();
    bool expanding = true;

    foreach(QGraphicsItem *it, childItems()) {
        QThemeItem *item = QThemeItem::themeItem(it);

        if (!item)
            break;

        if (item->isActive() && item->isVisible()) {
            visCount++;
            item->layout();

            if (d->orientation == Qt::Horizontal) {
                w = qRound(item->d->sr.width());

                if (expanding || w == 0)
                    ++expandingCount;
                else
                    fixedSize += resolveUnit(w, boundingRect().width(), item->d->unit[2]);

            } else {
                h = qRound(item->d->sr.height());

                if (expanding || h == 0)
                    ++expandingCount;
                else {
                    fixedSize += resolveUnit(h, boundingRect().height(), item->d->unit[3]);
                }
            }
        }
    }

    if (!visCount)
        return;

    qreal offs = 0;
    qreal expansionSize = 0;
    qreal size = d->orientation == Qt::Horizontal ? boundingRect().width() : boundingRect().height();

    if (expandingCount > 0) {
        expansionSize = (size - fixedSize - (visCount - 1) * spacing) / expandingCount;
    } else {
        if (align & Qt::AlignHCenter || align & Qt::AlignVCenter)
            offs = (size - fixedSize - (visCount - 1) * spacing) / 2;
        else if (align & Qt::AlignRight)
            offs = size - fixedSize - (visCount - 1) * spacing;
    }

    foreach(QGraphicsItem *it, childItems()) {
        QThemeItem *item = QThemeItem::themeItem(it);
        if (!item)
            break;
        if (item->isActive() && item->isVisible()) {
            if (d->orientation == Qt::Horizontal) {
                w = qRound(item->d->sr.width());

                if (expanding || w == 0) {
                    w = expansionSize;
                    item->d->unit[2] = Pixel;
                } else {
                    w = item->boundingRect().width();
                }
                item->setPos(offs, 0);
                item->d->boundingRect.setWidth(w);
                item->d->boundingRect.setHeight(boundingRect().height());
                offs += qAbs(resolveUnit(item->boundingRect().width(), boundingRect().width(), item->d->unit[2]));
            } else {
                h = qRound(item->d->sr.height());

                if (expanding || h == 0) {
                    h = expansionSize;
                    item->d->unit[3] = Pixel;
                } else {
                    h = item->boundingRect().height();
                }
                item->setPos(0, offs);
                item->d->boundingRect.setWidth(boundingRect().width());
                item->d->boundingRect.setHeight(h);
                offs += qAbs(resolveUnit(item->boundingRect().height(), boundingRect().height(), item->d->unit[3]));
            }
            offs += spacing;
        }
    }
}

/*!
  \reimp
*/
void QThemeLayoutItem::paint(QPainter *p, const QStyleOptionGraphicsItem *o,
                             QWidget *w)
{
    QThemeItem::paint(p, o, w);
}
#ifdef THEME_EDITOR
QWidget *QThemeLayoutItem::editWidget()
{
    return new QThemeLayoutItemEditor(this, QThemeItem::editWidget());
}

#endif

