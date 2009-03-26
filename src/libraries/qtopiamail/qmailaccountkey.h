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

#ifndef QMAILACCOUNTKEY_H
#define QMAILACCOUNTKEY_H

#include "qmaildatacomparator.h"

#include <QList>
#include <QMailAccountId>
#include <QSharedData>
#include <QVariant>

#include <qtopiaglobal.h>

class QMailAccountKeyPrivate;

class QTOPIAMAIL_EXPORT QMailAccountKey
{
public:
    enum Property
    {
        Id,
        Name,
        MessageType,
        EmailAddress
    };

    typedef QMailAccountId IdType;

public:
    QMailAccountKey();
    QMailAccountKey(Property p, const QVariant& value, QMailDataComparator::Comparator c = QMailDataComparator::Equal);
    explicit QMailAccountKey(const QMailAccountIdList& ids);
    QMailAccountKey(const QMailAccountKey& other);
    virtual ~QMailAccountKey();

    QMailAccountKey operator~() const;
    QMailAccountKey operator&(const QMailAccountKey& other) const;
    QMailAccountKey operator|(const QMailAccountKey& other) const;
    QMailAccountKey& operator&=(const QMailAccountKey& other);
    QMailAccountKey& operator|=(const QMailAccountKey& other);

    bool operator==(const QMailAccountKey& other) const;
    bool operator !=(const QMailAccountKey& other) const;

    QMailAccountKey& operator=(const QMailAccountKey& other);

    bool isEmpty() const;
    bool isNonMatching() const;

    //for subqueries
    operator QVariant() const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

    static QMailAccountKey nonMatchingKey();

private:
    friend class QMailStore;
    friend class QMailStorePrivate;

    void sanityCheck(Property p, const QVariant& value, QMailDataComparator::Comparator);

    QSharedDataPointer<QMailAccountKeyPrivate> d;
};

Q_DECLARE_USER_METATYPE(QMailAccountKey);

#endif
