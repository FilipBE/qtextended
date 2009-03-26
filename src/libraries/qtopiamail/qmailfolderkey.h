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

#ifndef QMAILFOLDERKEY_H
#define QMAILFOLDERKEY_H

#include "qmaildatacomparator.h"

#include <QList>
#include <QSharedData>
#include <QMailFolderId>
#include <QVariant>

#include <qtopiaglobal.h>

class QMailFolderKeyPrivate;

class QTOPIAMAIL_EXPORT QMailFolderKey
{
public:
    enum Property
    {
        Id,
        Name,
        ParentId,
        ParentAccountId,
        DisplayName,
        Status,
        AncestorFolderIds
    };

    typedef QMailFolderId IdType;

public:
    QMailFolderKey();
    QMailFolderKey(Property p, const QVariant& value, QMailDataComparator::Comparator c = QMailDataComparator::Equal);
    explicit QMailFolderKey(const QMailFolderIdList& ids);
    QMailFolderKey(const QMailFolderKey& other);
    virtual ~QMailFolderKey();

    QMailFolderKey operator~() const;
    QMailFolderKey operator&(const QMailFolderKey& other) const;
    QMailFolderKey operator|(const QMailFolderKey& other) const;
    QMailFolderKey& operator&=(const QMailFolderKey& other);
    QMailFolderKey& operator|=(const QMailFolderKey& other);

    bool operator==(const QMailFolderKey& other) const;
    bool operator !=(const QMailFolderKey& other) const;

    QMailFolderKey& operator=(const QMailFolderKey& other);

    bool isEmpty() const;
    bool isNonMatching() const;

    //for subqueries 
    operator QVariant() const;

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

    static QMailFolderKey nonMatchingKey();

private:
    friend class QMailStore;
    friend class QMailStorePrivate;

    void sanityCheck(Property p, const QVariant& value, QMailDataComparator::Comparator);

    QSharedDataPointer<QMailFolderKeyPrivate> d;
};

Q_DECLARE_USER_METATYPE(QMailFolderKey);

#endif
