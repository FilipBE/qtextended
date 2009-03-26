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

#ifndef N800KBDHANDLER_H
#define N800KBDHANDLER_H

#ifdef QT_QWS_N810

#include <QObject>
#include <QWSKeyboardHandler>

#include <termios.h>
#include <linux/kd.h>

class QSocketNotifier;

class N800KbdHandler : public QObject, public QWSKeyboardHandler
{
    Q_OBJECT
public:
    N800KbdHandler();
    ~N800KbdHandler();

private:
    QSocketNotifier *m_notify;
    int  kbdFD;
    struct termios origTermData;
    bool shift;
int numKeyPress;
private Q_SLOTS:
    void readKbdData();

    void handleTtySwitch(int sig);
};

#endif // QT_QWS_N800

#endif
