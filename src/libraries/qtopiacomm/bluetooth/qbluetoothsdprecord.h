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

#ifndef QBLUETOOTHSDPRECORD_H
#define QBLUETOOTHSDPRECORD_H

#include <QMap>
#include <QVariant>
#include <QList>
#include <qglobal.h>

#include <qbluetoothglobal.h>
#include <qbluetoothnamespace.h>

class QString;
class QBluetoothSdpUuid;

class QBLUETOOTH_EXPORT QBluetoothSdpAlternative : public QList<QVariant>
{
public:
    bool operator==(const QBluetoothSdpAlternative &other) const;
    bool operator!=(const QBluetoothSdpAlternative &other) const {
        return !operator==(other);
    }
};

class QBLUETOOTH_EXPORT QBluetoothSdpSequence : public QList<QVariant>
{
public:
    bool operator==(const QBluetoothSdpSequence &other) const;
    bool operator!=(const QBluetoothSdpSequence &other) const {
        return !operator==(other);
    }
};

class QBLUETOOTH_EXPORT QBluetoothSdpRecord {
public:
    static int rfcommChannel(const QBluetoothSdpRecord &service);
    static QBluetoothSdpRecord fromDevice(QIODevice *device);
    static QBluetoothSdpRecord fromData(const QByteArray &data);

    QBluetoothSdpRecord();
    ~QBluetoothSdpRecord();

    bool isNull() const;

    QBluetoothSdpRecord(const QBluetoothSdpRecord &other);
    QBluetoothSdpRecord &operator=(const QBluetoothSdpRecord &other);
    bool operator==(const QBluetoothSdpRecord &other) const;

    bool isInstance(QBluetooth::SDPProfile profile) const;
    bool isInstance(const QBluetoothSdpUuid &uuid) const;

    QString serviceName() const;
    void setServiceName(const QString &serviceName);

    QString serviceDescription() const;
    void setServiceDescription(const QString &serviceDesc);

    QString providerName() const;
    void setProviderName(const QString &providerName);

    QUrl docUrl() const;
    void setDocUrl(const QUrl &docUrl);

    QUrl execUrl() const;
    void setExecUrl(const QUrl &execUrl);

    QUrl iconUrl() const;
    void setIconUrl(const QUrl &iconUrl);

    QBluetoothSdpUuid id() const;
    void setId(const QBluetoothSdpUuid &id);

    QList<QBluetoothSdpUuid> browseGroups() const;
    void setBrowseGroups(const QList<QBluetoothSdpUuid> &groups);

    QBluetoothSdpUuid group() const;
    void setGroup(const QBluetoothSdpUuid &group);

    quint32 recordHandle() const;
    void setRecordHandle(quint32 handle);

    QList<quint16> attributeIds() const;
    bool addAttribute(quint16 id, const QVariant &attr);
    bool removeAttribute(quint16 id);
    QVariant attribute(quint16 id, bool *ok = NULL) const;
    void clearAttributes();

private:
    QMap<quint16, QVariant> m_attrs;
};

Q_DECLARE_METATYPE(QBluetoothSdpSequence)
Q_DECLARE_METATYPE(QBluetoothSdpAlternative)

#endif
