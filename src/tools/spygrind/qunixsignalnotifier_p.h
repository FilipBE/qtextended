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

#ifndef QUNIXSIGNALNOTIFIER_P_H
#define QUNIXSIGNALNOTIFIER_P_H

#include <QObject>

#include <signal.h>

class QUnixSignalNotifier : public QObject
{
    Q_OBJECT

public:
    static QUnixSignalNotifier* instance(int);

signals:
    void raised();

private slots:
    void onReadyRead();

private:
    QUnixSignalNotifier(int);

    static int sigpipe[2];
    static void sighandler(int);
    static sighandler_t oldhandler;
};

#endif

