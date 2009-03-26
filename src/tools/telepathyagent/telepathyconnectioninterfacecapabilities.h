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

#ifndef TELEPATHYCONNECTIONINTERFACECAPABILITIES_H
#define TELEPATHYCONNECTIONINTERFACECAPABILITIES_H

#include <QObject>
#include <QString>

template <class T> class QList;
template <class T, class U> class QMap;
class QStringList;
class TelepathyConnectionInterfaceCapabilitiesPrivate;
class TelepathyConnection;
class QDBusMessage;

class TelepathyConnectionInterfaceCapabilities : public QObject
{
    Q_OBJECT

public:
    TelepathyConnectionInterfaceCapabilities(TelepathyConnection *conn);
    ~TelepathyConnectionInterfaceCapabilities();

    enum CapabilityFlag {
        Create = 1,
        Invite = 2
    };

    Q_DECLARE_FLAGS(CapabilityFlags, CapabilityFlag)

    struct CapabilityPair {
        QString channel_type;
        uint type_specific_flags;
    };

    struct ContactCapability {
        uint handle;
        QString channel_type;
        CapabilityFlags generic_flags;
        uint type_specific_flags;
    };

    struct CapabilityChange {
        uint handle;
        QString channel_type;
        CapabilityFlags old_generic_flags;
        CapabilityFlags new_generic_flags;
        uint old_type_specific_flags;
        uint new_type_specific_flags;
    };

    bool advertiseCapabilities(const QList<TelepathyConnectionInterfaceCapabilities::CapabilityPair> &add,
                               const QStringList &remove);
    bool retrieveCapabilities(const QList<uint> &handles);

signals:
    void capabilitiesRetrieved(const QMap<uint,
                               TelepathyConnectionInterfaceCapabilities::ContactCapability> &capabilities);
    void capabilitiesChanged(const QList<TelepathyConnectionInterfaceCapabilities::CapabilityChange> &capabilities);

private slots:
    void capabilitiesChanged(const QDBusMessage &message);

private:
    TelepathyConnectionInterfaceCapabilitiesPrivate *m_data;
    Q_DISABLE_COPY(TelepathyConnectionInterfaceCapabilities)
    friend class CapabilitiesRequestHelper;
};

#endif
