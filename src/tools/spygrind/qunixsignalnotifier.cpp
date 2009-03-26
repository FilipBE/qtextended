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

#include "qunixsignalnotifier_p.h"

#include <QDebug>
#include <QSocketNotifier>

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int QUnixSignalNotifier::sigpipe[2]             = { 0, 0 };
sighandler_t QUnixSignalNotifier::oldhandler    = 0;

QUnixSignalNotifier::QUnixSignalNotifier(int signum)
{
    oldhandler = ::signal(signum, sighandler);
    if (-1 == ::pipe(QUnixSignalNotifier::sigpipe)) {
        ::perror("spygrind: pipe");
    } else {
        QSocketNotifier* sn = new QSocketNotifier
                    (sigpipe[0], QSocketNotifier::Read, this);
        if (!connect(sn, SIGNAL(activated(int)), this, SLOT(onReadyRead())))
            Q_ASSERT(0);
    }
}

QUnixSignalNotifier* QUnixSignalNotifier::instance(int signum)
{
    static int used_signum = signum;
    static QUnixSignalNotifier noty(used_signum);
    if (used_signum != signum) {
        qWarning()  << "QUnixSignalNotifier::instance first called with" << used_signum
                    << "and now called with" << signum << "; only first signal will be used";
    }
    return &noty;
}

void QUnixSignalNotifier::onReadyRead()
{
    char buf = 0x00;
    while (-1 == ::read(sigpipe[0], &buf, 1) && errno == EINTR) {}
    emit raised();
}

void QUnixSignalNotifier::sighandler(int signum)
{
    char byte = 0x01;
    while (-1 == ::write(sigpipe[1], &byte, 1) && errno == EINTR) {}
    if (oldhandler && oldhandler != SIG_ERR) oldhandler(signum);
}


