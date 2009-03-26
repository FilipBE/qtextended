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

#include <QThemeExclusiveItem>
#include <QPainter>
#include <QXmlStreamReader>
#include "qthemeitem_p.h"

class QThemeExclusiveItemPrivate
{

public:
    QThemeExclusiveItemPrivate();
};

/***************************************************************************/

/*!
  \class QThemeExclusiveItem
    \inpublicgroup QtBaseModule
  \brief The QThemeExclusiveItem class provides a exclusive item that allows only one of its children to be active in a QThemedView.
  \since 4.4

  \ingroup qtopiatheming
  \sa QThemeItem, QThemedView
*/

/*!
  Constructs a QThemeExclusiveItem.
  The \a parent is passed to the base class constructor.
*/
QThemeExclusiveItem::QThemeExclusiveItem(QThemeItem *parent)
        : QThemeItem(parent)
{
    QThemeItem::registerType(Type);
}

/*!
  Destroys the QThemeExclusiveItem;
*/
QThemeExclusiveItem::~QThemeExclusiveItem()
{
}

/*!
  \reimp
*/
int QThemeExclusiveItem::type() const
{
    return Type;
}

/*!
  \reimp
  Lays out the exclusive item.
  The exclusive item iterates through its children in reverse order, sets the first active found
  item to be visible, and sets all other items to not be visible.
  \sa setVisible(), isVisible()
*/
void QThemeExclusiveItem::layout()
{
    QThemeItem::layout();

    int count = childItems().count();
    if (count != 0) {
        bool found = false;
        while (count > 0) {
            QThemeItem *item = static_cast<QThemeItem *>(childItems().at(count - 1));
            if (item->isActive() && !found) {
                item->setVisible(true);
                found = true;
            } else {
                item->setVisible(false);
            }
            --count;
        }
    }
}
