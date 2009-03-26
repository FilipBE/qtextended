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

#ifndef GREENPHONEKBDHANDLER_H
#define GREENPHONEKBDHANDLER_H

#ifdef QT_QWS_GREENPHONE

#include <QObject>
#include <QWSKeyboardHandler>

#include <termios.h>
#include <linux/kd.h>

class QSocketNotifier;

class GreenphoneKbdHandler : public QObject, public QWSKeyboardHandler
{
    Q_OBJECT
public:
    GreenphoneKbdHandler();
    ~GreenphoneKbdHandler();

private:
    QSocketNotifier *m_notify;
    int  kbdFD;
//    unsigned int    m_repeatKeyCode;
//    unsigned short  m_unicode;
    struct termios origTermData;
//    QTimer*     m_timer;

private Q_SLOTS:
    void readKbdData();
    void handleTtySwitch(int sig);
//    void repeat();
};

#endif // QT_QWS_GREENPHONE

#endif
