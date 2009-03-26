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

#ifndef IRCONTROLLER_H
#define IRCONTROLLER_H

#include <QObject>
#include <QStringList>
#include <QByteArray>
#include <QIcon>

#include <qcommdevicecontroller.h>

class IRController : public QObject {
    Q_OBJECT

public:
    enum State { Off, On, On5Mins, On1Item, LastState=On1Item };

    IRController(QObject* parent);
    ~IRController();

    void setState(State s);
    State state() const;
    static QString stateDescription(State);

    int protocolCount() const;
    int currentProtocol() const;
    QString protocolName(int i) const;
    QIcon protocolIcon(int i) const;
    bool setProtocol(int i);

public slots:
    void powerStateChanged(QCommDeviceController::PowerState state);

signals:
    void stateChanged(IRController::State);

private:
    QStringList names;
    QStringList icons;
    QStringList targets;
    int curproto;
    int protocount;

    QCommDeviceController *m_device;
    State m_st;
};

#endif
