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

#include <QThemeTemplateItem>
#include <QXmlStreamReader>
#ifdef THEME_EDITOR
#include <QXmlStreamWriter>
#endif
#include <QDebug>
#include <QPainter>

struct QThemeTemplateItemPrivate {
    QString data;
};

/*!
  \internal
  \class QThemeTemplateItem
    \inpublicgroup QtBaseModule
*/

/*!
  Constructs a QThemeTemplateItem.
  The \a parent is passed to the base class constructor.
*/
QThemeTemplateItem::QThemeTemplateItem(QThemeItem *parent)
        : QThemeItem(parent), d(new QThemeTemplateItemPrivate)
{
    QThemeItem::registerType(Type);
}

/*!
  Destroys the QThemeTemplateItem;
*/
QThemeTemplateItem::~QThemeTemplateItem()
{
}

/*!
  \reimp
*/
void QThemeTemplateItem::loadChildren(QXmlStreamReader &reader)
{
    Q_ASSERT(reader.isStartElement() && reader.name() == "template");

    QString data;
    data += "<template ";
    foreach(QXmlStreamAttribute a, reader.attributes())
    data += QString(a.name().toString()) + "='" + a.value().toString() + "' ";
    data += '>';

    while (!(reader.isEndElement() && reader.name() == "template")) {
        reader.readNext();
        if (reader.isStartElement()) {
            data += '<' + reader.name().toString() + ' ';
            foreach(QXmlStreamAttribute a, reader.attributes()) {
                data += a.name().toString() + "='" + a.value().toString() + "' ";
            }
            data += '>';
        } else if (reader.isCharacters())
            data += reader.text().toString().trimmed();
        else if (reader.isEndElement())
            data += "</" + reader.name().toString() + '>';
    }
    d->data = data;
}

#ifdef THEME_EDITOR
void QThemeTemplateItem::saveChildren(QXmlStreamWriter &writer)
{
    QString output;
    output = ">" + d->data.section('>', 1, -3);//remove self
    qint64 written = writer.device()->write(output.toAscii());
    if (written != output.toAscii().size())
        qWarning("QThemeTemplateItem Write Error");
}
#endif
/*!
  \reimp
*/
int QThemeTemplateItem::type() const
{
    return Type;
}

/*!
  \internal
*/
QThemeTemplateInstanceItem* QThemeTemplateItem::createInstance(const QString &uid)
{
    if (d->data.isEmpty())
        return 0;

    QXmlStreamReader reader(d->data);
    reader.readNext();
    reader.readNext();
    QThemeItem *parent = QThemeItem::themeItem(parentItem());
    if (!parent)
        return 0;

    QThemeTemplateInstanceItem *templateInstance = new QThemeTemplateInstanceItem(parent);
    templateInstance->setValueSpacePath(uid);
    templateInstance->load(reader);
    templateInstance->layout();
    templateInstance->setVisible(false);
    return templateInstance;
}

/***************************************************************************/

/*!
  \internal
  \class QThemeTemplateInstanceItem
    \inpublicgroup QtBaseModule
*/

/*!
  Constructs a QThemeTemplateInstanceItem.
  The \a parent is passed to the base class constructor.
*/
QThemeTemplateInstanceItem::QThemeTemplateInstanceItem(QThemeItem *parent)
        : QThemeTemplateItem(parent)
{
}

/*!
  Destroys the QThemeTemplateItem;
*/
QThemeTemplateInstanceItem::~QThemeTemplateInstanceItem()
{
}

/*!
  \reimp
*/
int QThemeTemplateInstanceItem::type() const
{
    return Type;
}

/*!
  \reimp
*/
void QThemeTemplateInstanceItem::loadChildren(QXmlStreamReader &reader)
{
    QThemeItem::loadChildren(reader);
}

/*!
  \reimp
*/
void QThemeTemplateInstanceItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    foreach (QGraphicsItem *child, childItems()) {
        QThemeItem *item = QThemeItem::themeItem(child);
        if (item) {
            item->paint(painter, option, widget);
        }
    }
}
