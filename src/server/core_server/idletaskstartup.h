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

#ifndef IDLETASKSTARTUP_H
#define IDLETASKSTARTUP_H

#include <qtopiaserverapplication.h>
#include <qtopiaservertasks_p.h>

#include <QTimer>

class IdleTaskStartup : public QObject
{
Q_OBJECT

public:
    IdleTaskStartup(QtopiaServerTasksPrivate*, QList<QtopiaServerTasksPrivate::Task*> const&,
        int idleTimeout = 4000, int maximumTimeout = 60000);
    virtual ~IdleTaskStartup();

public slots:
    void start();

signals:
    void finished();

private slots:
    void onIdleTimeout();
    void onMaximumTimeout();

private:
    bool idle() const;
    void startNextTask();

    QtopiaServerTasksPrivate*              m_qst;
    QList<QtopiaServerTasksPrivate::Task*> m_tasks;
    QTimer                                 m_idleTimer;
    QTimer                                 m_maximumTimer;
    QValueSpaceItem                        m_activeCalls;
    QValueSpaceItem                        m_modemStatus;
};

#endif

