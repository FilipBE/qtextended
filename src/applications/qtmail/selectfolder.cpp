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

#include "selectfolder.h"

#include <QtopiaApplication>
#include <QListWidget>
#include <QLayout>
#include <QMailFolder>


SelectFolderDialog::SelectFolderDialog(const QMailFolderIdList &list, QWidget *parent)
    : QDialog( parent ),
      mFolderIds( list )
{
    QtopiaApplication::setMenuLike( this, true );
    setWindowTitle( tr( "Select folder" ) );

    mFolderList = new QListWidget( this );

    foreach (const QMailFolderId &folderId, mFolderIds) {
        QMailFolder folder(folderId);
        mFolderList->addItem(folder.name());
    }

    // Required for current item to be shown as selected(?)
    if (mFolderList->count())
        mFolderList->setCurrentRow( 0 );

    connect(mFolderList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(selected()) );

    QGridLayout *top = new QGridLayout( this );
    top->addWidget( mFolderList, 0, 0 );
}

SelectFolderDialog::~SelectFolderDialog()
{
}

QMailFolderId SelectFolderDialog::selectedFolderId() const
{
    return mFolderIds[mFolderList->currentRow()];
}

void SelectFolderDialog::selected()
{
    done(1);
}

