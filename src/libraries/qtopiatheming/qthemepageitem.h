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

#ifndef QTHEMEPAGEITEM_H
#define QTHEMEPAGEITEM_H

#include <qtopiaglobal.h>
#include <QThemeItem>
#ifdef THEME_EDITOR
#include <QXmlStreamWriter>
#endif

class QThemePageItemPrivate;

class QTOPIATHEMING_EXPORT QThemePageItem : public QThemeItem
{

public:
    QThemePageItem(QThemeItem *parent = 0);
    ~QThemePageItem();

    void loadAttributes(QXmlStreamReader &reader);
#ifdef THEME_EDITOR
    void saveAttributes(QXmlStreamWriter &writer);
    void layoutPage() {layout();}
#endif

    enum { Type = ThemeItemType + 2 };
    int type() const;

protected:
    void layout();
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
private:
    QThemePageItemPrivate *d;
};

#endif
