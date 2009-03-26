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
#ifndef SELECTFOLDER_H
#define SELECTFOLDER_H

#include <QDialog>
#include <QList>
#include <QMailFolderId>


class QListWidget;

class SelectFolderDialog : public QDialog
{
    Q_OBJECT

public:
    SelectFolderDialog(const QMailFolderIdList &folderIds, QWidget *parent = 0);
    virtual ~SelectFolderDialog();

    QMailFolderId selectedFolderId() const;

private slots:
    void selected();

private:
    QListWidget *mFolderList;
    QMailFolderIdList mFolderIds;
};

#endif
