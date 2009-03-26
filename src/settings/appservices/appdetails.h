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
#ifndef APPDETAILS_H
#define APPDETAILS_H

#include <QDialog>

#include <QMap>
#include <QContent>

class QListWidget;
class QListWidgetItem;

class AppDetails : public QDialog
{
    Q_OBJECT

    public:
        AppDetails(const QContent &app, QWidget *parent = 0, Qt::WFlags fl = 0);
        ~AppDetails();

        void accept();

    private slots:
        void loadState();
        void showMimeTypes(bool);

    private:
        void loadServices();

        QContent m_application;
        QMap<QString, QListWidgetItem *> m_serviceDict;

        QListWidget *m_services;
};

#endif
