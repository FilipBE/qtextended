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

#include <QThemeWidgetItem>
#include <QThemedScene>
#include <QPainter>
#include <QXmlStreamReader>
#include <QDebug>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include "qthemeitem_p.h"

class QThemeWidgetItemPrivate
{
public:
    QGraphicsProxyWidget *proxy;
    QWidget *widget;
    QMap<QString,QString> colorGroupAtts;
    int size;
    bool bold;
    bool completed;
    bool activeOnCreation;
};

/*!
  \class QThemeWidgetItem
    \inpublicgroup QtBaseModule
  \since 4.4
  \brief The QThemeWidgetItem class provides a widget item that you can add to a QThemedView.

  \ingroup qtopiatheming
  \sa QThemeItem, QThemedView
*/

/*!
  Constructs a QThemeWidgetItem.
  The \a parent is passed to the base class constructor.
*/
QThemeWidgetItem::QThemeWidgetItem(QThemeItem *parent)
        : QThemeItem(parent), d(new QThemeWidgetItemPrivate)
{
    QThemeItem::registerType(Type);
    d->proxy = 0;
    d->widget = 0;
    d->size = 0;
    d->bold = false;
    d->completed = false;
}

/*!
  Destroys the QThemeWidgetItem.
*/
QThemeWidgetItem::~QThemeWidgetItem()
{
    delete d->proxy;
    delete d;
}

/*!
  \reimp
*/
int QThemeWidgetItem::type() const
{
    return Type;
}

/*!
  \reimp
*/
void QThemeWidgetItem::loadAttributes(QXmlStreamReader &reader)
{
    QThemeItem::loadAttributes(reader);
    if (!reader.attributes().value("colorGroup").isEmpty()) {
        d->colorGroupAtts = parseSubAttributes(reader.attributes().value("colorGroup").toString());
    }
    if (!reader.attributes().value("size").isEmpty())
        d->size = reader.attributes().value("size").toString().toInt();
    if (!reader.attributes().value("bold").isEmpty())
        d->bold = reader.attributes().value("bold") == "yes" ? true : false;
}

/*!
  \reimp
*/
void QThemeWidgetItem::constructionComplete()
{
    d->completed = true;
    setupWidget();
    QThemeItem::constructionComplete();
}

/*!
  \internal
*/
void QThemeWidgetItem::setupWidget()
{
    if (!d->widget || !d->completed)
        return;

    QFont font = d->widget->font();
    font.setBold(d->bold);
    if (d->size != 0)
        font.setPointSize(d->size);
    d->widget->setFont(font);
    d->proxy = new QGraphicsProxyWidget(this);
    d->proxy->setWidget(d->widget);
    d->proxy->widget()->lower();
    parseColorGroup(d->colorGroupAtts);
    if (!d->widget->inherits("QLineEdit")) {
        QPalette p = d->proxy->palette();
        p.setColor(QPalette::Window, QColor(0, 0, 0, 0));
        d->proxy->setPalette(p);
    }
}

/*!
  \reimp
  Lays out the widget item.
  The widget is positioned and resized according to the theme.
*/
void QThemeWidgetItem::layout()
{
    if (!d->proxy || !d->proxy->widget())
        return;
    QThemeItem::layout();
    d->proxy->setPos(0, 0);
    d->proxy->widget()->setFixedSize(boundingRect().size().toSize());
}

/*!
  Sets the widget \a widget on this item.
*/
void QThemeWidgetItem::setWidget(QWidget* widget)
{
    d->widget = widget;
    setupWidget();
}

/*!
  Returns the widget for this item.
*/
QWidget* QThemeWidgetItem::widget() const
{
    if (d->proxy)
        return d->proxy->widget();
    return 0;
}

/*!
  \internal
*/
void QThemeWidgetItem::parseColorGroup(const QMap<QString,QString> &cgatts)
{
    if (!d->proxy || !d->proxy->widget())
        return;
    QPalette pal = d->proxy->palette();
    for (int i = 0; colorTable[i].role != QPalette::NColorRoles; ++i)
    {
        const QString curColorName = QString(colorTable[i].name).toLower();
        QColor colour;
        for (QMap<QString,QString>::ConstIterator it = cgatts.begin(); it != cgatts.end(); ++it)
        {
            if (it.key().toLower() == curColorName) {
                colour = colorFromString(*it);
                break;
            }
        }
        if (colour.isValid()) {
            pal.setColor(QPalette::Active, colorTable[i].role, colour);
            pal.setColor(QPalette::Inactive, colorTable[i].role, colour);
            pal.setColor(QPalette::Disabled, colorTable[i].role, colour);
        }
    }
    d->proxy->setPalette(pal);
}

