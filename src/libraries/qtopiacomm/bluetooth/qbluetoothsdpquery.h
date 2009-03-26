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

#ifndef QBLUETOOTHSDPQUERY_H
#define QBLUETOOTHSDPQUERY_H

#include <qbluetoothsdprecord.h>
#include <qbluetoothsdpuuid.h>
#include <qbluetoothnamespace.h>

#include <QString>
#include <QList>
#include <qglobal.h>
#include <qobject.h>

class QBluetoothAddress;
class QBluetoothLocalDevice;
class QBluetoothSdpUuid;

class QBluetoothSdpQuery_Private;

class QBLUETOOTH_EXPORT QBluetoothSdpQueryResult
{
public:
    QBluetoothSdpQueryResult();

    QBluetoothSdpQueryResult(const QBluetoothSdpQueryResult &other);
    QBluetoothSdpQueryResult &operator=(const QBluetoothSdpQueryResult &other);

    bool isValid() const;

    void setError(const QString &error);
    QString error() const;

    const QList<QBluetoothSdpRecord> &services() const;
    void addService(const QBluetoothSdpRecord &service);

    void reset();

private:
    QList<QBluetoothSdpRecord> m_services;
    QString m_error;
    bool m_valid;
};

class QBLUETOOTH_EXPORT QBluetoothSdpQuery : public QObject
{
    Q_OBJECT

    friend class QBluetoothSdpQuery_Private;

public:
    QBluetoothSdpQuery( QObject* parent = 0 );
    ~QBluetoothSdpQuery();

    bool searchServices(const QBluetoothAddress &remote,
                        const QBluetoothLocalDevice &local,
                        QBluetooth::SDPProfile profile);
    bool searchServices(const QBluetoothAddress &remote,
                        const QBluetoothLocalDevice &local,
                        const QBluetoothSdpUuid &uuid);
    bool browseServices(const QBluetoothAddress &remote,
                        const QBluetoothLocalDevice &local);

public slots:
    void cancelSearch();

signals:
    void searchComplete(const QBluetoothSdpQueryResult &result);
    void searchCancelled();

private:
    QBluetoothSdpQuery_Private *m_data;
    Q_DISABLE_COPY(QBluetoothSdpQuery)
};

#endif
