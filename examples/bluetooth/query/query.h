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

#ifndef QUERY_H
#define QUERY_H

#include <QMainWindow>

class QBluetoothLocalDevice;
class QBluetoothSdpQuery;
class QBluetoothSdpQueryResult;

class QWaitWidget;
class QListWidget;
class QAction;

class Query : public QMainWindow
{
    Q_OBJECT

public:
    Query(QWidget *parent = 0, Qt::WFlags f = 0);
    ~Query();

public slots:
    void cancelQuery();

private slots:
    void startQuery();
    void searchCancelled();
    void searchComplete(const QBluetoothSdpQueryResult &result);

private:
    QBluetoothLocalDevice *btDevice;
    QBluetoothSdpQuery *sdap;
    QWaitWidget *waiter;
    bool canceled;
    QListWidget *serviceList;
    QAction *startQueryAction;
};

#endif
