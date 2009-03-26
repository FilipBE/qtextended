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

#ifndef QMAILMESSAGESET_H
#define QMAILMESSAGESET_H

#include "qprivateimplementation.h"

#include <QAbstractItemModel>
#include <QList>
#include <QMailAccountKey>
#include <QMailFolderKey>
#include <QMailMessageKey>
#include <QMap>
#include <QString>


class QMailMessageSet;
class QMailMessageSetModel;
class QMailStore;

class QMailMessageSetContainerPrivate;

class QTOPIAMAIL_EXPORT QMailMessageSetContainer : public QPrivatelyNoncopyable<QMailMessageSetContainerPrivate>
{
public:
    typedef QMailMessageSetContainerPrivate ImplementationType;

    virtual ~QMailMessageSetContainer();

    int count() const;
    QMailMessageSet *at(int i) const;
    int indexOf(QMailMessageSet *child) const;

    virtual void append(QMailMessageSet *child);
    virtual void update(QMailMessageSet *child);
    virtual void remove(QMailMessageSet *child);

    virtual void remove(const QList<QMailMessageSet*> &obsoleteChildren);
    virtual void removeDescendants();

    QMailMessageSetContainer *parentContainer();

    virtual QMailMessageSetModel *model() = 0;

protected:
    template<typename Subclass>
    QMailMessageSetContainer(Subclass *p);

    virtual void resyncState();

private:
    friend class QMailMessageSet;

    virtual QObject *qObject() = 0;
};


class QMailMessageSetPrivate;

class QTOPIAMAIL_EXPORT QMailMessageSet : public QObject, public QMailMessageSetContainer
{
    Q_OBJECT

public:
    typedef QMailMessageSetPrivate ImplementationType;

    QMailMessageSet(QMailMessageSetContainer *container);
    ~QMailMessageSet();

    virtual QMailMessageKey messageKey() const = 0;
    virtual QMailMessageKey descendantsMessageKey() const;

    virtual QString displayName() const = 0;

    QVariant data(int role, int column = 0);

    QModelIndex modelIndex(int column = 0);

    virtual QObject *qObject();
    virtual QMailMessageSetModel *model();

protected:
    friend class QMailMessageSetContainer;

    template<typename Subclass>
    QMailMessageSet(Subclass *p, QMailMessageSetContainer *container);

    virtual void init();
};


class QMailFolderMessageSetPrivate;

class QTOPIAMAIL_EXPORT QMailFolderMessageSet : public QMailMessageSet
{
    Q_OBJECT

public:
    typedef QMailFolderMessageSetPrivate ImplementationType;

    QMailFolderMessageSet(QMailMessageSetContainer *container, const QMailFolderId &folderId, bool hierarchical = true);

    QMailFolderId folderId() const;
    bool hierarchical() const;

    virtual QMailMessageKey messageKey() const;
    virtual QMailMessageKey descendantsMessageKey() const;

    virtual QString displayName() const;

    static QMailMessageKey contentKey(const QMailFolderId &id, bool descendants);

protected slots:
    virtual void foldersAdded(const QMailFolderIdList &ids);
    virtual void foldersRemoved(const QMailFolderIdList &ids);
    virtual void foldersUpdated(const QMailFolderIdList &ids);
    virtual void folderContentsModified(const QMailFolderIdList &ids);

protected:
    virtual void init();
    virtual void synchronizeChildren();
    virtual void createChild(const QMailFolderId &childId);
    virtual void resyncState();

    QMailFolderKey folderKey() const;
};


class QMailAccountMessageSetPrivate;

class QTOPIAMAIL_EXPORT QMailAccountMessageSet : public QMailMessageSet
{
    Q_OBJECT

public:
    typedef QMailAccountMessageSetPrivate ImplementationType;

    QMailAccountMessageSet(QMailMessageSetContainer *container, const QMailAccountId &accountId, bool hierarchical = true);

    QMailAccountId accountId() const;
    bool hierarchical() const;

    virtual QMailMessageKey messageKey() const;
    virtual QMailMessageKey descendantsMessageKey() const;

    virtual QString displayName() const;

    static QMailMessageKey contentKey(const QMailAccountId &id, bool descendants);

protected slots:
    virtual void foldersAdded(const QMailFolderIdList &ids);
    virtual void foldersRemoved(const QMailFolderIdList &ids);
    virtual void foldersUpdated(const QMailFolderIdList &ids);
    virtual void accountsUpdated(const QMailAccountIdList &ids);
    virtual void accountContentsModified(const QMailAccountIdList &ids);

protected:
    virtual void init();
    virtual void synchronizeChildren();
    virtual void createChild(const QMailFolderId &childId);
    virtual void resyncState();

    QMailFolderKey rootFolderKey() const;
};


class QMailFilterMessageSetPrivate;

class QTOPIAMAIL_EXPORT QMailFilterMessageSet : public QMailMessageSet
{
    Q_OBJECT

public:
    typedef QMailFilterMessageSetPrivate ImplementationType;

    QMailFilterMessageSet(QMailMessageSetContainer *container, const QMailMessageKey &key, const QString &name, bool minimalUpdates = true);

    virtual QMailMessageKey messageKey() const;
    virtual void setMessageKey(const QMailMessageKey &key);

    virtual QString displayName() const;
    virtual void setDisplayName(const QString &name);

    virtual bool updatesMinimized() const;
    virtual void setUpdatesMinimized(bool set);

protected slots:
    virtual void messagesAdded(const QMailMessageIdList &ids);
    virtual void messagesRemoved(const QMailMessageIdList &ids);
    virtual void messagesUpdated(const QMailMessageIdList &ids);
    virtual void folderContentsModified(const QMailFolderIdList &ids);

protected:
    virtual void init();
    virtual void reset();
    virtual void resyncState();
};


class QMailMessageSetModelPrivate;

class QTOPIAMAIL_EXPORT QMailMessageSetModel : public QAbstractItemModel, public QMailMessageSetContainer
{
    Q_OBJECT

public:
    typedef QMailMessageSetModelPrivate ImplementationType;

    enum Roles
    {
        DisplayNameRole = Qt::UserRole,
        MessageKeyRole,
        SubclassUserRole
    };

    QMailMessageSetModel(QObject *parent = 0);
    virtual ~QMailMessageSetModel();


    virtual int rowCount(const QModelIndex &parentIndex) const;
    virtual int columnCount(const QModelIndex &) const;

    bool isEmpty() const;

    QModelIndex index(int row, int column, const QModelIndex &parentIndex) const;
    QModelIndex parent(const QModelIndex &index) const;

    QModelIndex indexFromAccountId(const QMailAccountId &id) const;
    QModelIndex indexFromFolderId(const QMailFolderId &id) const;

    QMailAccountId accountIdFromIndex(const QModelIndex &index) const;
    QMailFolderId folderIdFromIndex(const QModelIndex &index) const;

    QMailMessageSet *itemFromIndex(const QModelIndex &index) const;
    QModelIndex indexFromItem(QMailMessageSet *item) const;

    QVariant data(const QModelIndex &index, int role) const;

    virtual QVariant data(QMailMessageSet *item, int role, int column) const;

    virtual QMailMessageSetModel *model();

    bool ignoreMailStoreUpdates() const;
    void setIgnoreMailStoreUpdates(bool ignore);

signals:
    void accountsAdded(const QMailAccountIdList &ids);
    void accountsUpdated(const QMailAccountIdList &ids);
    void accountsRemoved(const QMailAccountIdList &ids);
    void accountContentsModified(const QMailAccountIdList &ids);

    void foldersAdded(const QMailFolderIdList &ids);
    void foldersRemoved(const QMailFolderIdList &ids);
    void foldersUpdated(const QMailFolderIdList &ids);
    void folderContentsModified(const QMailFolderIdList &ids);

    void messagesAdded(const QMailMessageIdList &ids);
    void messagesRemoved(const QMailMessageIdList &ids);
    void messagesUpdated(const QMailMessageIdList &ids);

protected slots:
    void mailStoreAccountsAdded(const QMailAccountIdList &ids);
    void mailStoreAccountsUpdated(const QMailAccountIdList &ids);
    void mailStoreAccountsRemoved(const QMailAccountIdList &ids);
    void mailStoreAccountContentsModified(const QMailAccountIdList &ids);

    void mailStoreFoldersAdded(const QMailFolderIdList &ids);
    void mailStoreFoldersRemoved(const QMailFolderIdList &ids);
    void mailStoreFoldersUpdated(const QMailFolderIdList &ids);
    void mailStoreFolderContentsModified(const QMailFolderIdList &ids);

    void mailStoreMessagesAdded(const QMailMessageIdList &ids);
    void mailStoreMessagesRemoved(const QMailMessageIdList &ids);
    void mailStoreMessagesUpdated(const QMailMessageIdList &ids);

    void ceasePropagatingUpdates();

    void delayedInit();

protected:
    QMailAccountId itemAccountId(QMailMessageSet *item) const;
    QMailFolderId itemFolderId(QMailMessageSet *item) const;

    virtual void appended(QMailMessageSet *child);
    virtual void removed(QMailMessageSet *child);
    virtual void updated(QMailMessageSet *child);


private:
    friend class QMailMessageSetContainer;
    friend class QMailMessageSet;

    virtual QObject *qObject();

    void beginAppend(QMailMessageSet *child);
    void endAppend(QMailMessageSet *child);

    void beginRemove(QMailMessageSet *child);
    void endRemove(QMailMessageSet *child);

    void doUpdate(QMailMessageSet *child);

    bool propagateUpdates() const;
    void testForResync();

    QModelIndex index(QMailMessageSet *item, int column) const;
    QModelIndex parentIndex(QMailMessageSet *item, int column) const;
};


#endif

