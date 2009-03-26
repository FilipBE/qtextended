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

#ifndef QBLUETOOTHSDPUUID_H
#define QBLUETOOTHSDPUUID_H

#include <QVariant>
#include <QtGlobal>
#include <QMetaType>

#include <qbluetoothnamespace.h>

class QString;
struct QBluetoothSdpUuid_Private;

class QBLUETOOTH_EXPORT QBluetoothSdpUuid {
public:
    enum Type { UUID16, UUID32, UUID128 };

    QBluetoothSdpUuid();
    explicit QBluetoothSdpUuid(quint16 data);
    explicit QBluetoothSdpUuid(quint32 data);
    explicit QBluetoothSdpUuid(quint128 data);
    explicit QBluetoothSdpUuid(const QString &str);
    QBluetoothSdpUuid(const QBluetoothSdpUuid &other);

    QBluetoothSdpUuid &operator=(const QBluetoothSdpUuid &other);

    ~QBluetoothSdpUuid();

    bool operator==(const QBluetoothSdpUuid &other) const;
    bool operator!=(const QBluetoothSdpUuid &other) const
    {
        return !operator==(other);
    }

    QString toString() const;

    QBluetoothSdpUuid::Type type() const;
    QVariant uuid();
    bool isValid() const;

    QBluetoothSdpUuid toUuid128() const;

    static QBluetoothSdpUuid PublicBrowseGroup;
    static QBluetoothSdpUuid L2cap;
    static QBluetoothSdpUuid create16Bit(quint16 id);
    static QBluetoothSdpUuid create32Bit(quint32 id);
    static QBluetoothSdpUuid create128Bit(quint128 id);

    static QBluetoothSdpUuid fromProfile(QBluetooth::SDPProfile profile);

private:
    bool m_isValid;
    QVariant m_uuid;
    QBluetoothSdpUuid::Type m_type;
};

Q_DECLARE_METATYPE(QBluetoothSdpUuid)

#endif
