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

#ifndef TELEPATHYCONNECTIONINTERFACEAVATARS_H
#define TELEPATHYCONNECTIONINTERFACEAVATARS_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QStringList>

template <class T> class QList;
template <class T, class U> class QMap;
class TelepathyConnectionInterfaceAvatarsPrivate;
class TelepathyConnection;
class QDBusMessage;

class TelepathyConnectionInterfaceAvatars : public QObject
{
    Q_OBJECT

public:
    TelepathyConnectionInterfaceAvatars(TelepathyConnection *conn);
    ~TelepathyConnectionInterfaceAvatars();

    struct AvatarRequirements {
        QStringList mimeTypes;
        quint16 minimumWidth;
        quint16 minimumHeight;
        quint16 maximumWidth;
        quint16 maximumHeight;
        uint maximumSize;
    };

    struct AvatarData {
        QByteArray imageData;
        QString mimeType;
    };

    AvatarRequirements avatarRequirements() const;
    QMap<uint, QString> knownAvatarTokens(const QList<uint> &contacts) const;
    AvatarData requestAvatar(uint contact);
    void requestAvatars(const QList<uint> &contacts);
    bool setAvatar(const AvatarData &avatar);
    bool clearAvatar();

signals:
    void avatarUpdated(uint contact, const QString &newAvatarToken);
    void avatarRetrieved(uint contact, const QString &token,
                         const TelepathyConnectionInterfaceAvatars::AvatarData &avatar);

private slots:
    void avatarRetrieved(const QDBusMessage &message);

private:
    TelepathyConnectionInterfaceAvatarsPrivate *m_data;
    Q_DISABLE_COPY(TelepathyConnectionInterfaceAvatars)
};

#endif
