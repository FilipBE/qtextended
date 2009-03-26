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

#ifndef TELEPATHYCONNECTION_H
#define TELEPATHYCONNECTION_H

#include <qglobal.h>
#include <QObject>

#include "telepathynamespace.h"

#include <QDBusObjectPath>

class QDBusError;
class QDBusMessage;
class TelepathyConnectionPrivate;
class TelepathyChannel;

template <class T> class QList;

class TelepathyConnection : public QObject
{
    Q_OBJECT

public:
    enum ConnectionState {
        ConnectedState = 0,
        ConnectingState,
        DisconnectedState
    };

    enum ConnectionStateReason {
        None = 0,
        Requested,
        NetworkError,
        AuthenticationFailed,
        EncryptionError,
        NameInUse,
        CertNotProvided,
        CertUntrusted,
        CertExpired,
        CertNotActivated,
        HostnameMismatch,
        FingerprintMismatch,
        CertSelfSigned,
        UnknownError
    };

    struct ChannelInfo {
        QDBusObjectPath path;
        QString type;
        Telepathy::HandleType handleType;
        uint handle;
    };

    TelepathyConnection(const QString &busName, const QDBusObjectPath &path,
                        QObject *parent = 0);
    ~TelepathyConnection();

    ConnectionState state() const;

    bool connectToServer();
    void disconnectFromServer();

    bool isValid() const;

    QString protocol() const;
    QStringList interfaces() const;

    QList<TelepathyConnection::ChannelInfo> channels() const;
    bool requestChannel(const QString &type,
                        Telepathy::HandleType handleType,
                        uint handle,
                        bool suppressHandler,
                        QObject *receiver, const char *slot);

    QList<uint> requestHandles(Telepathy::HandleType type,
                               const QStringList &handles);
    QStringList inspectHandles(Telepathy::HandleType type,
                               const QList<uint> &handles) const;
    bool holdHandles(Telepathy::HandleType type,
                     const QList<uint> &handles);
    bool releaseHandles(Telepathy::HandleType type,
                        const QList<uint> &handles);

    uint selfHandle() const;

    QDBusObjectPath path() const;
    QString service() const;

signals:
    void connectionStatusChanged(TelepathyConnection::ConnectionState state,
                                 TelepathyConnection::ConnectionStateReason);
    void newChannel(const TelepathyConnection::ChannelInfo &info, bool suppressHandler);

private slots:
    void connectionStatusChanged(uint status, uint reason);
    void newChannel(const QDBusObjectPath &path, const QString &type,
                    uint handle_type, uint handle, bool suppress_handler);
    void connectSucceeded(const QDBusMessage &msg);
    void connectFailed(const QDBusError &error);

private:
    Q_DISABLE_COPY(TelepathyConnection)
    TelepathyConnectionPrivate *m_data;
};

#endif
