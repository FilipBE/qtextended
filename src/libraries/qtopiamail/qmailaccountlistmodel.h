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

#ifndef QMAILACCOUNTLISTMODEL_H
#define QMAILACCOUNTLISTMODEL_H

#include <QAbstractListModel>
#include "qmailaccountkey.h"
#include "qmailaccountsortkey.h"

class QMailAccountListModelPrivate;

class QTOPIAMAIL_EXPORT QMailAccountListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles
    {
        NameTextRole = Qt::UserRole,
        MessageTypeRole,
        MessageSourcesRole,
        MessageSinksRole
    };

public:
    QMailAccountListModel(QObject* parent = 0);
    virtual ~QMailAccountListModel();

    int rowCount(const QModelIndex& index = QModelIndex()) const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;

    QMailAccountKey key() const;
    void setKey(const QMailAccountKey& key);

    QMailAccountSortKey sortKey() const;
    void setSortKey(const QMailAccountSortKey& sortKey);

    QMailAccountId idFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromId(const QMailAccountId& id) const;

    bool synchronizeEnabled() const;
    void setSynchronizeEnabled(bool val);

private slots:
    void accountsAdded(const QMailAccountIdList& ids); 
    void accountsUpdated(const QMailAccountIdList& ids);
    void accountsRemoved(const QMailAccountIdList& ids);

private:
    void fullRefresh();

private:
    QMailAccountListModelPrivate* d;

};

#endif
