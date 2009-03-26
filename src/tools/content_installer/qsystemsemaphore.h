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
#ifndef QSYSTEMSEMAPHORE_H
#define QSYSTEMSEMAPHORE_H

#include <QObject>
#include <QString>

class QSystemSemaphorePrivate;

class QSystemSemaphore
{
public:
    QSystemSemaphore(const QString &fpath, int n = 1);
    ~QSystemSemaphore();
    void acquire();
    void release();
    int available();
    bool tryAcquire();
    bool tryAcquire(int timeout);
    bool isValid();
    static bool clear(const QString &fpath);

private:
    QSystemSemaphorePrivate *d;
};

#endif

