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

#include <QThemeRectItem>
#include "qthemerectitem_p.h"
#include <QThemedScene>
#include <QPainter>
#include <QXmlStreamReader>
#include <QThemeItemAttribute>
#include <QPalette>
#include <QDebug>

QThemeRectItemPrivate::QThemeRectItemPrivate()
        : alpha(0), brush(QColor(Qt::transparent)), color(QColor())
{
}

/***************************************************************************/

/*!
  \class QThemeRectItem
    \inpublicgroup QtBaseModule
  \brief The QThemeRectItem class provides a rect item that you can add to a QThemedView.
  \since 4.4

  \ingroup qtopiatheming
  \sa QThemeItem, QThemedView
*/

/*!
  Constructs a QThemeRectItem.
  The \a parent is passed to the base class constructor.
*/
QThemeRectItem::QThemeRectItem(QThemeItem *parent)
        : QThemeItem(parent), d(new QThemeRectItemPrivate)
{
    QThemeItem::registerType(Type);
}

/*!
  Destroys the QThemeRectItem.
*/
QThemeRectItem::~QThemeRectItem()
{
    delete d;
}

/*!
  \reimp
*/
void QThemeRectItem::loadAttributes(QXmlStreamReader &reader)
{
    QThemeItem::loadAttributes(reader);

    if (!reader.attributes().value("alpha").isEmpty())
        d->alpha.setValue(reader.attributes().value("alpha").toString());
    if (!reader.attributes().value("brush").isEmpty())
        d->brush.setValue(reader.attributes().value("brush").toString());
    if (!reader.attributes().value("color").isEmpty())
        d->color.setValue(reader.attributes().value("color").toString());
}

#ifdef THEME_EDITOR
void QThemeRectItem::saveAttributes(QXmlStreamWriter &writer)
{
    QThemeItem::saveAttributes(writer);
    if (d->alpha.isModified())
        writer.writeAttribute("alpha", d->alpha.value().toString());
    if (d->brush.isModified())
        writer.writeAttribute("brush", d->brush.value().toString());
    if (d->color.isModified())
        writer.writeAttribute("color", d->color.value().toString());
}
#endif

/*!
  \reimp
*/
int QThemeRectItem::type() const
{
    return Type;
}

/*!
  \reimp
*/
void QThemeRectItem::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w)
{
    if (d->color.isModified())
        p->setPen(colorFromString(d->color.value().toString()));
    else
        p->setPen(Qt::NoPen);

    QColor brush;
    if (d->brush.isModified()) {
        brush = colorFromString(d->brush.value().toString());
        p->setBrush(brush);
    } else {
        p->setBrush(Qt::transparent);
    }

    if (d->alpha.isModified()) {
        brush.setAlpha(d->alpha.value().toInt());
        p->setBrush(brush);
    } else {
        p->setBrush(Qt::transparent);
    }
    p->drawRect(boundingRect());
    QThemeItem::paint(p, o, w);
}

#ifdef THEME_EDITOR
QWidget *QThemeRectItem::editWidget()
{
    return new QThemeRectItemEditor(this, QThemeItem::editWidget());
}
#endif
