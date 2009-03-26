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

#ifndef QMAILFOLDERSORTKEY_H
#define QMAILFOLDERSORTKEY_H

#include <qtopiaglobal.h>
#include <QSharedData>
#include <QtGlobal>

class QMailFolderSortKeyPrivate;

class QTOPIAMAIL_EXPORT QMailFolderSortKey
{
public:
    enum Property
    {
        Id,
        Name,
        ParentId,
        ParentAccountId,
        DisplayName,
        Status
    };

public:
    QMailFolderSortKey();
    explicit QMailFolderSortKey(Property p, Qt::SortOrder order = Qt::AscendingOrder);
    QMailFolderSortKey(const QMailFolderSortKey& other);
    virtual ~QMailFolderSortKey();

    QMailFolderSortKey operator&(const QMailFolderSortKey& other) const;
    QMailFolderSortKey& operator&=(const QMailFolderSortKey& other);

    bool operator==(const QMailFolderSortKey& other) const;
    bool operator !=(const QMailFolderSortKey& other) const;

    QMailFolderSortKey& operator=(const QMailFolderSortKey& other);

    bool isEmpty() const;

private:
    friend class QMailStore;
    friend class QMailStorePrivate;

    QSharedDataPointer<QMailFolderSortKeyPrivate> d;
};

#endif
