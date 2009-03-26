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

#ifndef QTHEMELAYOUTITEM_H
#define QTHEMELAYOUTITEM_H

#include <qtopiaglobal.h>
#include <QThemeItem>
#ifdef THEME_EDITOR
#include "qthemelayoutitemeditor.h"
#include <QXmlStreamWriter>
#endif

class QThemeLayoutItemPrivate;

class QTOPIATHEMING_EXPORT QThemeLayoutItem : public QThemeItem
{
#ifdef THEME_EDITOR
    friend class QThemeLayoutItemEditor;
#endif
public:
    QThemeLayoutItem(QThemeItem *parent = 0);
    ~QThemeLayoutItem();

    enum { Type = ThemeItemType + 5 };
    int type() const;

#ifdef THEME_EDITOR
    void saveAttributes(QXmlStreamWriter &writer);
    QWidget *editWidget();
#endif

protected:
    void loadAttributes(QXmlStreamReader &reader);
    void layout();
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

private:
    QThemeLayoutItemPrivate *d;
};

#endif
