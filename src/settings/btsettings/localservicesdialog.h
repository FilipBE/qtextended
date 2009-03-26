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

#ifndef LOCALSERVICESDIALOG_H
#define LOCALSERVICESDIALOG_H

#include <QDialog>

class QListView;
class ServicesModel;
class QBluetoothServiceController;
class QModelIndex;
class QItemSelection;
class QAction;

class LocalServicesDialog : public QDialog
{
    Q_OBJECT
public:
    LocalServicesDialog(QWidget *parent = 0, Qt::WFlags fl=0);
    ~LocalServicesDialog();

public slots:
    void start();

private slots:
    void activated(const QModelIndex &index);
    void serviceStarted(const QString &name, bool error, const QString &desc);
    void toggleCurrentSecurity(bool checked);
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    void toggleState(const QModelIndex &index);

    QBluetoothServiceController *m_serviceController;
    ServicesModel *m_model;
    QListView *m_view;

    QAction *m_securityAction;
    QString m_lastStartedService;
};

#endif
