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

#ifndef LISTSCREEN_H
#define LISTSCREEN_H

#include <QListWidget>
#include <QContent>
#include <QKeyEvent>

class IViewer;
class QContentSet;
class QContentSetModel;
class QAction;
class QKeyEvent;
class ImageScreen;
class InfoScreen;

class ListScreen : public QListWidget
{
    Q_OBJECT
public:
    ListScreen(IViewer *viewer);
private:
    void createActions();
    void createMenu();
    void setupContent();
    void populate();
    void setupUi();
    void openImage(int row); 
    void keyPressEvent(QKeyEvent *event);
private slots:
    void onOpenImage();
    void onImageActivated(const QModelIndex &index);
    void onRenameImage();
    void onDeleteImage();
    void onShowInfo();
    void updateList(QContentIdList a,QContent::ChangeType b);
private:
    IViewer          *_viewer;
    QContentSet      *_cs;
    QContentSetModel *_csm;
    QAction          *_openAction;
    QAction          *_renameAction;
    QAction          *_deleteAction;
    QAction          *_showInfoAction;
};

#endif
