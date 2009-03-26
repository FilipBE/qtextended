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

#ifndef QPHONESTATUS_H
#define QPHONESTATUS_H

#include <qtopiaglobal.h>

#include <QObject>

class QPhoneStatusPrivate;

class QTOPIAPHONE_EXPORT QPhoneStatus : public QObject
{
    Q_OBJECT
public:
    explicit QPhoneStatus(QObject *parent);
    ~QPhoneStatus();

    enum StatusItem {
        // int values
        BatteryLevel=1,
        SignalLevel=2,
        MissedCalls=3,
        NewMessages=4,
        ActiveCalls=5,

        // string values
        OperatorName=100,
        Profile=101,

        // bool values
        Roaming=200,
        Locked=201,
        Alarm=202,
        CallDivert=203,
        NetworkRegistered=204,
        VoIPRegistered=205,
        Presence=206
    };

    QVariant value(StatusItem);

signals:
    void incomingCall(const QString &number, const QString &name);
    void statusChanged();

private slots:
    void phoneStatusChanged();

private:
    QPhoneStatusPrivate *d;
};

#endif
