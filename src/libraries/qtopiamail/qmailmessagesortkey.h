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

#ifndef QMAILMESSAGESORTKEY_H
#define QMAILMESSAGESORTKEY_H

#include <qtopiaglobal.h>
#include <QSharedData>
#include <QtGlobal>

class QMailMessageSortKeyPrivate;

class QTOPIAMAIL_EXPORT QMailMessageSortKey
{
public:
    enum Property
    {
        Id,
        Type,
        ParentFolderId,
        Sender,
        Recipients,
        Subject,
        TimeStamp,
        Status,
        FromMailbox,
        ServerUid,
        Size,
        ParentAccountId,
        ContentType,
        PreviousParentFolderId
    };

public:
    QMailMessageSortKey();
    explicit QMailMessageSortKey(Property p, Qt::SortOrder order = Qt::AscendingOrder);
    QMailMessageSortKey(const QMailMessageSortKey& other);
    virtual ~QMailMessageSortKey();

    QMailMessageSortKey operator&(const QMailMessageSortKey& other) const;
    QMailMessageSortKey& operator&=(const QMailMessageSortKey& other);

    bool operator==(const QMailMessageSortKey& other) const;
    bool operator !=(const QMailMessageSortKey& other) const;

    QMailMessageSortKey& operator=(const QMailMessageSortKey& other);

    bool isEmpty() const;

private:
    friend class QMailStore;
    friend class QMailStorePrivate;

    QSharedDataPointer<QMailMessageSortKeyPrivate> d;
};

#endif
