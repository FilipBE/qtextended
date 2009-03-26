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

#include <QThemeGroupItem>
#include <QPainter>
#include <QXmlStreamReader>
#include "qthemeitem_p.h"
#include <QGraphicsSceneMouseEvent>

class QThemeGroupItemPrivate
{

public:
    QThemeGroupItemPrivate();
};

/***************************************************************************/

/*!
  \class QThemeGroupItem
    \inpublicgroup QtBaseModule
  \brief The QThemeGroupItem class groups its children into a single item in a QThemedView.
  \since 4.4

  \ingroup qtopiatheming
  \sa QThemeItem, QThemedView
*/

/*!
  Constructs a QThemeGroupItem.
  The \a parent is passed to the base class constructor.
*/
QThemeGroupItem::QThemeGroupItem(QThemeItem *parent)
        : QThemeItem(parent)
{
    QThemeItem::registerType(Type);
}

/*!
  Destroys the QThemeGroupItem;
*/
QThemeGroupItem::~QThemeGroupItem()
{
}

/*!
  \reimp
*/
int QThemeGroupItem::type() const
{
    return Type;
}

/*!
  \reimp
*/
void QThemeGroupItem::layout()
{
    QThemeItem::layout();
}

/*!
  \reimp
*/
void QThemeGroupItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    pressed(event);
    foreach(QGraphicsItem *item, childItems()) {
        QThemeItem *i = QThemeItem::themeItem(item);
        if (i)
            i->pressed(event);
    }
}

/*!
  \reimp
*/
void QThemeGroupItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    released(event);
    foreach(QGraphicsItem *item, childItems()) {
        QThemeItem *i = QThemeItem::themeItem(item);
        if (i)
            i->released(event);
    }
}
