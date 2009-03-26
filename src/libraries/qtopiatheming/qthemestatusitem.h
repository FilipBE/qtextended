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

#ifndef QTHEMESTATUSITEM_H
#define QTHEMESTATUSITEM_H

#include <qtopiaglobal.h>
#include <qthemeimageitem.h>
#ifdef THEME_EDITOR
#include "qthemestatusitemeditor.h"
#include <QXmlStreamWriter>
#endif

class QThemeStatusItemPrivate;

class QTOPIATHEMING_EXPORT QThemeStatusItem : public QThemeImageItem
{
#ifdef THEME_EDITOR
    friend class QThemeStatusItemEditor;
#endif
public:
    QThemeStatusItem(QThemeItem *parent = 0);
    ~QThemeStatusItem();

    enum { Type = ThemeItemType + 7 };
    int type() const;

#ifdef THEME_EDITOR
    QWidget *editWidget();
    void saveAttributes(QXmlStreamWriter &writer);
#endif

protected:
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
    void loadAttributes(QXmlStreamReader &reader);
    void expressionChanged(QExpressionEvaluator *);
    void constructionComplete();

private:
    QThemeStatusItemPrivate *d;
};



#endif
