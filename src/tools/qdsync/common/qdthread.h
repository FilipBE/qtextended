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
#ifndef QDTHREAD_H
#define QDTHREAD_H

#include <qdglobal.h>
#include <QThread>
#include <QMutex>
#include <QSemaphore>

class QD_EXPORT QDThread : public QThread
{
public:
    QDThread( QObject *parent = 0 );
    virtual ~QDThread();

    void init();
    void start();
    virtual void t_init();
    virtual void t_run();
    virtual void t_quit();

private:
    void run();
    bool inited;
    QMutex waitForStartMutex; 
    QSemaphore waitForRunSemaphore;
};

#endif

