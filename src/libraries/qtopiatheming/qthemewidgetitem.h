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

#ifndef QTHEMEWIDGETITEM_H
#define QTHEMEWIDGETITEM_H

#include <QThemeItem>
#include <QMap>

class QThemeWidgetItemPrivate;

class QTOPIATHEMING_EXPORT QThemeWidgetItem : public QThemeItem
{

public:
    QThemeWidgetItem(QThemeItem *parent = 0);
    ~QThemeWidgetItem();

    enum { Type = ThemeItemType + 10 };
    virtual int type() const;

    virtual void setWidget(QWidget *widget);
    QWidget *widget() const;

protected:
    virtual void loadAttributes(QXmlStreamReader &reader);
    virtual void layout();
    virtual void constructionComplete();
    void setupWidget();

private:
    void parseColorGroup(const QMap<QString,QString> &cgatts);

private:
    QThemeWidgetItemPrivate *d;
};

#endif
