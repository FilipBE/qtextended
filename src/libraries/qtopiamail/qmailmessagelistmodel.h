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

#ifndef QMAILMESSAGELISTMODEL_H
#define QMAILMESSAGELISTMODEL_H

#include <QAbstractListModel>
#include "qmailmessagekey.h"
#include "qmailmessagesortkey.h"

class QMailMessageListModelPrivate;

class QTOPIAMAIL_EXPORT QMailMessageListModel : public QAbstractListModel
{
    Q_OBJECT 

public:
    enum Roles 
    {
        MessageAddressTextRole = Qt::UserRole, //sender or recipient address
        MessageSubjectTextRole, //subject
        MessageFilterTextRole, //the address text role concatenated with subject text role
        MessageTimeStampTextRole, //timestamp text
        MessageTypeIconRole, //mms,email,sms icon
        MessageStatusIconRole, //read,unread,downloaded icon 
        MessageDirectionIconRole, //incoming or outgoing message icon
        MessagePresenceIconRole, 
        MessageBodyTextRole, //body text, only if textual data
        MessageIdRole
    };

public:
    QMailMessageListModel(QObject* parent = 0);
    virtual ~QMailMessageListModel();

    int rowCount(const QModelIndex& index = QModelIndex()) const;

    bool isEmpty() const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);

    QMailMessageKey key() const;
    void setKey(const QMailMessageKey& key);

    QMailMessageSortKey sortKey() const;
    void setSortKey(const QMailMessageSortKey& sortKey);

    QMailMessageId idFromIndex(const QModelIndex& index) const;
    QModelIndex indexFromId(const QMailMessageId& id) const;

    bool ignoreMailStoreUpdates() const;
    void setIgnoreMailStoreUpdates(bool ignore);

signals:
    void modelChanged();

private slots:
    void messagesAdded(const QMailMessageIdList& ids); 
    void messagesUpdated(const QMailMessageIdList& ids);
    void messagesRemoved(const QMailMessageIdList& ids);
    void contactModelReset();

private:
    void fullRefresh(bool modelChanged);

private:
    QMailMessageListModelPrivate* d;
};

#endif
