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

#ifndef QCOLLECTIVEPRESENCE_H
#define QCOLLECTIVEPRESENCE_H

#include <qtopiaglobal.h>
#include <QAbstractIpcInterface>
#include <qcollectivenamespace.h>
#include <QMap>
#include <qcollectivepresenceinfo.h>

class QCollectivePresenceInfo;
class QString;

class QCollectivePresencePrivate;


class QTOPIACOLLECTIVE_EXPORT QCollectivePresence : public QAbstractIpcInterface
{
    Q_OBJECT
    Q_PROPERTY(QString protocol READ protocol)

public:
    explicit QCollectivePresence(const QString &service = QString(),
                                 QObject *parent = 0, QAbstractIpcInterface::Mode = Client);
    ~QCollectivePresence();

    QString protocol() const;

    QCollectivePresenceInfo localInfo() const;
    QCollectivePresenceInfo peerInfo(const QString &uri) const;

    QMap<QString, QCollectivePresenceInfo::PresenceType> statusTypes() const;

    QStringList pendingPublishRequests() const;
    QStringList pendingPeerSubscriptions() const;

    QStringList blockedPeers() const;
    QStringList subscribedPeers() const;
    QStringList publishedPeers() const;

public slots:
    virtual void setLocalPresence(const QCollectivePresenceInfo &info);
    virtual void subscribePeer(const QString &uri);
    virtual void unsubscribePeer(const QString &uri);

    virtual void authorizePeer(const QString &uri);
    virtual void denyPeer(const QString &uri);

    virtual void blockPeer(const QString &uri);
    virtual void unblockPeer(const QString &uri);

signals:
    void localPresenceChanged();
    void peerPresencesChanged(const QStringList &presences);

    void pendingPublishRequestsChanged();

    void blockedPeersChanged();
    void subscribedPeersChanged();
    void publishedPeersChanged();
    void pendingPeerSubscriptionsChanged();

protected:
    void publishPeerInfo(const QCollectivePresenceInfo &info);
    void removePeerInfo(const QString &uri);

private:
    QCollectivePresenceInfo readPeerInfo(const QString &uri) const;

    Q_DISABLE_COPY(QCollectivePresence)
    QCollectivePresencePrivate *d;
};

#endif
