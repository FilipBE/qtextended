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

#ifndef QBLUETOOTHABSTRACTSERVICE_H
#define QBLUETOOTHABSTRACTSERVICE_H

#include <qbluetoothnamespace.h>

class QBluetoothAddress;
class QBluetoothSdpRecord;
class QBluetoothAbstractServicePrivate;

class QBLUETOOTH_EXPORT QBluetoothAbstractService : public QObject
{
    Q_OBJECT

public:
    enum Error {
        NoError,
        InvalidArguments,
        NotConnected,
        NotAuthorized,
        NotAvailable,
        DoesNotExist,
        Failed,
        Rejected,
        Canceled,
        UnknownError
    };

    QBluetoothAbstractService(const QString &name, const QString &displayName, QObject *parent = 0);
    QBluetoothAbstractService(const QString &name, const QString &displayName, const QString &description, QObject *parent = 0);
    virtual ~QBluetoothAbstractService();

    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void setSecurityOptions(QBluetooth::SecurityOptions options) = 0;

    QString name() const;
    QString displayName() const;
    QString description() const;

protected:
    quint32 registerRecord(const QBluetoothSdpRecord &record);
    quint32 registerRecord(const QString &filename);
    bool updateRecord(quint32 handle, const QBluetoothSdpRecord &record);
    bool updateRecord(quint32 handle, const QString &filename);
    bool unregisterRecord(quint32 handle);

    bool requestAuthorization(const QBluetoothAddress &addr, const QString &uuid = QString());
    bool cancelAuthorization(const QBluetoothAddress &addr, const QString &uuid = QString());

    QBluetoothAbstractService::Error error() const;
    QString errorString() const;

private:
    friend class QBluetoothAbstractServicePrivate;
    QBluetoothAbstractServicePrivate *m_data;

signals:
    void started(bool error, const QString &description);
    void stopped();

    void authorizationSucceeded();
    void authorizationFailed();
};

#endif
