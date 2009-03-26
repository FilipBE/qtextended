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

#ifndef QTHEMETEXTITEM_H
#define QTHEMETEXTITEM_H

#include <qtopiaglobal.h>
#include <qthemeitem.h>

#ifdef THEME_EDITOR
#include "qthemetextitemeditor.h"
#include <QXmlStreamWriter>
#endif

class QAbstractTextDocumentLayout;
class QThemeTextItemPrivate;

class QTOPIATHEMING_EXPORT QThemeTextItem : public QThemeItem
{
#ifdef THEME_EDITOR
    friend class QThemeTextItemEditor;
#endif
public:
    QThemeTextItem(QThemeItem *parent = 0);
    ~QThemeTextItem();

    enum { Type = ThemeItemType + 3 };
    int type() const;

    void setTextFormat(Qt::TextFormat format);
    Qt::TextFormat textFormat() const;
    void setText(const QString&);
    QString text() const;

#ifdef THEME_EDITOR
    QWidget *editWidget();
    void saveAttributes(QXmlStreamWriter &writer);
#endif

protected:
    void expressionChanged(QExpressionEvaluator *);
    void constructionComplete();
    void loadChildren(QXmlStreamReader &reader);
    void loadAttributes(QXmlStreamReader &reader);
    void loadTranslation();
    QString loadTranslationArguments(QXmlStreamReader &reader) const;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
    void setupThemeText();
    void drawOutline(QPainter *painter, const QRect &rect, int flags, const QString &text);
    void drawOutline(QPainter *p, const QRect &r, const QPalette &pal, QAbstractTextDocumentLayout *layout);

private:
    QThemeTextItemPrivate *d;
};


#endif
