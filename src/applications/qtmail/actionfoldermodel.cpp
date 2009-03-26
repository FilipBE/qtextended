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

#include "actionfoldermodel.h"
#include <QTimer>
#include <QMailMessageSet>
#include <QMailAccount>
#include <QMailFolder>
#include <QMailStore>
#include <QSettings>

static const char* settingsGroupId = "cachedFolderNames";
static const QPair<QString,QString> settingsInfo("Trolltech","qtmail");

/* ActionFolderMessageSet */

ActionFolderMessageSet::ActionFolderMessageSet(QMailMessageSetContainer *container, const QMailFolderId &folderId, bool hierarchical)
    : QMailFolderMessageSet(container, folderId, hierarchical)
{
}

QMailMessageKey ActionFolderMessageSet::messageKey() const
{
    return contentKey(folderId());
}

QMailMessageKey ActionFolderMessageSet::contentKey(const QMailFolderId &id)
{
    // Only return non-email messages from this foder
    return (QMailFolderMessageSet::contentKey(id, false) &
            QMailMessageKey(QMailMessageKey::Type, QMailMessage::Email, QMailDataComparator::NotEqual));
}

void ActionFolderMessageSet::createChild(const QMailFolderId &childId)
{
    // Our child folders should also be non-email-only
    ActionFolderMessageSet *child = new ActionFolderMessageSet(this, childId, hierarchical());
    append(child);
}


/* ComposeActionMessageSet */

ComposeActionMessageSet::ComposeActionMessageSet(QMailMessageSetContainer *container, const QString &name)
    : QMailFilterMessageSet(container, QMailMessageKey::nonMatchingKey(), name)
{
}

void ComposeActionMessageSet::init()
{
    QTimer::singleShot(0,this,SLOT(delayedInit()));
}

void ComposeActionMessageSet::delayedInit()
{
    QMailFilterMessageSet::init();
}

/* EmailActionMessageSet */

EmailActionMessageSet::EmailActionMessageSet(QMailMessageSetContainer *container, const QString &name)
    : QMailFilterMessageSet(container, contentKey(), name, false)
{
}

void EmailActionMessageSet::init()
{
    QTimer::singleShot(0,this,SLOT(delayedInit()));
}

void EmailActionMessageSet::delayedInit()
{
    QMailFilterMessageSet::init();
}

QMailMessageKey EmailActionMessageSet::contentKey()
{
    // Return all messages of type email
    return QMailMessageKey(QMailMessageKey::Type, QMailMessage::Email);
}


/* ActionFolderModel */

ActionFolderModel::ActionFolderModel(QObject *parent)
    : FolderModel(parent)
{
    init();
}

ActionFolderModel::~ActionFolderModel()
{
    saveCachedNames();
}

QVariant ActionFolderModel::headerData(int section, Qt::Orientation, int role) const
{
    if (role == Qt::DisplayRole && section == 0)
        return tr("Action");

    return QVariant();
}

QVariant ActionFolderModel::data(QMailMessageSet *item, int role, int column) const
{
    if(item && role == Qt::DisplayRole && column == 0)
    {
        //if the storage system is not initialized, then load the last displayed folder name
        if(!QMailStore::storeInitialized())
        {
            if (QMailFolderMessageSet* fms = qobject_cast<QMailFolderMessageSet*>(item))
                return m_cachedNames.value(fms->folderId());
        }
    }
    return FolderModel::data(item,role,column);
}


void ActionFolderModel::init()
{
    // Add the 'compose' action
    append(new ComposeActionMessageSet(this, tr("New message")));

    // Add the standard folders

    foreach (QMailFolder::StandardFolder identifier,
             QList<QMailFolder::StandardFolder>() << QMailFolder::InboxFolder
                                                  << QMailFolder::SentFolder
                                                  << QMailFolder::DraftsFolder
                                                  << QMailFolder::TrashFolder
                                                  << QMailFolder::OutboxFolder)
        append(new ActionFolderMessageSet(this, QMailFolderId(identifier), false));

    // Add the 'email' action
    append(new EmailActionMessageSet(this, tr("Email")));

    loadCachedNames();
}

QIcon ActionFolderModel::itemIcon(QMailMessageSet *item) const
{
    if (qobject_cast<ComposeActionMessageSet*>(item)) {
        return QIcon(":icon/new");
    } else if (qobject_cast<EmailActionMessageSet*>(item)) {
        return QIcon(":icon/email");
    }

    return FolderModel::itemIcon(item);
}

QString ActionFolderModel::itemStatus(QMailMessageSet *item) const
{
    // No status for the Compose action
    if (qobject_cast<ComposeActionMessageSet*>(item))
        return QString();

    return FolderModel::itemStatus(item);
}

QString ActionFolderModel::itemStatusDetail(QMailMessageSet *item) const
{
    // No status detail for the Compose action
    if (qobject_cast<ComposeActionMessageSet*>(item))
        return QString();

    return FolderModel::itemStatusDetail(item);
}

void ActionFolderModel::loadCachedNames()
{
    QSettings settings(settingsInfo.first,settingsInfo.second);

    settings.beginGroup(settingsGroupId);
    QStringList keys = settings.allKeys();
    foreach(QString key, keys)
    {
        QMailFolderId id (key.toULongLong());
        m_cachedNames.insert(id,settings.value(key).toString());
    }
    settings.endGroup();
}

void ActionFolderModel::saveCachedNames()
{
    QSettings settings(settingsInfo.first, settingsInfo.second);

    settings.beginGroup(settingsGroupId);
    settings.remove("");
    for(int i = 0; i < count(); ++i)
        if(QMailFolderMessageSet* fms = qobject_cast<QMailFolderMessageSet*>(at(i)))
            settings.setValue(QString::number(fms->folderId().toULongLong()),fms->displayName());
    settings.endGroup();
}

