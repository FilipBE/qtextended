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

#ifndef QOBEXHEADER_H
#define QOBEXHEADER_H

#include <qobexnamespace.h>

#include <QVariant>
#include <QString>
#include <QDateTime>
#include <QUuid>

class QDataStream;
class QDebug;
class QObexHeaderPrivate;

class QTOPIAOBEX_EXPORT QObexHeader
{
public:
    enum HeaderId {
        Count = 0xc0,
        Name = 0x01,
        Type = 0x42,
        Length = 0xc3,
        Time = 0x44,
        Description = 0x05,
        Target = 0x46,
        Http = 0x47,
        Who = 0x4A,
        ConnectionId = 0xcb,
        AppParameters = 0x4c,
        AuthChallenge = 0x4d,
        AuthResponse = 0x4e,
        CreatorId = 0xcf,
        WanUuid = 0x50,
        ObjectClass = 0x51,
        SessionParameters = 0x52,
        SessionSequenceNumber = 0x93,

        MaximumHeaderId = 0x3f
    };

    QObexHeader();
    QObexHeader(const QObexHeader &other);
    ~QObexHeader();

    QObexHeader &operator=(const QObexHeader &other);

    bool operator==(const QObexHeader &other) const;
    inline bool operator!=(const QObexHeader &other) const { return !operator==(other); }

    bool contains(int headerId) const;
    QList<int> headerIds() const;
    int size() const;

    void setCount(quint32 count);
    quint32 count() const;

    void setName(const QString &name);
    QString name() const;

    void setType(const QString &type);
    QString type() const;

    void setLength(quint32 length);
    quint32 length() const;

    void setTime(const QDateTime &dateTime);
    QDateTime time() const;

    void setDescription(const QString &description);
    QString description() const;

    void setTarget(const QByteArray &target);
    QByteArray target() const;

    void setHttp(const QByteArray &http);
    QByteArray http() const;

    void setWho(const QByteArray &who);
    QByteArray who() const;

    void setConnectionId(quint32 connectionId);
    quint32 connectionId() const;

    void setAppParameters(const QByteArray &params);
    QByteArray appParameters() const;

    void setCreatorId(quint32 creatorId);
    quint32 creatorId() const;

    void setWanUuid(QUuid uuid);
    QUuid wanUuid() const;

    void setObjectClass(const QByteArray &objectClass);
    QByteArray objectClass() const;

    void setSessionParameters(const QByteArray &params);
    QByteArray sessionParameters() const;

    void setSessionSequenceNumber(quint8 num);
    quint8 sessionSequenceNumber() const;

    void setAuthenticationChallenge(QObex::AuthChallengeOptions options = 0,
                                    const QString &realm = QString());

    bool setValue(int headerId, const QVariant &variant);
    QVariant value(int headerId) const;
    bool remove(int headerId);
    void clear();

    static QDateTime dateTimeFromString(const QString &dateTimeString);
    static QString stringFromDateTime(const QDateTime &dateTime);

private:
    friend class QObexHeaderPrivate;
    QObexHeaderPrivate *m_data;
};

#ifndef QT_NO_DEBUG_STREAM
QTOPIAOBEX_EXPORT QDebug operator<<(QDebug, const QObexHeader &);
#endif

#endif
