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

#ifndef QCOLLECTIVEPRESENCEINFO_H
#define QCOLLECTIVEPRESENCEINFO_H

#include <qtopiaglobal.h>
#include <QSharedData>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <qtopiaipcmarshal.h>

class QCollectivePresenceInfoData;

class QTOPIACOLLECTIVE_EXPORT QCollectivePresenceInfo
{
public:
    explicit QCollectivePresenceInfo(const QString &uri = QString() );
    QCollectivePresenceInfo(const QCollectivePresenceInfo &other);
    ~QCollectivePresenceInfo();

    bool isNull() const;

    QString uri() const;
    void setUri(const QString &uri);

    enum PresenceType
    {
        None = 0,
        Offline,
        Online,
        Away,
        ExtendedAway,
        Hidden,
        Busy
    };

    QString presence() const;
    PresenceType presenceType() const;
    void setPresence(const QString &presence, PresenceType type);

    QString displayName() const;
    void setDisplayName( const QString &displayName );

    QStringList capabilities() const;
    void setCapabilities(const QStringList &capabilities);

    QString message() const;
    void setMessage(const QString &msg);

    QString avatar() const;
    void setAvatar(const QString &avatar);

    QDateTime lastUpdateTime() const;
    void setLastUpdateTime( const QDateTime &ts );

    QVariantMap properties() const;
    void setProperties(const QVariantMap &properties);

    bool operator==(const QCollectivePresenceInfo &other) const;
    bool operator!=(const QCollectivePresenceInfo &other) const;

    QCollectivePresenceInfo &operator=(const QCollectivePresenceInfo &other);

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QSharedDataPointer<QCollectivePresenceInfoData> d;
};

Q_DECLARE_USER_METATYPE(QCollectivePresenceInfo);
Q_DECLARE_USER_METATYPE_NO_OPERATORS(QList<QCollectivePresenceInfo>);
Q_DECLARE_USER_METATYPE_ENUM(QCollectivePresenceInfo::PresenceType);
#ifndef QT_NO_DEBUG_STREAM
QTOPIACOLLECTIVE_EXPORT QDebug operator<<(QDebug, const QCollectivePresenceInfo &);
#endif

#endif
