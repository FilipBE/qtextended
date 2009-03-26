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

#ifndef QTHEMERECTITEM_H
#define QTHEMERECTITEM_H

#include <qtopiaglobal.h>
#include <QThemeItem>
#ifdef THEME_EDITOR
#include "qthemerectitemeditor.h"
#include <QXmlStreamWriter>
#endif

class QThemeRectItemPrivate;

class QTOPIATHEMING_EXPORT QThemeRectItem : public QThemeItem
{
#ifdef THEME_EDITOR
    friend class QThemeRectItemEditor;
#endif
public:
    QThemeRectItem(QThemeItem *parent = 0);
    ~QThemeRectItem();

    void loadAttributes(QXmlStreamReader &reader);

    enum { Type = ThemeItemType + 8 };
    int type() const;

#ifdef THEME_EDITOR
    QWidget *editWidget();
    void saveAttributes(QXmlStreamWriter &writer);
#endif

protected:
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

private:
    QThemeRectItemPrivate *d;
};

#endif
