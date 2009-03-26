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

#ifndef QBLUETOOTHPASSKEYAGENT_H
#define QBLUETOOTHPASSKEYAGENT_H

#include <QObject>

#include <qbluetoothglobal.h>
class QBluetoothPasskeyAgent_Private;
class QBluetoothLocalDevice;
class QBluetoothPasskeyRequest;
class QString;
class QBluetoothAddress;

class QBLUETOOTH_EXPORT QBluetoothPasskeyAgent : public QObject
{
    Q_OBJECT

public:
    explicit QBluetoothPasskeyAgent(const QString &name, QObject *parent = 0);
    virtual ~QBluetoothPasskeyAgent();

    QString name() const;

protected:
    virtual void requestPasskey(QBluetoothPasskeyRequest &req) = 0;
    virtual void cancelRequest(const QString &localDevice,
                               const QBluetoothAddress &remoteAddr);
    virtual bool confirmPasskey(const QString &localDevice,
                                const QBluetoothAddress &remoteAddr,
                                const QString &passkey);
    virtual void release();

public:
    enum Error {
        NoError,
        AlreadyExists,
        DoesNotExist,
        UnknownAddress,
        UnknownError };

    QBluetoothPasskeyAgent::Error error() const;

    bool registerDefault();
    bool unregisterDefault();
    bool registerDefault(const QString &localDevice);
    bool unregisterDefault(const QString &localDevice);

    bool registerForAddress(QBluetoothAddress &addr);
    bool unregisterForAddress(QBluetoothAddress &addr);
    bool registerForAddress(const QString &localDevice, QBluetoothAddress &addr);
    bool unregisterForAddress(const QString &localDevice, QBluetoothAddress &addr);

private:
    friend class QBluetoothPasskeyAgent_Private;
    QBluetoothPasskeyAgent_Private *m_data;
    Q_DISABLE_COPY(QBluetoothPasskeyAgent)
};

#endif
