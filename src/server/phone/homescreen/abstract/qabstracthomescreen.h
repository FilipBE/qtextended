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

#ifndef QABSTRACTHOMESCREEN_H
#define QABSTRACTHOMESCREEN_H

#include <QWidget>

class QAbstractHomeScreen : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("SingletonServerWidget", "true");

public:
    QAbstractHomeScreen(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~QAbstractHomeScreen();

    virtual bool locked() const = 0;
#ifdef QTOPIA_CELL
    virtual bool simLocked() const = 0;
#endif
    virtual void setLocked(bool) = 0;

Q_SIGNALS:
#ifdef QTOPIA_TELEPHONY
    void callEmergency(const QString &number);
    void showCallScreen();
    void showCallHistory();
    void showMissedCalls();
    void dialNumber(const QString &number);
    void hangupCall();
#endif
    void speedDial(const QString &);
    void lockStateChanged(bool);
    void showPhoneBrowser();
};

#endif
