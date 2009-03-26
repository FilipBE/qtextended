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

#include <QThemeStatusItem>
#include "qthemestatusitem_p.h"
#include <QPainter>
#include <QXmlStreamReader>
#include <QThemeItemAttribute>
#include <QExpressionEvaluator>
#include <QDebug>

class QThemeImageItemPrivate;

/***************************************************************************/

/*!
  \class QThemeStatusItem
    \inpublicgroup QtBaseModule
  \brief The QThemeStatusItem class provides a status item that you can add to a QThemedView to display a status icon.
  \since 4.4

  \ingroup qtopiatheming
  \sa QThemeItem, QThemedView
*/

/*!
  Constructs a QThemeStatusItem.
  The \a parent is passed to the base class constructor.
*/
QThemeStatusItem::QThemeStatusItem(QThemeItem *parent)
        : QThemeImageItem(parent), d(new QThemeStatusItemPrivate)
{
    QThemeItem::registerType(Type);
}

/*!
  Destroys the QThemeStatusItem.
*/
QThemeStatusItem::~QThemeStatusItem()
{
    if (d->onExpr) {
        delete d->onExpr;
        d->onExpr = 0;
    }
    delete d;
}

/*!
  \reimp
*/
void QThemeStatusItem::loadAttributes(QXmlStreamReader &reader)
{
    QThemeImageItem::loadAttributes(reader);

    if (!reader.attributes().value("imageon").isEmpty())
        source()->setValue(reader.attributes().value("imageon").toString(), "on");
    if (!reader.attributes().value("imageoff").isEmpty())
        source()->setValue(reader.attributes().value("imageoff").toString(), "off");

    QString expr = reader.attributes().value("on").toString();
    bool isliteral = !isExpression(expr);
    if (isliteral) {
        d->isOn = expr != QLatin1String("no");
    } else {
        d->isOn = false;
        d->onExpression = strippedExpression(expr);
    }
}
#ifdef THEME_EDITOR
/*!
  \reimp
*/
void QThemeStatusItem::saveAttributes(QXmlStreamWriter &writer)
{
    QThemeImageItem::saveAttributes(writer);
    if (!source()->value("on").toString().isEmpty())
        writer.writeAttribute("imageon", source()->value("on").toString());
    if (!source()->value("off").toString().isEmpty())
        writer.writeAttribute("imageoff", source()->value("off").toString());
    if (d->onExpr)
        writer.writeAttribute("on", QString("expr:") + d->onExpr->expression());
    else
        writer.writeAttribute("on", d->isOn ? "yes" : "no");
}
#endif
/*!
  \reimp
*/
int QThemeStatusItem::type() const
{
    return Type;
}

/*!
  \reimp
*/
void QThemeStatusItem::paint(QPainter *p, const QStyleOptionGraphicsItem *o, QWidget *w)
{
    resizeImages(boundingRect().width(), boundingRect().height());
    p->setClipRect(boundingRect());
    p->setClipping(true);
    if (d->isOn) {
        p->drawPixmap(boundingRect().topLeft(), image("on"));
    } else if(!source()->value("off").toString().isEmpty()){
        p->drawPixmap(boundingRect().topLeft(), image("off"));
    }
    QThemeItem::paint(p, o, w);
}

/*!
  \reimp
*/
void QThemeStatusItem::expressionChanged(QExpressionEvaluator *expr)
{
    if (d->onExpr == expr) {
        QVariant result = getExpressionResult(expr, QVariant::Bool);
        if (!result.canConvert(QVariant::Bool)) {
            qWarning() << "ThemeStatusItem::expressionChanged() - Cannot convert value to Bool";
        } else {
            d->isOn = result.toBool();//setOn( result.toBool() );
            update();
        }
    } else {
        QThemeItem::expressionChanged(expr);
    }
}

/*!
  \reimp
*/
void QThemeStatusItem::constructionComplete()
{
    if (!d->onExpression.isEmpty()) {
        if (d->onExpr != 0) //remove previous expression
            delete d->onExpr;
        d->onExpr = createExpression(d->onExpression);
        if (d->onExpr != 0)
            expressionChanged(d->onExpr); // Force initial update
        d->onExpression = QString();
    }
    QThemeImageItem::constructionComplete();
}

#ifdef THEME_EDITOR
QWidget *QThemeStatusItem::editWidget()
{
    return new QThemeStatusItemEditor(this, QThemeItem::editWidget());
}
#endif
