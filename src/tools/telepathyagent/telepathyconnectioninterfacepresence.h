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

#ifndef TELEPATHYCONNECTIONINTERFACEPRESENCE_H
#define TELEPATHYCONNECTIONINTERFACEPRESENCE_H

#include <QObject>
#include <QDateTime>
#include <QVariantMap>

class TelepathyConnection;
class TelepathyConnectionInterfacePresencePrivate;
class QStringList;
class QDBusMessage;
class QDBusError;
template <class T> class QList;
class QDateTime;

class TelepathyConnectionInterfacePresence : public QObject
{
    Q_OBJECT

public:
    enum PresenceType {
        Unset = 0,
        Offline = 1,
        Available = 2,
        Away = 3,
        ExtendedAway = 4,
        Hidden = 5,
        Busy = 6
    };

    struct LastActivityAndStatuses {
        QDateTime lastActivity;
        QString status;
        QVariantMap parameters;
    };

    typedef QMap<uint, LastActivityAndStatuses> ContactPresences;

    struct StatusInfo {
        PresenceType type;
        bool maySetOnSelf;
        bool exclusive;
        QMap<QString, QString> parameter_types;
    };

    TelepathyConnectionInterfacePresence(TelepathyConnection *conn);
    ~TelepathyConnectionInterfacePresence();

    bool setStatus(const QString &status, const QVariantMap &parameters);
    bool clearStatus();

    QMap<QString, StatusInfo> availableStatuses() const;

    bool setLastActivityTime(const QDateTime &time);

    ContactPresences readCachedPresence(const QList<uint> &contacts);
    bool requestPresence(const QList<uint> &contacts);

signals:
    void presenceUpdate(const TelepathyConnectionInterfacePresence::ContactPresences &presences);

private slots:
    void presenceUpdate(const QDBusMessage &msg);

private:
    Q_DISABLE_COPY(TelepathyConnectionInterfacePresence)
    TelepathyConnectionInterfacePresencePrivate *m_data;
};

#endif
