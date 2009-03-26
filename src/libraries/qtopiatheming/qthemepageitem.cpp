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

#include <QThemePageItem>
#include <QThemedView>
#include <QThemedScene>
#include <QDesktopWidget>
#include <QApplication>
#include <QXmlStreamReader>
#include <QDebug>
#include "qthemeitem_p.h"

class QThemePageItemPrivate
{

public:
    QThemePageItemPrivate(): transparent(false) {
        ;
    }
    bool transparent;
};

/***************************************************************************/

/*!
  \class QThemePageItem
    \inpublicgroup QtBaseModule
  \brief The QThemePageItem class provides a page item that you can add to a QThemedView to display a page.
  \since 4.4

  \ingroup qtopiatheming
  \sa QThemeItem, QThemedView
*/

/*!
  Constructs a QThemePageItem.
  The \a parent is passed to the base class constructor.
*/
QThemePageItem::QThemePageItem(QThemeItem *parent)
        : QThemeItem(parent), d(new QThemePageItemPrivate())
{
    QThemeItem::registerType(Type);
}

/*!
  Destroys the QThemePageItem.
*/
QThemePageItem::~QThemePageItem()
{
    delete d;
}

/*!
  \reimp
*/
int QThemePageItem::type() const
{
    return Type;
}

/*!
  \reimp
*/
void QThemePageItem::loadAttributes(QXmlStreamReader &reader)
{
    QThemeItem::loadAttributes(reader);
    d->transparent = false;
    if (reader.attributes().value("transparent") == "yes") {
        d->transparent = true;
        QPalette pal = themedScene()->palette();
        pal.setColor(QPalette::All, QPalette::Window, QColor(Qt::transparent));
        themedScene()->setPalette(pal);
    }
}

#ifdef THEME_EDITOR
void QThemePageItem::saveAttributes(QXmlStreamWriter &writer)
{
    writer.writeAttribute("xmlns", "http://www.trolltech.com");
    writer.writeAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
    writer.writeAttribute("xsi:schemaLocation", "http://www.trolltech.com themedview.xsd");
    QThemeItem::saveAttributes(writer);
    if (d->transparent)
        writer.writeAttribute("transparent", "yes");
}
#endif
/*!
  \reimp
*/
void QThemePageItem::layout()
{
    QSizeF s = themedScene()->themedView()->size();
#if defined(THEME_EDITOR)
    s = themedScene()->sceneRect().size();
    foreach(QGraphicsItem* item, childItems()){
        if(qthemeitem_cast<QThemeItem*>(item)){
            qthemeitem_cast<QThemeItem*>(item)->layout();
        }
    }
#endif
    prepareGeometryChange();
    resize(s.width(), s.height());
//    setPos(0, 0);
}

/*!
  \reimp
*/
void QThemePageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    Q_UNUSED(painter);
#if defined(THEME_EDITOR)
    painter->setPen(Qt::black);
    painter->setBrush(Qt::white);
    painter->drawRect(boundingRect());
#endif
}

