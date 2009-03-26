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

#include "itemfactory.h"

#include <QListWidgetItem>

#include <QCoreApplication>

#include <QtopiaService>
#include <QTranslatableSettings>
#include <QContentSet>
#include <QMimeType>

QListWidgetItem *createServiceItem(const QString &service)
{
    QListWidgetItem *item = 0;

    QTranslatableSettings settings(QtopiaService::config(service), QSettings::IniFormat);
    settings.beginGroup("Service");

    if (settings.value("Multiple", 0).toInt() != 0)
        return 0;

    int applicationCount = QtopiaService::apps(service).count();

    if (applicationCount == 0)
        return 0;

    QString name = settings.value("Name", service).toString();
    QString icon = settings.value("Icon").toString();

    item = new QListWidgetItem;
    item->setData(Qt::UserRole, applicationCount);

    QString action;
    if (name.startsWith("Open/")) {
        action = QCoreApplication::translate("AppServices", "Open %1", "e.g. Open html");
        name.remove(0, 5);
    } else if (name.startsWith("Receive/")) {
        action = QCoreApplication::translate("AppServices", "Receive %1", "e.g Receive ppt");
        name.remove(0, 8);
    } else if (name.startsWith("View/")) {
        action = QCoreApplication::translate("AppServices", "View %1", "e.g. View xls");
        name.remove(0, 5);
    }

    if (!action.isEmpty()) {
        QMimeType mimeType(name);

        if (name.startsWith("application/"))
            name.remove(0, 12);

        if (name.startsWith("x-"))
            name.remove(0, 2);

        item->setText(action.arg(name));
        item->setIcon(mimeType.icon());
    } else {
        item->setText(name);

        if (!icon.isEmpty())
            item->setIcon(QIcon(":icon/" + icon));
    }

    return item;
}

QListWidgetItem *createTypeItem(QString type)
{
    QListWidgetItem *item = new QListWidgetItem;

    if (type.endsWith("/*"))
        type.chop(2);

    QString service = "Open/" + type;

    QMimeType mimeType(type);

    if (type.startsWith("application/"))
        type.remove(0, 12);

    if (type.startsWith("x-"))
        type.remove(0, 2);

    item->setText(type);
    item->setIcon(mimeType.icon());

    return item;
}

