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

#ifndef QMAILFOLDER_H
#define QMAILFOLDER_H

#include "qmailid.h"
#include <QString>
#include <QList>
#include <QSharedData>
#include <qtopiaglobal.h>

class QMailFolderPrivate;

class QTOPIAMAIL_EXPORT QMailFolder
{
public:
    enum StandardFolder
    {
        InboxFolder = 1,
        OutboxFolder = 2,
        DraftsFolder = 3,
        SentFolder = 4,
        TrashFolder = 5
    };

    static const quint64 &SynchronizationEnabled;
    static const quint64 &Synchronized;

    QMailFolder();

    QMailFolder(const QString& name,
                const QMailFolderId& parentFolderId = QMailFolderId(),
                const QMailAccountId& parentAccountId = QMailAccountId());

    explicit QMailFolder(const StandardFolder& sf);

    explicit QMailFolder(const QMailFolderId& id);

    QMailFolder(const QMailFolder& other);

    virtual ~QMailFolder();

    QMailFolder& operator=(const QMailFolder& other);

    QMailFolderId id() const;
    void setId(const QMailFolderId& id);

    QString name() const;
    void setName(const QString& name);

    QString displayName() const;
    void setDisplayName(const QString& name);

    QMailFolderId parentId() const;
    void setParentId(const QMailFolderId& id);

    QMailAccountId parentAccountId() const;
    void setParentAccountId(const QMailAccountId& id);

    quint64 status() const;
    void setStatus(quint64 newStatus);
    void setStatus(quint64 mask, bool set);

    bool isRoot() const;

    static quint64 statusMask(const QString &flagName);

private:
    friend class QMailStore;
    friend class QMailStorePrivate;

    static void initStore();

private:
    QSharedDataPointer<QMailFolderPrivate> d;
};

typedef QList<QMailFolder> QMailFolderList;

#endif
