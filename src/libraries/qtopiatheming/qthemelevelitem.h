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

#ifndef QTHEMELEVELITEM_H
#define QTHEMELEVELITEM_H

#include <qtopiaglobal.h>
#include <qthemeimageitem.h>
#ifdef THEME_EDITOR
#include "qthemelevelitemeditor.h"
#include <QXmlStreamWriter>
#endif

class QThemeLevelItemPrivate;

class QTOPIATHEMING_EXPORT QThemeLevelItem : public QThemeImageItem
{
#ifdef THEME_EDITOR
    friend class QThemeLevelItemEditor;
#endif
public:
    QThemeLevelItem(QThemeItem *parent = 0);
    ~QThemeLevelItem();

    enum { Type = ThemeItemType + 13 };
    int type() const;

    void advance(int i = 0);
    void start();
    void stop();
    void setFrame(int frame);
    void setValue(int value);
    void updateValue(int value);

#ifdef THEME_EDITOR
    QWidget *editWidget();
    void saveAttributes(QXmlStreamWriter &writer);
#endif

protected:
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
    void loadAttributes(QXmlStreamReader &reader);
    void loadChildren(QXmlStreamReader &reader);
    void expressionChanged(QExpressionEvaluator *);
    void constructionComplete();

private:
    QThemeLevelItemPrivate *d;
};

#endif
