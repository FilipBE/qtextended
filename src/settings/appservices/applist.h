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
#ifndef APPLIST_H
#define APPLIST_H

#include <QDialog>

class QContentSet;
class SelectableContentSetModel;

class QListView;
class QModelIndex;

class AppList : public QDialog
{
    Q_OBJECT

    public:
        AppList(const QString &service, QWidget *parent = 0, Qt::WFlags fl = 0);
        ~AppList();

    private slots:
        void loadState();
        void activated(const QModelIndex &index);
        void showDetails();
        void currentChanged(const QModelIndex &current, const QModelIndex &previous);

    private:
        void loadApplications();

        QContentSet *m_applications;
        SelectableContentSetModel *m_model;
        QString m_service;

        QListView *m_applicationView;
};

#endif
