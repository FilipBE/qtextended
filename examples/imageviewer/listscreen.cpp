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

#include "listscreen.h"
#include "iviewer.h"
#include "imagescreen.h"
#include "inputdialog.h"
#include "infoscreen.h"
#include <QtopiaApplication>
#include <QContentSet>
#include <QContentSetModel>
#include <QAction>
#include <QSoftMenuBar>
#include <QMenu>
#include <QKeyEvent>
#include <QInputDialog>
#include <QtGui>

ListScreen::ListScreen(IViewer *viewer) 
: QListWidget(viewer), _viewer(viewer) 
{
    setupUi();
    setupContent();
    populate();
    connect(_cs,SIGNAL(changed(QContentIdList,QContent::ChangeType)),
            this,SLOT(updateList(QContentIdList,QContent::ChangeType)));
}

void ListScreen::setupContent() 
{
    _cs  = new QContentSet(QContentFilter::MimeType, "image/png", this);
    _csm = new QContentSetModel(_cs, this);
}

void ListScreen::populate() 
{
    clear();
    foreach (QContent c, _cs->items()) {
        QListWidgetItem *i = new QListWidgetItem(this);
        QIcon icon(c.file());
        i->setText(c.name());
        i->setIcon(icon);
    }
    if (count() > 0) {
        QListWidgetItem *i = item(0);
        setCurrentItem(i);
    }
}

void ListScreen::setupUi() 
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setIconSize(QSize(32,32));
    connect(this, SIGNAL(activated(QModelIndex)), 
            this, SLOT(onImageActivated(QModelIndex)));
    createActions();
    createMenu();
}

void ListScreen::createActions() 
{
    _openAction = new QAction("Open", this);
    connect(_openAction, SIGNAL(triggered()), this, SLOT(onOpenImage()));
    _renameAction = new QAction("Rename", this);
    connect(_renameAction, SIGNAL(triggered()), this, SLOT(onRenameImage()));
    _deleteAction = new QAction("Delete", this);
    connect(_deleteAction, SIGNAL(triggered()), this, SLOT(onDeleteImage()));
    _showInfoAction = new QAction("Show Info", this);
    connect(_showInfoAction, SIGNAL(triggered()), this, SLOT(onShowInfo()));
}

void ListScreen::createMenu() 
{
    QMenu* menu = QSoftMenuBar::menuFor(this);
    menu->addAction(_openAction);
    menu->addAction(_renameAction);
    menu->addAction(_deleteAction);
    menu->addAction(_showInfoAction);
    QSoftMenuBar::setLabel(this, Qt::Key_Back, "", "Exit", QSoftMenuBar::AnyFocus);
}

void ListScreen::onOpenImage() 
{
    openImage(currentIndex().row());
}

void ListScreen::onImageActivated(const QModelIndex& index) 
{
    openImage(currentIndex().row());
}

void ListScreen::openImage(int row) 
{
    QContent c     = _cs->content(row);
    ImageScreen *s = _viewer->imageScreen();
    s->setImage(c);
    _viewer->setCurrentWidget(s);
}

void ListScreen::onRenameImage() 
{
    QContent c   = _cs->content(currentIndex().row());
    QString name = c.name();
    InputDialog *dlg = new InputDialog(this);
    dlg->setText(name);
    dlg->setWindowTitle("Rename Image:");
    int ans = QtopiaApplication::execDialog(dlg);
    if (ans == QDialog::Accepted) {
        QString newName = dlg->text();
        if (newName.isEmpty())
            return;
        c.setName(newName);
        c.commit();
    }
    delete dlg;
}

void ListScreen::onDeleteImage() 
{
    QContent c = _cs->content(currentIndex().row());
    QString file = c.file();
    int ans = QMessageBox::question(this, "Delete Image", file, QMessageBox::Yes, QMessageBox::No);
    if (ans == QMessageBox::No)
        return;
    c.removeFiles();
    c.commit();
}

void ListScreen::onShowInfo()
{
    InfoScreen *infoScreen = new InfoScreen(_viewer);
    infoScreen->setImage(_cs->content(currentIndex().row()));
    infoScreen->showMaximized();
}

void ListScreen::updateList(QContentIdList a, QContent::ChangeType b)
{
    populate();
}

void ListScreen::keyPressEvent(QKeyEvent* event) 
{
    switch (event->key()) {
    case Qt::Key_Select:
        onOpenImage();
        break;
    default:
        QListWidget::keyPressEvent(event);
        break;
    }
}
