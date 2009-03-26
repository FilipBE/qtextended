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

#ifndef QBLUETOOTHAUTHORIZATIONAGENT_H
#define QBLUETOOTHAUTHORIZATIONAGENT_H

#include <qbluetoothglobal.h>
#include <QObject>

class QBluetoothAuthorizationAgentPrivate;
class QBluetoothLocalDevice;
class QBluetoothAddress;

class QBLUETOOTH_EXPORT QBluetoothAuthorizationAgent : public QObject
{
    Q_OBJECT

public:
    explicit QBluetoothAuthorizationAgent(const QString &name, QObject *parent = 0);
    ~QBluetoothAuthorizationAgent();

    enum Error {
        NoError,
        AlreadyExists,
        DoesNotExist,
        UnknownError
    };

    QString name() const;

    bool registerAgent();
    bool unregisterAgent();

    bool registerAgent(const QString &localDevice);
    bool unregisterAgent(const QString &localDevice);

    QBluetoothAuthorizationAgent::Error error() const;

protected:
    virtual bool authorize(const QString &localDevice,
                    const QBluetoothAddress &addr,
                    const QString &service,
                    const QString &uuid) = 0;
    virtual void cancel(const QString &localDevice,
                         const QBluetoothAddress &addr,
                         const QString &service,
                         const QString &uuid);
    virtual void release();

private:
    friend class QBluetoothAuthorizationAgentPrivate;
    QBluetoothAuthorizationAgentPrivate *m_data;
    Q_DISABLE_COPY(QBluetoothAuthorizationAgent)
};

#endif
