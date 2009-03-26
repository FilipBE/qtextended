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

#ifndef QTHEMEITEMFACTORY_H
#define QTHEMEITEMFACTORY_H

#include <qtopiaglobal.h>
#include <QList>

#ifndef THEME_EDITOR
class QPluginManager;
#endif
class QThemeItem;

class QTOPIATHEMING_EXPORT QThemeItemPlugin : public QObject
{
    Q_OBJECT
public:
    virtual QStringList keys() const = 0;
    virtual QThemeItem* create(const QString &type, QThemeItem *parent) = 0;
};

class QTOPIATHEMING_EXPORT QThemeItemFactory
{
public:
    static QThemeItem *create(const QString &type, QThemeItem *parent);
    static void clear();
    static void registerBuiltin(QThemeItemPlugin *plugin);

private:
#ifndef THEME_EDITOR
    static QThemeItem *createPlugin(const QString &type, QThemeItem *parent);
    static QThemeItem *createBuiltin(const QString &type, QThemeItem *parent);

private:
    static QPluginManager *pluginManager;
    static QList<QThemeItemPlugin *> builtins;
#endif
};

#endif
