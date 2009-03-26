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

#ifndef QMAILID_H
#define QMAILID_H

#include <QDebug>
#include <QString>
#include <QVariant>
#include <qtopiaglobal.h>
#include <QSharedData>
#include <qtopiaipcmarshal.h>

class MailIdPrivate;

class QTOPIAMAIL_EXPORT MailId
{
private:
    friend class QMailAccountId;
    friend class QMailFolderId;
    friend class QMailMessageId;

    explicit MailId(quint64 value);

    QSharedDataPointer<MailIdPrivate> d;

public:
    MailId();
    MailId(const MailId& other);
    virtual ~MailId();

    MailId& operator=(const MailId& other);

    bool isValid() const;
    quint64 toULongLong() const;

    bool operator!=(const MailId& other) const;
    bool operator==(const MailId& other) const;
    bool operator<(const MailId& other) const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);
};


class QTOPIAMAIL_EXPORT QMailAccountId : private MailId
{
public:
    QMailAccountId();
    explicit QMailAccountId(quint64 value);
    QMailAccountId(const QMailAccountId& other);
    virtual ~QMailAccountId();

    QMailAccountId& operator=(const QMailAccountId& other);

    bool isValid() const;
    quint64 toULongLong() const;

    operator QVariant() const;

    bool operator!=(const QMailAccountId& other) const;
    bool operator==(const QMailAccountId& other) const;
    bool operator<(const QMailAccountId& other) const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

    friend QDebug& operator<<(QDebug&, const QMailAccountId&);
};


class QTOPIAMAIL_EXPORT QMailFolderId : private MailId
{
public:
    QMailFolderId();
    explicit QMailFolderId(quint64 value);
    QMailFolderId(const QMailFolderId& other);
    virtual ~QMailFolderId();

    QMailFolderId& operator=(const QMailFolderId& other);

    bool isValid() const;
    quint64 toULongLong() const;

    operator QVariant() const;

    bool operator!=(const QMailFolderId& other) const;
    bool operator==(const QMailFolderId& other) const;
    bool operator<(const QMailFolderId& other) const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

    friend QDebug& operator<<(QDebug&, const QMailFolderId&);
};


class QTOPIAMAIL_EXPORT QMailMessageId : private MailId
{
public:
    QMailMessageId();
    explicit QMailMessageId(quint64 value);
    QMailMessageId(const QMailMessageId& other);
    virtual ~QMailMessageId();

    QMailMessageId& operator=(const QMailMessageId& other);

    bool isValid() const;
    quint64 toULongLong() const;

    operator QVariant() const;

    bool operator!=(const QMailMessageId& other) const;
    bool operator==(const QMailMessageId& other) const;
    bool operator<(const QMailMessageId& other) const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

    friend QDebug& operator<<(QDebug&, const QMailMessageId&);
};


typedef QList<QMailAccountId> QMailAccountIdList;
typedef QList<QMailFolderId> QMailFolderIdList;
typedef QList<QMailMessageId> QMailMessageIdList;

QTOPIAMAIL_EXPORT QDebug& operator<< (QDebug& debug, const MailId &id);
QTOPIAMAIL_EXPORT QDebug& operator<< (QDebug& debug, const QMailAccountId &id);
QTOPIAMAIL_EXPORT QDebug& operator<< (QDebug& debug, const QMailFolderId &id);
QTOPIAMAIL_EXPORT QDebug& operator<< (QDebug& debug, const QMailMessageId &id);

Q_DECLARE_USER_METATYPE(MailId);
Q_DECLARE_USER_METATYPE(QMailAccountId);
Q_DECLARE_USER_METATYPE(QMailFolderId);
Q_DECLARE_USER_METATYPE(QMailMessageId);

Q_DECLARE_METATYPE(QMailAccountIdList)
Q_DECLARE_METATYPE(QMailFolderIdList)
Q_DECLARE_METATYPE(QMailMessageIdList)

Q_DECLARE_USER_METATYPE_TYPEDEF(QMailAccountIdList, QMailAccountIdList)
Q_DECLARE_USER_METATYPE_TYPEDEF(QMailFolderIdList, QMailFolderIdList)
Q_DECLARE_USER_METATYPE_TYPEDEF(QMailMessageIdList, QMailMessageIdList)

uint QTOPIAMAIL_EXPORT qHash(const QMailAccountId &id);
uint QTOPIAMAIL_EXPORT qHash(const QMailFolderId &id);
uint QTOPIAMAIL_EXPORT qHash(const QMailMessageId &id);

#endif
