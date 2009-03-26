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

#ifndef OOMMANAGER_H
#define OOMMANAGER_H

#include <QObject>
#include <QMap>
#include <QProcess>
#include "applicationlauncher.h"

class OomPrivate;

class OomManager : public QObject
{
    Q_OBJECT

public:
    enum Importance { Expendable, Important, Critical };

    OomManager();
    ~OomManager();

    void insert(const QString& app, int pid);
    void remove(const QString& app);

    const QMap<QString,int>& expendableProcs() const;
    const QMap<QString,int>& importantProcs() const;
    bool hasExpendableProcs() const;
    bool hasImportantProcs() const;
    bool isExpendable(const QString& app) const;
    bool isImportant(const QString& app) const;

    QString procWithBiggestScore(Importance t) const;
    void printOomValues(bool score);

private slots:
    void pidStateChanged(const QString& app, Q_PID pid);

private:
    OomPrivate* d;
};

QTOPIA_TASK_INTERFACE(OomManager);
#endif
