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

#ifndef ACTIONFOLDERVIEW_H
#define ACTIONFOLDERVIEW_H

#include "actionfoldermodel.h"
#include "folderdelegate.h"
#include "folderview.h"

#include <QtopiaItemDelegate>


class ActionFolderView : public FolderView
{
    Q_OBJECT

public:
    ActionFolderView(QWidget *parent);

    virtual ActionFolderModel *model() const;
    void setModel(ActionFolderModel *model);

signals:
    void composeActionActivated(QMailMessageSet *);
    void emailActionActivated(QMailMessageSet *);
    void folderActivated(QMailMessageSet *);

protected slots:
    virtual void itemActivated(const QModelIndex &index);

protected:
    virtual void showEvent(QShowEvent *e);

private:
    virtual void setModel(QAbstractItemModel *model);

    ActionFolderModel *mModel;
    int mSize;
};


class ActionFolderDelegate : public FolderDelegate
{
    Q_OBJECT

public:
    ActionFolderDelegate(ActionFolderView *parent = 0);
};

#endif
