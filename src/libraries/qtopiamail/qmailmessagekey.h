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

#ifndef QMAILMESSAGEKEY_H
#define QMAILMESSAGEKEY_H

#include "qmaildatacomparator.h"

#include <QFlags>
#include <QList>
#include <QMailMessageId>
#include <QSharedData>
#include <QVariant>

#include <qtopiaglobal.h>

class QMailMessageKeyPrivate;

class QTOPIAMAIL_EXPORT QMailMessageKey
{
public:
    enum Property
    {
        Id = 0x0001,
        Type = 0x0002,
        ParentFolderId = 0x0004,
        Sender = 0x0008,
        Recipients = 0x0010,
        Subject = 0x0020,
        TimeStamp = 0x0040, 
        Status = 0x0080,
        FromMailbox = 0x0200,
        ServerUid = 0x0400,      
        Size = 0x0800,
        ParentAccountId = 0x1000,
        AncestorFolderIds = 0x2000,
        ContentType = 0x4000,
        PreviousParentFolderId = 0x8000
    };
    Q_DECLARE_FLAGS(Properties,Property)

    typedef QMailMessageId IdType;

public:
    QMailMessageKey();
    QMailMessageKey(Property p, const QVariant& value, QMailDataComparator::Comparator c = QMailDataComparator::Equal);
    explicit QMailMessageKey(const QMailMessageIdList& ids);
    QMailMessageKey(const QMailMessageKey& other);
    virtual ~QMailMessageKey();

    QMailMessageKey operator~() const;
    QMailMessageKey operator&(const QMailMessageKey& other) const;
    QMailMessageKey operator|(const QMailMessageKey& other) const;
    QMailMessageKey& operator&=(const QMailMessageKey& other);
    QMailMessageKey& operator|=(const QMailMessageKey& other);

    bool operator==(const QMailMessageKey& other) const;
    bool operator !=(const QMailMessageKey& other) const;

    QMailMessageKey& operator=(const QMailMessageKey& other);

    bool isEmpty() const;
    bool isNonMatching() const;

    operator QVariant() const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

    static QMailMessageKey nonMatchingKey();

private:
    friend class QMailStore;
    friend class QMailStorePrivate;

    void sanityCheck(Property p, const QVariant& value, QMailDataComparator::Comparator);

    QSharedDataPointer<QMailMessageKeyPrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QMailMessageKey::Properties)

Q_DECLARE_USER_METATYPE(QMailMessageKey);

#endif
