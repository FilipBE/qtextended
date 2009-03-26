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

#ifndef N810KBDHANDLER_H
#define N810KBDHANDLER_H

#ifdef QT_QWS_N810

#include <QObject>
#include <QWSKeyboardHandler>
#include <QTimer>

class QSocketNotifier;

struct N810Keys {
    uint code;
    uint keyCode;
    uint FnKeyCode;
    ushort unicode;
    ushort shiftUnicode;
    ushort fnUnicode;
};

class N810KbdHandler : public QObject, public QWSKeyboardHandler
{
    Q_OBJECT
public:
    N810KbdHandler();
    ~N810KbdHandler();
    virtual const N810Keys *keyMap() const;

private:
    QSocketNotifier *m_notify;
    QSocketNotifier *powerNotify;
    QTimer *keytimer;

    int  kbdFD;
    int powerFd;
    bool shift;
    int numKeyPress;
    bool keyFunction;
    bool controlButton;

    int getKeyCode(int code, bool isFunc);
    int getUnicode(int code);

private Q_SLOTS:
    void readKbdData();
    void readPowerKbdData();
    void timerUpdate();
};

#endif // QT_QWS_N810

#endif
