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

#ifndef QTHEMEPLUGINITEM_H
#define QTHEMEPLUGINITEM_H

#include <QThemeItem>

class ThemedItemPlugin;

class QThemePluginItemPrivate;
class QTOPIATHEMING_EXPORT QThemePluginItem : public QThemeItem
{
public:
    QThemePluginItem(QThemeItem *parent = 0);
    ~QThemePluginItem();

    void setPlugin(const QString &plugin);
    void setBuiltin(ThemedItemPlugin *);

    enum { Type = ThemeItemType + 15 };
    int type() const;
    virtual void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);

protected:
    virtual void layout();

private:
    void releasePlugin();

private:
    QThemePluginItemPrivate *d;
};

#endif
