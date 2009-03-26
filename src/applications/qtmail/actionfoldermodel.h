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

#ifndef ACTIONFOLDERMODEL_H
#define ACTIONFOLDERMODEL_H

#include "foldermodel.h"
#include <QMap>

// A message set that returns only the non-email messages within a folder:

class ActionFolderMessageSet : public QMailFolderMessageSet
{
    Q_OBJECT

public:
    ActionFolderMessageSet(QMailMessageSetContainer *container, const QMailFolderId &folderId, bool hierarchical);

    virtual QMailMessageKey messageKey() const;

    static QMailMessageKey contentKey(const QMailFolderId &id);

protected:
    virtual void createChild(const QMailFolderId &childId);
};


// A folder element which represents the compose selection

class ComposeActionMessageSet : public QMailFilterMessageSet
{
    Q_OBJECT

public:
    ComposeActionMessageSet(QMailMessageSetContainer *container, const QString &name);

protected:
    void init();

private slots:
    void delayedInit();

};


// A folder element which returns all email messages

class EmailActionMessageSet : public QMailFilterMessageSet
{
    Q_OBJECT

public:
    EmailActionMessageSet(QMailMessageSetContainer *container, const QString &name);

protected:
    void init();

private slots:
    void delayedInit();

    static QMailMessageKey contentKey();
};


// A model which provides the actions in our 'action list'

class ActionFolderModel : public FolderModel
{
    Q_OBJECT

public:
    enum Roles
    {
        FolderIconRole = FolderModel::FolderIconRole,
        FolderStatusRole = FolderModel::FolderStatusRole,
        FolderStatusDetailRole = FolderModel::FolderStatusDetailRole,
        FolderIdRole = FolderModel::FolderIdRole
    };

    ActionFolderModel(QObject *parent = 0);
    ~ActionFolderModel();

    virtual QVariant headerData(int section, Qt::Orientation, int role) const;

    QVariant data(QMailMessageSet *item, int role, int column) const;


protected:
    virtual void init();

    virtual QIcon itemIcon(QMailMessageSet *item) const;
    virtual QString itemStatus(QMailMessageSet *item) const;
    virtual QString itemStatusDetail(QMailMessageSet *item) const;

    void loadCachedNames();
    void saveCachedNames();

private:
    QMap<QMailFolderId,QString> m_cachedNames;
};


#endif

