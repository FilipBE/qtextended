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

#ifndef QTHEMETEMPLATEITEM_H
#define QTHEMETEMPLATEITEM_H

#include <QThemeItem>

class QThemeTemplateInstanceItem;
class QThemeTemplateItemPrivate;

class QTOPIATHEMING_EXPORT QThemeTemplateItem : public QThemeItem
{

public:
    QThemeTemplateItem(QThemeItem *parent = 0);
    virtual ~QThemeTemplateItem();

    enum { Type = ThemeItemType + 9 };
    virtual int type() const;

    QThemeTemplateInstanceItem* createInstance(const QString &uid);

protected:
    virtual void loadChildren(QXmlStreamReader &reader);
#ifdef THEME_EDITOR
    virtual void saveChildren(QXmlStreamWriter &writer);
#endif

private:
    QThemeTemplateItemPrivate *d;
};

class QTOPIATHEMING_EXPORT QThemeTemplateInstanceItem : public QThemeTemplateItem
{

public:
    QThemeTemplateInstanceItem(QThemeItem *parent = 0);
    ~QThemeTemplateInstanceItem();

    enum { Type = ThemeItemType + 12 };
    int type() const;

    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

protected:
    void loadChildren(QXmlStreamReader &reader);

private:
};

#endif
