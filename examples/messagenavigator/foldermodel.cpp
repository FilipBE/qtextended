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

#include "foldermodel.h"
#include <QIcon>
#include <QMailStore>
#include <QMailAccountMessageSet>


FolderModel::FolderModel(QObject *parent)
    : QMailMessageSetModel(parent)
{
    // Add an entry for each account, that will maintain its own tree of folders
    foreach (const QMailAccountId &id, QMailStore::instance()->queryAccounts()) 
        append(new QMailAccountMessageSet(this, id, true));
}

FolderModel::~FolderModel()
{
}

QVariant FolderModel::data(QMailMessageSet* item, int role, int column) const
{
    if (role == Qt::DecorationRole) {
        if (qobject_cast<QMailAccountMessageSet*>(item)) {
            // This item is an account message set
            return QIcon(":icon/qtmail/account");
        } else {
            // This item is a folder message set
            return QIcon(":icon/folder");
        }
    } else {
        return QMailMessageSetModel::data(item, role, column);
    }
}
// end-data

