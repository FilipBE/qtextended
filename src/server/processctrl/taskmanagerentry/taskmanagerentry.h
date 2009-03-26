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

#ifndef TASKMANAGERENTRY_H
#define TASKMANAGERENTRY_H

#include <QObject>

class TaskManagerEntryPrivate;

class TaskManagerEntry : public QObject
{
    Q_OBJECT
public:
    TaskManagerEntry(const QString &description, const QString &iconPath, QObject *parent = 0);

public slots:
    void show();
    void hide();

signals:
    void activated();

private:
    friend class TaskManagerEntryPrivate;
    TaskManagerEntryPrivate *m_data;
};

#endif
